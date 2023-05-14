#ifndef __GLOBALS_HPP_

#include "Arduino.h"

// Defined for readability.
#define YES 1
#define NO 0

// This switches on or off serial debug output.
#define DEBUG YES // or NO
#define LIVE_ERRORS YES

// This determines the size of the array used to store the snake.  The
// snake may be 4 times this size + 1 for the head.  Maximum if all spaces 
// were in the snake would be 160 sections so 40 bytes of data should be enough.
#if (DEBUG == YES)
constexpr uint8_t SNAKE_DATA_SIZE { 20 };
#elif (DEBUG == NO)
constexpr uint8_t SNAKE_DATA_SIZE { 40 };
#endif

// Store Points as a pair of this type.
// int8_t will give a range of -127 to +128.
// uint8_t will give a range of 0 to 255.
using POINT_DATA_TYPE = uint8_t;


// If you want sound. U will need a buzzer.
#define SOUND NO
#if (SOUND == 0)
	#define tone(x, y, z)
#endif


// This makes all debugging print statements disappear when DEBUG set to NO.
#if (DEBUG == YES)
	#include "string.h"
	#define DEBUG_PRINT_FLASH(x) 	Serial.print(F(x))
	#define DEBUG_PRINTLN_FLASH(x) 	Serial.println(F(x))
	#define DEBUG_PRINT(x) 			Serial.print(x)
	#define DEBUG_PRINTLN(x) 		Serial.println(x)
	#define DEBUG_PRINT_HEX(x) 		Serial.print(x, HEX)
	#define DEBUG_PRINTLN_HEX(x) 	Serial.println(x, HEX)
#else
	#define DEBUG_PRINT_FLASH(x)
	#define DEBUG_PRINTLN_FLASH(x)
	#define DEBUG_PRINT(x)
	#define DEBUG_PRINTLN(x)
	#define DEBUG_PRINT_HEX(x)
	#define DEBUG_PRINTLN_HEX(x)
#endif // DEBUG



namespace Utility {

// A utility to tell if an integer type is signed or unsigned at runtime or compile time.
template <typename T>
struct is_signed {
	static constexpr bool value { (T(-1) < T(0)) }; 
};

template <typename T>
struct is_unsigned {
	static constexpr bool value { (T(-1) > T(0)) };
};


#if (DEBUG == YES)
// This class allows to print any unsigned (or positive) integer type in binary format.
//  It is here to aid in debugging. 
template<typename T>
struct Binary : public Printable {
    T value;
	constexpr Binary(T value) : value{value} { static_assert(is_unsigned<T>::value, "Needs to be unsigned.\n"); } 
	size_t printTo(Print& p) const;
};
#endif // (DEBUG == YES)
}


// The pin numbers.
namespace Pin {

	constexpr uint8_t UP 	{ 7 };
	constexpr uint8_t DOWN 	{ 8 };
	constexpr uint8_t LEFT 	{ 4 };
	constexpr uint8_t RIGHT { 2 };

#if (SOUND == YES)
	constexpr uint8_t SOUND { 9 };
#endif
}


namespace Display {

	constexpr uint8_t Width 	{ 128 };
	constexpr uint8_t Height 	{ 64 };
}


namespace World {

	// How large in pixels do you want the game rows and columns to be.
	constexpr uint8_t Scale { 6 };

	// Offsets top left and right.
	constexpr uint8_t xMinOffset { 4 };
	constexpr uint8_t xMaxOffset { 2 };
	constexpr uint8_t yMinOffset { 12 };
	constexpr uint8_t yMaxOffset { 2 };

	// How big the world is.
	constexpr uint8_t minX  { 0 };
	constexpr uint8_t maxX  { (Display::Width - xMinOffset - xMaxOffset) / Scale };
	constexpr uint8_t minY  { 0 };
	constexpr uint8_t maxY  { (Display::Height - yMinOffset - yMaxOffset) / Scale };

}


// All the directions you need.
// First 4 directions are reversible if bits are reversed.
// ie UP is 0b00 and down is 0b11.
enum class Direction : uint8_t {
    UP = 0, LEFT, RIGHT, DOWN, NONE
};

// Flipping the bits reverses the Direction.
Direction operator~(const Direction direction);

#if (DEBUG == YES)
String DirectionAsString(const Direction d);
#endif // (DEBUG == YES)


// Template function definitions are defined in the header.

namespace Utility {

#if (DEBUG == YES)
template<typename T>
size_t Binary<T>::printTo(Print& p) const {

	char binStr[sizeof(T) + 2];
	T b { this->value };
	T rev {0};

    for (uint16_t i{}; i < static_cast<uint16_t>(sizeof(T)) * 8; ++i) {
        rev = rev << 1;
        rev += (b % 2);
        b = b >> 1;
    }
    for (uint16_t i{}; i < static_cast<uint16_t>(sizeof(T)) * 8; ++i) {
    	binStr[i] = (rev % 2);
        rev = rev >> 1;
    }
	return p.print(binStr);
}
#endif // (DEBUG == YES)

}

#define __GLOBALS_HPP_
#endif // __GLOBALS_HPP_
