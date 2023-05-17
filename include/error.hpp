#ifndef __ERROR_HPP_
#define __ERROR_HPP_

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


namespace Error {

//Adafruit_SSD1306* displayPtr;
void initErrors(Adafruit_SSD1306& display);
void displayError(int16_t line, const char* file, const char* msg);
//void displayError(int16_t line, const char* file, const __FlashStringHelper* msg);

}

#endif // __ERROR_HPP_