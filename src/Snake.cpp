#include "Snake.hpp"


Direction CrumbPtr::getValue() { 
	return static_cast<Direction>((*ptr >> ((3 - crumb) << 1)) & 0x03);
}
  
void CrumbPtr::putValue(Direction newval) {
      
 //     std::cout << "Byte before = 0x" << std::hex << static_cast<uint16_t>(*ptr) << '\n';
 //     std::cout << "Pushing new val: " << newval << " = 0x" << static_cast<uint16_t>(newval) << '\n';
		*ptr &= ~(0x03 << ((3 - crumb) << 1));
		*ptr |= (static_cast<uint8_t>(newval) << ((3 - crumb) << 1));
      
 //     std::cout << "curByte = 0x" << static_cast<uint16_t>(*ptr) << std::dec << '\n';
	}
  
CrumbPtr::selfType CrumbPtr::operator++() { 
	if (crumb == 3) { crumb = 0; ptr++; }
	else crumb++; 
	return *this;
}
	
CrumbPtr::selfType CrumbPtr::operator++(int) {
    	selfType rVal = *this;
    	if (crumb == 3) { crumb = 0; ptr++; } else crumb++;
    	return rVal;
}

CrumbPtr::selfType CrumbPtr::operator--() {
	if (crumb == 0) { crumb = 3; ptr--; }
	else crumb--;
	return *this; 
}

CrumbPtr::selfType CrumbPtr::operator--(int) {
	selfType rVal = *this;
	if (crumb == 0) { crumb = 3; ptr--; }
	return rVal;
}

