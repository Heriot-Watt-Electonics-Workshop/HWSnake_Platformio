
#include "error.hpp"

namespace Error {

Adafruit_SSD1306* displayPtr = nullptr;

void initErrors(Adafruit_SSD1306& display) {
	displayPtr = &display;
}

void displayError(int16_t line, const char* file, const char* msg) {

	if (displayPtr == nullptr) return;
	
	auto& d = *displayPtr;

	d.clearDisplay();
	d.setTextColor(WHITE);
	d.setCursor(0, 0);
	d.setTextSize(1);
	d.println("ERROR");
	d.println("Line: ");
	d.println(line);
	d.println("File: ");
	d.println(file);
	d.println(msg);
	d.display();
	delay(10000);
}
}