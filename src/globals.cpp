#include "globals.hpp"

#if (DEBUG == YES)
template<typename T>
size_t Binary<T>::printTo(Print& p) const {
	char binStr[sizeof(T) + 2];
	Binary<T> b { *this };
	
	T rev {0};

    for (int i = 0; i < sizeof(T) * 8; ++i) {
        rev = rev << 1;
        rev += (b.value % 2);
        b.value = b.value >> 1;
    }
    for (int i = 0; i < sizeof(T) * 8; ++i) {
    	binStr[i] = (rev % 2);
        rev = rev >> 1;
    }
	return p.print(binStr);
}
#endif // (DEBUG == YES)

Direction operator~(const Direction& direction) {
    return static_cast<Direction>(~static_cast<uint8_t>(direction) & 0x03);
}

#if (DEBUG == YES)
String DirectionAsString(Direction d) {
	switch (d) {
		case Direction::UP: return "UP"; break;
		case Direction::LEFT: return "LEFT"; break;
		case Direction::RIGHT: return "RIGHT"; break;
		case Direction::DOWN: return "DOWN"; break;
		case Direction::NONE: return "NONE"; break; 
		default: return "ERROR";
	}
}
#endif // (DEBUG == YES)
