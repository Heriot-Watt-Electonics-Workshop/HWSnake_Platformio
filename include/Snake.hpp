#ifndef __SNAKE_HPP_
#define __SNAKE_HPP_

#include <Arduino.h>
#include "globals.hpp"
#include "Point.hpp"

// constexpr const uint16_t MAX_SNAKE_SEGMENTS { 256 };
// constexpr const uint8_t SNAKE_DATA_SIZE { MAX_SNAKE_SEGMENTS / 4 };


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
struct CrumbPtr {
    
	uint8_t* ptr; // 64 bits on a pc. 8 bits on an avr.  
	uint8_t crumb;
  
	using selfType = CrumbPtr;
  
	Direction getValue();
  	void putValue(Direction newval);
  
	selfType operator++();
	selfType operator++(int);
	selfType operator--();
	selfType operator--(int);
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

#if (DEBUG == YES)
	size_t printTo(Print& p) const;
#endif
};


template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
bool Snake<SNAKE_DATA_SIZE, POINT_TYPE>::push(const POINT_TYPE& p) {
    
	DEBUG_PRINTLN(m_length);
	if (full()) { return false; } // Basically if full don't add more.
    
	DEBUG_PRINT_FLASH("Adding new head at:");
	DEBUG_PRINTLN(p);

	if (m_length > 0) {  
        // move current head to memory.
        if (p.y == m_head.y + 1) { memend.putValue(Direction::DOWN); }
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
        m_head = p;
        
    } // insert dir in mem
    else { m_head = m_tail = p; } // If empty.
    m_length++;
    return true;
}

template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
const POINT_TYPE Snake<SNAKE_DATA_SIZE, POINT_TYPE>::pop() {
    
    if (empty()) { return POINT_TYPE { 0, 0 }; } 
    const Point rval = m_tail;
    
    switch(~memstart.getValue()) {
        
        case Direction::UP: m_tail += { 1, 0 }; break;
        case Direction::DOWN: m_tail -= { 1, 0 }; break;
        case Direction::LEFT: m_tail -= { 0, 1 }; break;
        case Direction::RIGHT: m_tail += { 0, 1 }; break;
            
        default: exit(1);
    }
    memstart++;
    if (memstart.ptr >= data + sizeof(data) - 1) memstart.ptr = data;
	
	DEBUG_PRINTLN_FLASH("m_length--");
    m_length--;
    return rval;
}

template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
const POINT_TYPE Snake<SNAKE_DATA_SIZE, POINT_TYPE>::operator[](size_t index) const {
	// we are talking head to tail index here.
	// tail to head is counting forward and head to tail is counting backwards.
	
	// Print an error.
	if (index + 1 > m_length) { exit(1); /* DO SOME ERROR THINGY; */ return {0, 0}; }
	if (index == 0) { return m_head; };
	
	// So we follow the snake from the tail.
	int indexFromTail = m_length - index - 1;
	CrumbPtr cp = memstart;
	Point p = m_tail;
	
	for (int i = 0; i < indexFromTail; ++i) {
		
		switch(~cp.getValue()) {
	
			case Direction::UP: p += { 1, 0 }; break;
			case Direction::DOWN: p -= { 1, 0 }; break;
			case Direction::LEFT: p -= { 0, 1 }; break;
			case Direction::RIGHT: p += { 0, 1 }; break;
			default: exit(1);
		}
		cp++;
		// I don't agree with the constness here.  Data is never altered.
		if (cp.ptr >= data + sizeof(data)) cp.ptr = const_cast<uint8_t*>(data);
	}
	return p;
}


#if (DEBUG == YES)
template <uint8_t SNAKE_DATA_SIZE, typename POINT_TYPE>
size_t Snake<SNAKE_DATA_SIZE, POINT_TYPE>::printTo(Print& p) const {
	
	char snake_str[51] {};
	//uint8_t count = 0;
	//const char tail[] = "<<<";
	//const char head[] = ":=<";

	Point pnt = m_tail;
	auto it = memstart;
	snprintf(snake_str, 51, "(%d,%d)", m_tail.y, m_tail.x);

	for (uint16_t i = 0; i < m_length - 1; ++i) {

		switch(~it.getValue()) {
            
             case Direction::UP: pnt += { 1, 0 }; break;
             case Direction::DOWN: pnt -= { 1, 0 }; break;
             case Direction::LEFT: pnt -= { 0, 1 }; break;
             case Direction::RIGHT: pnt += { 0, 1 }; break;
             default: exit(1);
        }

		snprintf(snake_str, 50, "%s-(%d,%d)", snake_str, pnt.y, pnt.x); 
	}
	return p.print(snake_str);
}
#endif

#endif // __SNAKE_HPP_
