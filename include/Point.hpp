#ifndef __POINT_HPP_
#define __POINT_HPP_

#include <Arduino.h>
#include <globals.hpp>

// A point
// Templated to allow for changing the stored data type easily throughout the whole
//	program.
template <typename T>
struct Point
#if (DEBUG == YES) 
: public Printable	// Inheriting from printable can allow you to print() a point.
#endif 
{
	Point() : y{0}, x{0} {}
	Point(T y, T x) : y{y}, x{x} {}

	// The Coordinates
	T y, x; 
	// Maybe int8_t would suffice. Or uints if subtraction operator was defined.
	// However small size of type is not such an imperative when the snake is made of crumbs.

	// Override equality operator to compare points.
	bool operator==(const Point& other) const { return (other.y == y && other.x == x); }
	// Override addition assignment operator to add and assign.
	void operator+=(const Point& other) { y += other.y; x += other.x; }
	void operator-=(const Point& other) { y -= other.y; x -= other.x; }

#if (DEBUG == YES)
size_t printTo(Print& p) const {
	char rVal[20];
	if (Utility::is_signed<T>::value)
		sprintf(rVal, "(%d, %d)", y, x);
	else if (Utility::is_unsigned<T>::value)
		sprintf(rVal, "(%u, %u)", y, x);
	return p.print(rVal);
}
#endif
};

#endif // __POINT_HPP_
