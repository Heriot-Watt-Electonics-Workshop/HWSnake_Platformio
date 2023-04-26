#ifndef __SNAKESTRUCT_HPP_
#define __SNAKESTRUCT_HPP_

#include "RingBuffer.hpp"

// SnakeStruct is a wrapper around ringbuffer to allow us to store our snake as 
//	a series of cardinal directions to the next coordinate rather than as a series
//	of coordinates.  Hence we will be able to store a segment in a crumb instead of
//	2 bytes which will be about 16 times more efficient.

template<typename EXTERNALTYPE, uint16_t Size>
struct SnakeStruct : RingBuffer<uint8_t, (Size / 4) + 1> {

	bool initialized { false };
	EXTERNALTYPE head { 0, 0 };
	uint8_t crumb { 3 };  // fill from 3 to 0;  empty from 0 to 3; l > r / l < r;
	uint16_t m_size { 0 };

	bool push(EXTERNALTYPE data);
	EXTERNALTYPE pop();
	uint16_t size();

    EXTERNALTYPE& front() {}
	constexpr const EXTERNALTYPE& back() const {}
	EXTERNALTYPE& back() {}
    constexpr const EXTERNALTYPE& front() const {}

	
	// Forward iterator should iterate from the write pointer back.
	ForwardIterator begin() noexcept;
	ForwardIterator end() noexcept;
};


#endif // __SNAKESTRUCT_HPP_


template <typename EXTERNALTYPE, uint16_t Size>
bool SnakeStruct<EXTERNALTYPE, Size>::push(EXTERNALTYPE data) { 
		DEBUG_PRINT("Pushing data....\n");
		DEBUG_PRINTLN(data);
		if (initialized) {
			// If a new byte then create a new byte and push it to data.
			// If not a new byte pop the value edit and push it again.
			uint8_t byte = RingBuffer<uint8_t, (Size / 4) + 1>::pop();
			// Insert the value to the byte.
		} 
		else { head = data; initialized = true; }
		
		return true;
	}

template <typename EXTERNALTYPE, uint16_t Size>
EXTERNALTYPE SnakeStruct<EXTERNALTYPE, Size>::pop() {
	DEBUG_PRINTLN("Popping data");
}

template <typename EXTERNALTYPE, uint16_t Size>
uint16_t SnakeStruct<EXTERNALTYPE, Size>::size() {
	return m_size;
}
