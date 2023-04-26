#ifndef __GLOBALS_HPP_

#include "Arduino.h"

// Defined for readability.
#define YES 1
#define NO 0

// This switches on or off serial debug output.
#define DEBUG YES // or NO

#if (DEBUG == YES)
constexpr uint8_t SNAKE_DATA_SIZE { 1 };
#elif (DEBUG == NO)
constexpr uint8_t SNAKE_DATA_SIZE { 8 };
#endif

// Store Points as a pair of this type.
// int8_t will give a range of -127 to +128.
using POINT_DATA_TYPE = int8_t;

// If you want sound. U will need a buzzer.
#define SOUND NO
#if (SOUND == 0)
	#define tone(x, y, z)
#endif

// This makes all debugging code disappear when DEBUG set to NO.
#if (DEBUG == YES)
	#include "string.h"
	#define DEBUG_PRINT_FLASH(x) Serial.print(F(x))
	#define DEBUG_PRINTLN_FLASH(x) Serial.println(F(x))
	#define DEBUG_PRINT(x) Serial.print(x)
	#define DEBUG_PRINTLN(x) Serial.println(x)
	#define DEBUG_PRINT_HEX(x) Serial.print(x, HEX)
	#define DEBUG_PRINTLN_HEX(x) Serial.println(x, HEX)
#else
	#define DEBUG_PRINT_FLASH(x)
	#define DEBUG_PRINTLN_FLASH(x)
	#define DEBUG_PRINT(x)
	#define DEBUG_PRINTLN(x)
	#define DEBUG_PRINT_HEX(x)
	#define DEBUG_PRINTLN_HEX(x)
#endif // DEBUG



#if (DEBUG == YES)
template<typename T>
// This class allows to print any integer type in binary format. It is here to aid in debugging. 
struct Binary : public Printable {
    T value;
//    Binary(T value) : value {value} {}
	size_t printTo(Print& p) const;
};
#endif // (DEBUG == YES)


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


// All the directions you need.
// First 4 directions are reversible if bits are reversed.
// ie UP is 0b00 and down is 0b11.
enum class Direction : uint8_t {
    UP = 0, LEFT, RIGHT, DOWN, NONE
};

// Flipping the bits reverses the Direction.
Direction operator~(const Direction& direction);



#if (DEBUG == YES)
String DirectionAsString(Direction d);
#endif // (DEBUG == YES)




#define __GLOBALS_HPP_
#endif // __GLOBALS_HPP_