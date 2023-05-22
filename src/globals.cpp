#include "globals.hpp"


Direction operator~(const Direction direction) {
	// Return inverted if in range 0-3;
	if (static_cast<uint8_t>(direction) > 0x03) return static_cast<Direction>(0x4);
    return static_cast<Direction>(~static_cast<uint8_t>(direction) & 0x03);
}

// Use __FlashStringHelper* DirectionAsString instead
//#if (DEBUG == YES)
// String DirectionAsString(const Direction d) {
// 	switch (d) {
// 		case Direction::UP: 	return "UP"; break;
// 		case Direction::LEFT: 	return "LEFT"; break;
// 		case Direction::RIGHT: 	return "RIGHT"; break;
// 		case Direction::DOWN: 	return "DOWN"; break;
// 		case Direction::NONE: 	return "NONE"; break; 
// 		default: return "ERROR";
// 	}
// }
//#endif // (DEBUG == YES)
