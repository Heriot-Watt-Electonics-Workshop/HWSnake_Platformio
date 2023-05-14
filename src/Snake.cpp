#include "Snake.hpp"

Direction CrumbPtr::getValue() { 
	return static_cast<Direction>((*ptr >> ((3 - crumb) << 1)) & 0x03);
}
  
void CrumbPtr::putValue(Direction newval) {
	*ptr &= ~(0x03 << ((3 - crumb) << 1));
	*ptr |= (static_cast<uint8_t>(newval) << ((3 - crumb) << 1));      
}


bool CrumbPtr::operator==(const CrumbPtr& other) const {
	return ((this->ptr == other.ptr) && (this->crumb == other.crumb));
}

bool CrumbPtr::operator!=(const CrumbPtr& other) const {
	return !(*this == other);
}  

// Prefix
CrumbPtr::selfType& CrumbPtr::operator++() { 
	if (crumb == 3) { crumb = 0; ptr++; }
	else crumb++; 
	return *this;
}
	
// Postfix
CrumbPtr::selfType CrumbPtr::operator++(int) {
    selfType rVal{*this};
    ++(*this);
	return rVal;
}

// Prefix
CrumbPtr::selfType& CrumbPtr::operator--() {
	if (crumb == 0) { crumb = 3; ptr--; }
	else crumb--;
	return *this; 
}

// Postfix
CrumbPtr::selfType CrumbPtr::operator--(int) {
	selfType rVal {*this};
	--(*this);
	return rVal;
}

#if (DEBUG == YES)
size_t CrumbPtr::printTo(Print& p) const {
	char addr[10];
	sprintf(addr, "%p:%u", ptr, crumb);
	return p.print(addr);
}

#endif // (DEBUG == YES)
