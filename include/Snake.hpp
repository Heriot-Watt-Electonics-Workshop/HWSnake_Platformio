#ifndef __SNAKE_HPP_
#define __SNAKE_HPP_

#include <Arduino.h>
#include "globals.hpp"
#include "Geometry.hpp"
#include "error.hpp"



// // Split boolean value return type.
// template <typename T>
// struct OptionalCOORD {
// 	bool _true;
// 	T val;
// };

// Optional wrapper for a point. ie it may or may not contain data.
template <typename DATA_TYPE>
struct OptionalPoint : protected Point<DATA_TYPE> {
private:
	const bool mHasValue { true };
public:
	OptionalPoint(const DATA_TYPE y, const DATA_TYPE x) :Point<DATA_TYPE>{y, x} { }
	OptionalPoint(const Point<DATA_TYPE>& p) : Point<DATA_TYPE>{p} { }
	OptionalPoint() : mHasValue{ false } { }
	bool hasValue() const { return hasValue; }
	bool isEmpty() const { return !hasValue; }
	const Point<DATA_TYPE> getValue() const { return {this->y, this->x}; }
	operator bool() { return mHasValue; }
};





// A crumb is half of a nibble which is half of a byte so it is essentially 2-bits.
//  The smallest amount of memory addresable by a pointer in C++ is a byte so this struct
//  is required to address a smaller amount.  Allowing the Snake to be stored in 1/8 of the 
//  memory it would be if one were to store 2-byte points (uint8_t, uint8_t).  Instead we store
//  the coordinate of the Snake's tail and then iterate through it's body by using directions.
//  Up, Down, Left and Right.  These 4 can be stored in a crumb or 2 bits.  00, 01, 10, and 11.
//  On an 8-bit avr pointers are stored as bytes.  I could have used a 16-bit data type and
//  shifted in and out the crumb using left and right shift to separate the pointer from the 
//  crumb value, however i developed the snake on a 64-bit pc.  The largest integer data type 
//  in C++ is 64-bit so instead i created this struct.
struct CrumbPtr 
#if (DEBUG == YES)
: public Printable
#endif // (DEBUG == YES)
{
    
	uint8_t* ptr; // 64 bits on a pc. 8 bits on an avr.  
	uint8_t crumb;
  
	using selfType = CrumbPtr;

	CrumbPtr(uint8_t* ptr, uint8_t crumb) : ptr{ptr}, crumb{crumb} {}
	Direction getValue();
  	void putValue(Direction newval);
	
	bool operator==(const CrumbPtr&) const;
	bool operator!=(const CrumbPtr&) const;
	selfType& operator++(); // Prefix
	selfType operator++(int); // Postfix
	selfType& operator--(); // Prefix
	selfType operator--(int); // Postfix

#if (DEBUG == YES)
	size_t printTo(Print& p) const;
#endif
};



//    ---- memory ----
// <  ================  <0>
//  memstart       memend
template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
class Snake 
#if (DEBUG == YES)
: public Printable
#endif 
{

	uint8_t data[SNAKE_DATA_SIZE] {};
	uint16_t m_length { 0 };
	Direction m_dir { Direction::NONE };
	POINT_TYPE m_head {};
	POINT_TYPE m_tail {};
	CrumbPtr memstart { data, 0 }; // When popping memstart is increased.
	CrumbPtr memend { data, 0 }; // When pushing memend is increased.
    
public:
    
	//Snake() : data{}, m_length{}, m_dir{Direction::NONE}, m_head{}, m_tail{}, memstart{data, 0}, memend {data, 0}  {}
    uint16_t capacity() const { return 1 + (sizeof(data) * 4); }
    bool full() const { return ( m_length == capacity() ); }
    bool empty() const { return ( m_length == 0 ); }
    uint16_t length() const { return m_length; }
    const POINT_TYPE& head() const { return m_head; } 
    const POINT_TYPE& tail() const { return m_tail; }
	Direction getDirection() const { return m_dir; }
	void setDirection(Direction d) { m_dir = d; }

    // Pushing adds to the head end.
    bool push(const POINT_TYPE& p);
    // pop
    const POINT_TYPE pop();    
	// Access index using subscript operator.
	const POINT_TYPE operator[](size_t index) const;
	// Point is in the snake. and return detected point.
	OptionalPoint<POINT_DATA_TYPE> pointIsInside(const POINT_TYPE& p);

#if (DEBUG == YES)
	size_t printTo(Print& p) const;
#endif
};


template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
bool Snake<SNAKE_DATA_SIZE, POINT_TYPE>::push(const POINT_TYPE& p) {
    
	//DEBUG_PRINT("Len: ");
	//DEBUG_PRINTLN(m_length);
	if (full()) { return false; } // Basically if full don't add more.

	if (m_length > 0) {  
        // move current head to memory.
        if (p.y == m_head.y + 1) {		memend.putValue(Direction::DOWN); }
        else if (p.x == m_head.x + 1) { memend.putValue(Direction::LEFT); }
        else if (p.y + 1 == m_head.y) { memend.putValue(Direction::UP); }
        else if (p.x + 1 == m_head.x) { memend.putValue(Direction::RIGHT); }
        else {
			DEBUG_PRINTLN_FLASH("Error. Bad insert.\n");
			//exit(1);
		}
        
        ++memend;

        if (memend.ptr > data + sizeof(data) - 1)
            memend.ptr = data;
        // Round off the ptr here.
        // add new point to head.
	//	DEBUG_PRINT_FLASH("Setting head to: ");
	//	DEBUG_PRINTLN(p);
        m_head = p;
        
    } // insert dir in mem
    else { m_head = m_tail = p; } // If empty.
    m_length++;
    return true;
}

template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
const POINT_TYPE Snake<SNAKE_DATA_SIZE, POINT_TYPE>::pop() {

    if (empty()) { return POINT_TYPE { 0, 0 }; } // Should never be empty in the game.

    DEBUG_PRINT_FLASH("Len: ");
	DEBUG_PRINTLN(m_length);
	const POINT_TYPE rval = m_tail;
	if (m_length == 1) { 
		m_tail = m_head = { 0, 0 };
		m_length = 0;
		return rval;
	}// Length of 1 head and tail are reset and snake length set to zero.
	if (m_length == 2) {
		m_tail = m_head;
		m_length--;
		memstart.ptr = memend.ptr = data;	// Reset the memory.
		memstart.crumb = memend.crumb = 0;
		return rval; 
	} // Length of 2 so tail set to head and no need to adjust the buffer.
    DEBUG_PRINTLN("p");
    switch(~memstart.getValue()) {
        
        case Direction::UP: m_tail += { 1, 0 }; break;
        case Direction::DOWN: m_tail -= { 1, 0 }; break;
        case Direction::LEFT: m_tail -= { 0, 1 }; break;
        case Direction::RIGHT: m_tail += { 0, 1 }; break;
            
        default: exit(1);
    }
//	DEBUG_PRINT_FLASH("m_tail set to: ");
//	DEBUG_PRINTLN(m_tail);
    memstart++;
    if (memstart.ptr > data + sizeof(data) - 1)
		memstart.ptr = data;
	
//	DEBUG_PRINTLN_FLASH("m_length--");
    m_length--;
	DEBUG_PRINTLN("r");
    return rval;
}

#pragma message "Would be more efficient if index was counted from closest end of snake."
template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
const POINT_TYPE Snake<SNAKE_DATA_SIZE, POINT_TYPE>::operator[](size_t index) const {
	// we are talking head to tail index here.
	// tail to head is counting forward and head to tail is counting backwards.
	
	// Print an error.
	if (index + 1 > m_length) { 
		Error::displayError(__LINE__, __FILE__, "Out of range access.");
		return {0, 0}; 
	}
	if (index == 0) { return m_head; };
	
	// So we follow the snake from the tail.
	int indexFromTail { static_cast<int>(m_length) - static_cast<int>(index) - 1 };
	CrumbPtr cp { memstart };
	POINT_TYPE p { m_tail };
	
	for (int i{0}; i < indexFromTail; ++i) {
		
		switch(~cp.getValue()) {
	
			case Direction::UP: p += { 1, 0 }; break;
			case Direction::DOWN: p -= { 1, 0 }; break;
			case Direction::LEFT: p -= { 0, 1 }; break;
			case Direction::RIGHT: p += { 0, 1 }; break;
			default: exit(1);
		}
		cp++;
		// I don't agree with the constness here.  Data is never altered.
		if (cp.ptr > data + sizeof(data) - 1) cp.ptr = const_cast<uint8_t*>(data);
	}
	return p;
}


template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
OptionalPoint<POINT_DATA_TYPE> Snake<SNAKE_DATA_SIZE, POINT_TYPE>::pointIsInside(const POINT_TYPE& p) {
	
	if (p == m_head) return { m_head };
	if (m_length == 1) return OptionalPoint<POINT_DATA_TYPE>();
	
	//DEBUG_PRINT_FLASH("p: ");
	//DEBUG_PRINTLN(p);
	
	auto segment {m_head};
	auto cp { memend };
	--cp;
	auto end { memstart };
	// deal with the below afterwards by returning the colliding point and discarding
	// if it is the tail.
	//++end; // We want to end just before tail because tail will move out of the way.
	//if (end.ptr > data + sizeof(data) - 1) end.ptr = data;

//	DEBUG_PRINT_FLASH("memstart: "); DEBUG_PRINTLN(memstart);
//	DEBUG_PRINT_FLASH("memend: "); DEBUG_PRINTLN(memend);
//	DEBUG_PRINT(m_head);

	for (;; cp--) {

		if (cp.ptr < data) cp.ptr = data + (sizeof(data) - 1);

//		DEBUG_PRINT_FLASH("cp: "); DEBUG_PRINTLN(cp);
		
		switch(cp.getValue()) {
            
             case Direction::UP: 	segment += { 1, 0 }; break;
             case Direction::DOWN: 	segment -= { 1, 0 }; break;
             case Direction::LEFT: 	segment -= { 0, 1 }; break;
             case Direction::RIGHT: segment += { 0, 1 }; break;
             default: exit(1);
        }
//		DEBUG_PRINT(segment);
		if (segment == p) return { segment };
		if (cp == end) break;
	}
	//DEBUG_PRINTLN();
	return OptionalPoint<POINT_DATA_TYPE>{};
}


#if (DEBUG == YES)
template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
size_t Snake<SNAKE_DATA_SIZE, POINT_TYPE>::printTo(Print& p) const {
	
	size_t count {0};
	POINT_TYPE pnt {m_tail};
	auto it { memstart };

	DEBUG_PRINT_FLASH("<<<");
	count += p.print(m_tail);

	for (uint16_t i = 0; i < m_length - 1; ++i) {

		switch(~it.getValue()) {
            
             case Direction::UP: 	pnt += { 1, 0 }; break;
             case Direction::DOWN: 	pnt -= { 1, 0 }; break;
             case Direction::LEFT: 	pnt -= { 0, 1 }; break;
             case Direction::RIGHT: pnt += { 0, 1 }; break;
             default: exit(1);
        }
		
		count += p.print(pnt);
		it++;

		if (it.ptr > data + sizeof(data) - 1) it.ptr = const_cast<uint8_t*>(data); 
	}
	
	DEBUG_PRINT_FLASH(":=<");
	return count;
}
#endif

#endif // __SNAKE_HPP_
