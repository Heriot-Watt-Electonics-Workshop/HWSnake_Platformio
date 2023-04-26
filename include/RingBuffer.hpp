#ifndef __RINGBUFFER_HPP_
#define __RINGBUFFER_HPP_

#include "stdint.h"

// A ring buffer is a memory structure where a contiguous block of memory is allocated at one end
// and de-allocated at the other.  At one point the memory loops around and starts again.  As the 
// memory is filled the head will start to catch up with the tail and when it does the buffer is 
// full.  Ring buffers are often used when data is transmitted and received such as with audio.
// The snake in this game would seem to be suited to a ringbuffer.


/// ->->->-> A RingBuffer and Iterators for the snake. <-<-<-<-

// As there is not enough memory to store the whole snake as a structure made of 2 byte
//  coordinates instead we will store each part of the snake using only a cardinal direction.
//  we will have to check the location of the head *** or a count of elements in the snake ***
//  to know when we have completed a complete traversal of the snake structure.  In this way
//  a snake body segment can be reduced to only taking half a nibble (4 bits) or a crumb (2 bits).


template <typename T, uint8_t Size> // This is a C++ class template.  Only used functions are instantiated.
class RingBuffer {

static_assert(Size < 255, "Buffer too big.  This only takes up to 254.\n");

public:
	struct ForwardIterator;
	struct ReverseIterator;

    RingBuffer();

	// Forward iterator should iterate from the write pointer back.
	ForwardIterator begin() noexcept;
	ForwardIterator end() noexcept;

    ReverseIterator rbegin() noexcept;
	const ReverseIterator rbegin() const noexcept;
    ReverseIterator rend() noexcept;
	const ReverseIterator rend() const noexcept;

    constexpr uint8_t size() const;
	constexpr bool empty() const;
	constexpr bool full() const;
	constexpr uint8_t spaceRemaining() const;
    constexpr uint8_t capacity() const;

    T& front();
    constexpr const T& front() const;
    T& back();
    constexpr const T& back() const;

	bool push(T data);
	T pop();
	void clear();
	
private:
	T data[Size + 1]; // write == read is empty so we need 1 more index.

	// A ReverseIterator because forwards is reading from write back to read.
    ReverseIterator write;
	ReverseIterator read;
};

// We will create a template class which is a subclass of Ringbuffer<int>


template <typename T, uint8_t Size>
struct RingBuffer<T, Size>::ForwardIterator {

	friend class RingBuffer<T, Size>;
    
    using self_type = RingBuffer<T, Size>::ForwardIterator;
    using value_type = T;
    using reference = T&;
	using const_reference = const T&;
    using pointer = T*;
	using const_pointer = const T*;
    using difference_type = ptrdiff_t;

    ForwardIterator(pointer ptr, RingBuffer<T, Size>& buf);
    
    reference operator*();
	constexpr const_reference operator*() const;
	pointer operator->();
	constexpr const_pointer operator->() const;
    
    // Unary operators
    // prefix
	self_type operator++();
    self_type operator--();
    // postfix
	self_type operator++(int);
    self_type operator--(int);

    // Comparison operators
    constexpr bool operator==(const self_type& other) const;
    constexpr bool operator!=(const self_type& other) const;
    constexpr bool operator<=(const self_type& other) const;
    constexpr bool operator>=(const self_type& other) const;
    
    // Arithmetic operators
	constexpr self_type operator-(const difference_type& distance) const;
	constexpr self_type operator+(const difference_type& distance) const;

    constexpr difference_type operator-(const self_type& other) const;

private:
    pointer ptr;
	RingBuffer<T, Size>& buf;
};


template <typename T, uint8_t Size>
struct RingBuffer<T, Size>::ReverseIterator {

	friend class RingBuffer<T, Size>;
    
    using self_type = RingBuffer<T, Size>::ReverseIterator;
    using value_type = T;
    using reference = T&;
	using const_reference = const T&;
    using pointer = T*;
	using const_pointer = const T*;
    using difference_type = ptrdiff_t;

    ReverseIterator(pointer ptr, RingBuffer<T, Size>& buf);
    
	reference operator*();
	constexpr const_reference operator*() const;
	pointer operator->();
	constexpr const_pointer operator->() const;
    
    // Unary operators
    // prefix
	self_type operator++();
	self_type operator--();
    // postfix
	self_type operator++(int);
	self_type operator--(int);

    // Comparison operators
	constexpr bool operator==(const self_type& other) const;
    constexpr bool operator!=(const self_type& other) const;
    constexpr bool operator<=(const self_type& other) const;
    constexpr bool operator>=(const self_type& other) const;
    
    // Arithmetic operators
	constexpr self_type operator+(const difference_type& distance) const;
	constexpr self_type operator-(const difference_type& distance) const;
	constexpr difference_type operator-(const self_type& other) const;

private:
    pointer ptr;
    RingBuffer<T, Size>& buf;
};



// *** RingBuffer ***

template<typename T, uint8_t Size>
RingBuffer<T, Size>::RingBuffer() : write{data, *this}, read{data, *this} {}

template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ForwardIterator
RingBuffer<T, Size>::begin() noexcept {
	return ForwardIterator{ &*(write - 1), *this };
}
	
template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ForwardIterator RingBuffer<T, Size>::end() noexcept {
	return ForwardIterator{ &*(read - 1), *this };
}
    
template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ReverseIterator
RingBuffer<T, Size>::rbegin() noexcept {
	return read;
}

template<typename T, uint8_t Size>
const typename RingBuffer<T, Size>::ReverseIterator
RingBuffer<T, Size>::rbegin() const noexcept {
	return read;
}

template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ReverseIterator
RingBuffer<T, Size>::rend() noexcept {
	return write; 
}

template<typename T, uint8_t Size>
const typename RingBuffer<T, Size>::ReverseIterator
RingBuffer<T, Size>::rend() const noexcept {
	return write;
}
    

template<typename T, uint8_t Size>
constexpr uint8_t RingBuffer<T, Size>::size() const {
	return ((write >= read) ? write - read : write - read + (Size + 1) );
}

template<typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::empty() const {
		return (size() == 0);
}

template<typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::full() const {
	return (write + 1 == read); 
}

template<typename T, uint8_t Size>
constexpr uint8_t RingBuffer<T, Size>::spaceRemaining() const {
	return capacity() - size(); 
}

template<typename T, uint8_t Size>
constexpr uint8_t RingBuffer<T, Size>::capacity() const {
	return Size; 
}

template<typename T, uint8_t Size>
T& RingBuffer<T, Size>::front() {
	assert(!empty() && "Empty");
	return *begin(); 
}

template<typename T, uint8_t Size>
constexpr const T& RingBuffer<T, Size>::front() const {
	static_assert(!empty(), "empty");
//	assert(!empty() && "Empty");
	return *begin();
}

template<typename T, uint8_t Size>
T& RingBuffer<T, Size>::back() {
	assert(!empty() && "Empty");
	return *read;  
}

template<typename T, uint8_t Size>
constexpr const T& RingBuffer<T, Size>::back() const {
	static_assert(!empty(), "empty");
//	assert(!empty() && "Empty");
	return *read; 
}

template<typename T, uint8_t Size>
bool RingBuffer<T, Size>::push(T data) {
	if (write + 1 == read)
		return false;
	*write++ = data;
	return true; 
}

template<typename T, uint8_t Size>
T RingBuffer<T, Size>::pop() {
	assert(!empty() && "Cannot pop from empty buffer");
	T rVal = *read; ++read;
	return rVal;
}

template<typename T, uint8_t Size>
void RingBuffer<T, Size>::clear() {
	write.ptr = data;
	read.ptr = data;
}


// *** Forward Iterator ***

template<typename T, uint8_t Size>
RingBuffer<T, Size>::ForwardIterator::ForwardIterator(pointer ptr, RingBuffer<T, Size>& buf) : ptr{ptr}, buf{buf} {}

template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ForwardIterator::reference
RingBuffer<T, Size>::ForwardIterator::operator*() {
	return *ptr; 
}

template<typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ForwardIterator::const_reference
RingBuffer<T, Size>::ForwardIterator::operator*() const { 
	return *ptr; 
}


template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ForwardIterator::pointer
RingBuffer<T, Size>::ForwardIterator::operator->() {
	return ptr;
}

template<typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ForwardIterator::const_pointer
RingBuffer<T, Size>::ForwardIterator::operator->() const {
	return ptr;
}


// Unary operators
// prefix
template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ForwardIterator::self_type
RingBuffer<T, Size>::ForwardIterator::operator++() {
	if (--ptr < buf.data) ptr = buf.data + Size;
	return *this;
}

template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ForwardIterator::self_type
RingBuffer<T, Size>::ForwardIterator::operator--() {
	if (++ptr == &buf.data[Size + 1]) ptr = buf.data;
	return *this;
}

// postfix
template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ForwardIterator::self_type
RingBuffer<T, Size>::ForwardIterator::operator++(int) {
	self_type rVal = *this;
	--(*this);
	return rVal; 
}

template<typename T, uint8_t Size>
typename RingBuffer<T, Size>::ForwardIterator::self_type
RingBuffer<T, Size>::ForwardIterator::operator--(int) {
	self_type rVal = *this;
	++(*this);
	return rVal;
}


// Comparison operators
template<typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::ForwardIterator::operator==(const self_type& other) const {
	return other.ptr == this->ptr;
}

template<typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::ForwardIterator::operator!=(const self_type& other) const {
	return other.ptr != this->ptr;
}

template<typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::ForwardIterator::operator<=(const self_type& other) const {
	return this->ptr >= other.ptr;
}

template<typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::ForwardIterator::operator>=(const self_type& other) const {
	return this->ptr <= other.ptr;
}


// Arithmetic operators
template<typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ForwardIterator::self_type
RingBuffer<T, Size>::ForwardIterator::operator-(const difference_type& distance) const {
	return self_type(buf.data + ((ptr - buf.data + distance) % (Size + 1)), buf);
}

template<typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ForwardIterator::self_type
RingBuffer<T, Size>::ForwardIterator::operator+(const difference_type& distance) const {
	auto indexOfResult = ((ptr - buf.data) + ((Size + 1) - distance)) % (Size + 1);
	return self_type(buf.data + indexOfResult, buf);
}

template<typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ForwardIterator::difference_type
RingBuffer<T, Size>::ForwardIterator::operator-(const self_type& other) const {
	//return this->ptr - other->ptr;
	return (reinterpret_cast<intptr_t>(this->ptr) - reinterpret_cast<intptr_t>(other.ptr)) / sizeof(value_type);
}




// *** Reverse Iterator ***

template <typename T, uint8_t Size>
RingBuffer<T, Size>::ReverseIterator::ReverseIterator(pointer ptr, RingBuffer<T, Size>& buf) : ptr{ptr}, buf{buf} {}


template <typename T, uint8_t Size>
typename RingBuffer<T, Size>::ReverseIterator::reference
RingBuffer<T, Size>::ReverseIterator::operator*() {
	return *ptr;
}

template <typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ReverseIterator::const_reference
RingBuffer<T, Size>::ReverseIterator::operator*() const {
	return *ptr;
}

template <typename T, uint8_t Size>
typename RingBuffer<T, Size>::ReverseIterator::pointer
RingBuffer<T, Size>::ReverseIterator::operator->() {
	return ptr;
}

template <typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ReverseIterator::const_pointer
RingBuffer<T, Size>::ReverseIterator::operator->() const {
	return ptr;
}


// Unary operators
// prefix
template <typename T, uint8_t Size>
typename RingBuffer<T, Size>::ReverseIterator::self_type
RingBuffer<T, Size>::ReverseIterator::operator++() {
	if (++ptr == &buf.data[Size + 1]) 
		ptr = buf.data;
	return *this;
}

template <typename T, uint8_t Size>
typename RingBuffer<T, Size>::ReverseIterator::self_type
RingBuffer<T, Size>::ReverseIterator::operator--() {
	if (--ptr < buf.data)
		ptr = buf.data + Size;
	return *this;
}

// postfix
template <typename T, uint8_t Size>
typename RingBuffer<T, Size>::ReverseIterator::self_type
RingBuffer<T, Size>::ReverseIterator::operator++(int) {
	self_type rVal = *this; ++(*this);
	return rVal;
}

template <typename T, uint8_t Size>
typename RingBuffer<T, Size>::ReverseIterator::self_type
RingBuffer<T, Size>::ReverseIterator::operator--(int) {
	self_type rVal = *this;
	--(*this);
	return rVal;
}


// Comparison operators
template <typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::ReverseIterator::operator==(const self_type& other) const {
	return this->ptr == other.ptr;
}

template <typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::ReverseIterator::operator!=(const self_type& other) const {
	return this->ptr != other.ptr;
}

template <typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::ReverseIterator::operator<=(const self_type& other) const {
	return this->ptr <= other.ptr;
}

template <typename T, uint8_t Size>
constexpr bool RingBuffer<T, Size>::ReverseIterator::operator>=(const self_type& other) const {
	return this->ptr >= other.ptr;
}


// Arithmetic operators
template <typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ReverseIterator::self_type
RingBuffer<T, Size>::ReverseIterator::operator+(const difference_type& distance) const {
	return self_type(buf.data + ((ptr - buf.data + distance) % (Size + 1)), buf);
}

template <typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ReverseIterator::self_type
RingBuffer<T, Size>::ReverseIterator::operator-(const difference_type& distance) const {
	auto indexOfResult = ((ptr - buf.data) + ((Size + 1) - distance)) % (Size + 1);
	return self_type(buf.data + indexOfResult, buf);
}

template <typename T, uint8_t Size>
constexpr typename RingBuffer<T, Size>::ReverseIterator::difference_type
RingBuffer<T, Size>::ReverseIterator::operator-(const self_type& other) const {
	//return this->ptr - other->ptr;
	return (reinterpret_cast<intptr_t>(this->ptr) - reinterpret_cast<intptr_t>(other.ptr)) / sizeof(value_type);
}


#endif // __RINGBUFFER_HPP_