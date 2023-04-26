// HWSnake game for 1st Year PRAXIS course @
// Heriot Watt University, Edinburgh. Scotland
// Build your own Arduino and OLED display shield.
// Coded by Will W from the EPS Electronics Workshop with too much abstraction
// and too much C++, mostly for his own entertainment.
// Version 2.0 was created on the 22nd April 2023
// The snake should now grow to fill the entire screen if you are good enough.
// Previous version was Version 1.0 on the 20th November 2021

// Dependencies are the SSD1306 library from Adafruit.
// and 'TimerInterrupt' by Khoi Hoang.
// These can be installed from 'Tools/Manage Libraries' in the menubar above.


#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>			// To save hi-score.
// Timer Interrupt for button debounce.
#define USE_TIMER_1 true
#include "TimerInterrupt.h"
#include "assert.h"
#include "globals.hpp"
#include "Point.hpp"
#include "Snake.hpp"


namespace Display {

	constexpr const uint8_t Width 	{ 128 };
	constexpr const uint8_t Height 	{ 64 };
	
	// Initialize the display.
	Adafruit_SSD1306 display( Width, Height );  
}


// The gameworld space.
namespace World {

	// How large in pixels do you want the game rows and columns to be.
	constexpr const uint8_t Scale { 6 };

	// Offsets top left and right.
	constexpr const uint8_t xMinOffset { 4 };
	constexpr const uint8_t xMaxOffset { 2 };
	constexpr const uint8_t yMinOffset { 12 };
	constexpr const uint8_t yMaxOffset { 2 };

	// How big the world is.
	constexpr const uint8_t minX  { 0 };
	constexpr const uint8_t maxX  { (Display::Width - xMinOffset - xMaxOffset) / Scale };
	constexpr const uint8_t minY  { 0 };
	constexpr const uint8_t maxY  { (Display::Height - yMinOffset - yMaxOffset) / Scale };
	
	// Where the food is.
	Point<POINT_DATA_TYPE> scranPos {};

	// Get a random point.  This is a C++ lambda function.
	auto getRandomPoint { []() -> Point<POINT_DATA_TYPE> { 
		return { static_cast<POINT_DATA_TYPE>( random(World::minY, World::maxY) ),
				 static_cast<POINT_DATA_TYPE>( random(World::minX, World::maxX) ) }; 
	}};

	// Converts game coordinates to display coordinates.
	auto toWorld { [](const Point<POINT_DATA_TYPE>& p) -> Point<POINT_DATA_TYPE> {
		return {	static_cast<POINT_DATA_TYPE>((p.y * Scale) + yMinOffset),
					static_cast<POINT_DATA_TYPE>((p.x * Scale) + xMinOffset) };
	}};
}


Snake<SNAKE_DATA_SIZE, Point<POINT_DATA_TYPE>> snake {};


// Pressing the button sets this variable.  It is volatile as it is updated from user input.
volatile Direction lastDirectionPressed { Direction::NONE };


namespace Score {

	uint16_t current { 0 };
	// Highscore is read from the EEPROM non-volatile memory.
	uint16_t high 	 { ((EEPROM.read(0) != 255) ? static_cast<uint16_t>(EEPROM.read(0) * 10) : 0) };
}


// Define a button.
struct Button {

	enum class State { pressed, notPressed };

	constexpr static uint8_t readingPeriod_ms { 2 }; 	// Time between button reads.
	constexpr static uint8_t triggerCount { 3 }; 		// Number of reads required to trigger a press. 

	constexpr Button(uint8_t pin, Direction direction) : pin{ pin }, direction{ direction } {}

	const uint8_t pin;			// The pin of the button.
	const Direction direction;	// The direction the button represents.

	mutable State state = State::notPressed;
	mutable uint8_t pressedCount { 0 };
	mutable uint8_t unPressedCount { 0 };
};



// Create buttons.
namespace Buttons {

	constexpr Button 	bUp		{ Pin::UP, Direction::UP },
						bDown	{ Pin::DOWN, Direction::DOWN },
						bLeft	{ Pin::LEFT, Direction::LEFT },
						bRight	{ Pin::RIGHT, Direction::RIGHT };

	constexpr Button const* All[] { &bUp, &bDown, &bLeft, &bRight };
}


// Sets the pace of the game.
namespace Timing {

	uint16_t gameUpdateTime_ms { 300 };						// This decreases as you score points.
	constexpr uint16_t gameUpdateTimeOnReset_ms { 300 };	// This is the value it resets to.
	unsigned long lastGameUpdatedTime { 0 };				// A counter used in the loop.
}



// ------------ Function Declarations ------------

/**
 * @brief Check if food eaten.
 * @return true if yes.
 */
inline bool detectPlayerAteScran();


/**
 * @brief Check if the player left the game area.
 * @return true if yes.
 */
inline bool detectPlayerOutOfArea(Point<POINT_DATA_TYPE> newHead);


/**
 * @brief Check if the player collided with himself.
 * @return false 
 */
inline bool detectSelfCollision(Point<POINT_DATA_TYPE> newHead);


/**
 * @brief Run the game over sequence.
 */
void doGameOver();


/**
 * @brief Draws a random Line
 * @param colour The colour to draw the line.
 */
void drawARandomLine(uint8_t colour = WHITE);


/**
 * @brief Draws the background for the game.
 */
void drawDisplayBackground();


/**
 * @brief Draw the food.
 */
void drawScran();


/**
 * @brief Draw the Snake.
 */
void drawSnake();


/**
 * @brief Place food at a random location.
 */
inline void placeRandomScran();


/**
 * @brief Read the button states and do debounce.
 */
void readButtons();


/**
 * @brief Reset snake, score and food.
 */
void resetGameParameters();


/**
 * @brief The game loop.
 */
void updateGame();


/**
 * @brief Update the score.
 */
void drawUpdatedScore();

/**
 * @brief Wait for a press at the start of the game.
 */
void doSplashScreen();


#if (DEBUG == YES)
/**
 * @brief Convert directions to string.
 * @param d The direction to convert.
 * @returns String represenatation of direction.
 */
const __FlashStringHelper* directionToString(Direction d);
#endif




//  Main Functions...

void setup() {

	using namespace Display;
	delay(Timing::gameUpdateTime_ms);

	ITimer1.init();
	if (ITimer1.attachInterruptInterval(Button::readingPeriod_ms, readButtons)) {
		DEBUG_PRINTLN_FLASH("Timer attached successfully.");
	}

	randomSeed(analogRead(0));
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

#if (DEBUG == 1)
	Serial.begin(9600);
#endif
    delay(Timing::gameUpdateTime_ms);
	// DEBUG_PRINT_FLASH("Size: ("); DEBUG_PRINT(World::maxX);
	// DEBUG_PRINT_FLASH(", "); DEBUG_PRINT(World::maxY);
	// DEBUG_PRINTLN_FLASH(")");

    display.clearDisplay();   				// start with a clean display
    display.setTextColor(WHITE);			// set up text color rotation size etc  
    display.setRotation(0); 
    display.setTextWrap(false);
    display.dim(0);         				// set the display brighness

	for (const auto* button : Buttons::All) {
		pinMode(button->pin, INPUT_PULLUP);
	}
    
	DEBUG_PRINTLN_FLASH("Setup Complete");
	DEBUG_PRINT_FLASH("GCC Version: ");
	DEBUG_PRINTLN(__VERSION__);
	DEBUG_PRINT_FLASH("C++ Version: ");
	DEBUG_PRINTLN(__cplusplus);

    doSplashScreen();    		// display the snake start up screen
	resetGameParameters();
}



void loop() {
	auto tNow { millis() };

	if (tNow - Timing::lastGameUpdatedTime > Timing::gameUpdateTime_ms) {
		DEBUG_PRINTLN_FLASH("\nTICK");
		DEBUG_PRINTLN_FLASH("SNAKE START:");
		DEBUG_PRINTLN(snake);
		updateGame();
		DEBUG_PRINTLN_FLASH("SNAKE END:");
		DEBUG_PRINTLN(snake);
		Timing::lastGameUpdatedTime = tNow;
	}
}



// This is called by the timer interrupt.
void readButtons() {

	using namespace Timing;

	for (auto& button : Buttons::All) {

  		if (!digitalRead(button->pin) ) {
    		button->unPressedCount = 0;

			if (++button->pressedCount >= Button::triggerCount / Button::readingPeriod_ms) {
    
      			if (button->state != Button::State::pressed) {
					lastDirectionPressed = button->direction;
        			button->state = Button::State::pressed;
    			}
    		}
		} else if (button->state == Button::State::pressed && (++button->unPressedCount >= Button::triggerCount / Button::readingPeriod_ms)) {

			button->state = Button::State::notPressed;
      		button->pressedCount = 0;
    	}
  	}
}



void resetGameParameters() {

// Can we just create a new Snake.
	lastDirectionPressed = Direction::NONE;
	snake = Snake<SNAKE_DATA_SIZE, Point<POINT_DATA_TYPE>> {};
//	snake.setDirection(Direction::NONE);
	Score::current = 0;
	snake.push(World::getRandomPoint());
	Timing::gameUpdateTime_ms = Timing::gameUpdateTimeOnReset_ms;
	Display::display.clearDisplay();
	drawDisplayBackground();	// Draw the whole display but only once.
	placeRandomScran();
}



void drawDisplayBackground() {

	//DEBUG_PRINTLN_FLASH("Updating Display");
	using namespace Display;

	display.setTextSize(0);
	display.setTextColor(WHITE);
	
	display.fillRect(0, 0, Display::Width - 1, 8, BLACK);
	
	// draw scores
	display.setCursor(2, 1);
	display.print(F("Score:"));
	display.print(Score::current);
	
	display.setCursor((Width / 2) + 2, 1);	
	display.print(F("High:"));
	display.print(Score::high);

	// draw play area
	//      pos  1x, 1y, 2x, 2y, colour
	display.drawLine(0, 0, Width - 1, 0, WHITE); 						// very top border
	display.drawLine((Width / 2) - 1, 0, (Width / 2) - 1, 9, WHITE); 	// score seperator
	display.fillRect(0, 9, Width - 1, 2, WHITE); 						// below text border
	display.drawLine(0, 0, 0, 9, WHITE);
	display.drawLine(Width - 1, 0, Width - 1, 9, WHITE); 

	display.fillRect(0, Height - 3, Width - 1, 3, WHITE);				// bottom border
	display.fillRect(0, 9, 3, Height - 1, WHITE); 						// left border
	display.fillRect(Width - 3, 9, 3, Height - 1, WHITE); 				// right border    
}



void updateGame() {
    
	using namespace Display;

	// Update the Snake's direction from button input.
	if (lastDirectionPressed != snake.getDirection()) {

#pragma "query this code"
		switch (lastDirectionPressed) {

			case Direction::UP:
				if (snake.getDirection() != Direction::DOWN)
					snake.setDirection(lastDirectionPressed);
				break;
			case Direction::DOWN:
				if (snake.getDirection() != Direction::UP)
					snake.setDirection(lastDirectionPressed);
				break;
			case Direction::LEFT:
				if (snake.getDirection() != Direction::RIGHT)
					snake.setDirection(lastDirectionPressed);
				break;
			case Direction::RIGHT:
				if (snake.getDirection() != Direction::LEFT)
					snake.setDirection(lastDirectionPressed);
			default: break;
		}
	}

	if (snake.getDirection() != Direction::NONE) {  // Nothing happens.

		// Move the Snake.
		// Save current head position.
		auto currentHead = snake.head();
		Point<POINT_DATA_TYPE> newHead {};

		switch (snake.getDirection()) { 

			case Direction::UP:
				newHead = {static_cast<POINT_DATA_TYPE>(currentHead.y - 1), currentHead.x };
				break;
			case Direction::DOWN:
				newHead = { static_cast<POINT_DATA_TYPE>(currentHead.y + 1), currentHead.x };
				break;
			case Direction::LEFT:
				newHead = { currentHead.y, static_cast<POINT_DATA_TYPE>(currentHead.x - 1) };
				break;
			case Direction::RIGHT:
				newHead = { currentHead.y, static_cast<POINT_DATA_TYPE>(currentHead.x + 1) };
				break;
			default: break;
		}

		if (detectPlayerOutOfArea(newHead) || detectSelfCollision(newHead)) {
			doGameOver();
			return;
		} else snake.push(newHead);

		if (detectPlayerAteScran()) { // If eating tail stays put and only head advances.
			drawUpdatedScore();
		} else {
			DEBUG_PRINTLN_FLASH("Removing tail.");
			auto removed = snake.pop();
			// best place to remove the tail.
			display.fillRect((removed.x * World::Scale) + World::xMinOffset, (removed.y * World::Scale) + World::yMinOffset, World::Scale, World::Scale, BLACK);
		}
	}
	DEBUG_PRINTLN_FLASH("Draw");
	drawSnake();
	display.display();
}



void doSplashScreen() {

	using namespace Display;
	display.clearDisplay();

	while (true) {

		auto tNow { millis() };

		if (tNow - Timing::lastGameUpdatedTime > Timing::gameUpdateTime_ms) {

			drawARandomLine(); // draw a random white line
			drawARandomLine(BLACK); // draw a random black line so that the screen not completely fill white

			display.fillRect(19, 20, 90, 32, BLACK); // blank background for text
			display.setTextColor(WHITE);
			display.setCursor(35, 25);
			display.setTextSize(2); // bigger font
			display.println(F("SNAKE"));
							//    x  y   w  h r  col
			display.drawRoundRect(33, 22, 62, 20, 4,WHITE);  // border Snake
			display.drawRect(19, 20, 90, 32, WHITE);         // border box  - 3
			display.setCursor(28, 42);
			display.setTextSize(0);  // font back to normal

			display.println(F("press any key"));
			display.display();
			
			Timing::lastGameUpdatedTime = tNow;
		}

		if (lastDirectionPressed != Direction::NONE) {
			lastDirectionPressed = Direction::NONE;
			break;
		}
	}
}



void drawARandomLine(uint8_t colour) {

	auto getRand { [](uint8_t max) -> uint8_t { return static_cast<uint8_t>(random(0, max)); } };

	Point start { getRand(Display::Height), getRand(Display::Width) };
	Point end {	  getRand(Display::Height), getRand(Display::Width) };
	
	Display::display.drawLine(start.x, start.y, end.x, end.y, colour);
}


void drawScran() {

	auto& d = Display::display;
	using namespace World;
	
	d.drawRect ( ( scranPos.x * Scale ) + xMinOffset,
				 ( scranPos.y * Scale ) + yMinOffset,
				   Scale,
				   Scale,
				   WHITE
				);
}


void drawSnake() {
	// Only draws what is changed.
	auto& d = Display::display;
	using namespace World;

	auto headPos = toWorld(snake.head());
	// draw the head.
	d.fillRect( headPos.x, headPos.y, Scale, Scale, WHITE );

	// We don't want to draw all.  We want to draw the one after the head. and the last 2.
	if (snake.length() == 1) return;
	if (snake.length() > 1) {
		auto tailPos = toWorld(snake.tail());
		d.fillRect( tailPos.x, tailPos.y, Scale, Scale, BLACK);
		d.fillRect( tailPos.x + 3 , tailPos.y + 3, Scale - 3, Scale - 3, WHITE);
	}
	if (snake.length() > 2) {
		auto pos = toWorld(snake[snake.length() - 2]);
		d.fillRect( pos.x, pos.y, Scale, Scale, BLACK);
		d.fillRect( pos.x + 2, pos.y + 2, Scale - 2, Scale - 2, WHITE);
	}
	if (snake.length() > 3) {
		auto pos = toWorld(snake[1]);
		d.fillRect( pos.x, pos.y, Scale, Scale, BLACK);
		d.fillRect( pos.x + 1, pos.y + 1, Scale - 1, Scale - 1, WHITE);
	}
}

void drawUpdatedScore() {
	using namespace Display;
	// draw scores
	display.fillRect(36, 1, 27, 8, BLACK);
	display.setCursor(38, 1);
	display.print(Score::current);
}


void placeRandomScran() {

	using namespace World;

// This is a C++ lambda function.
	static auto isInSnake = [](Point<POINT_DATA_TYPE>& point)->bool {
		for (uint16_t i = 0; i < snake.length(); ++i) {
			if (point == snake[i]) { 
				return true; 
			};
		}
		return false;
	};

	do {
		scranPos = getRandomPoint();
		DEBUG_PRINT_FLASH("scranpos: "); DEBUG_PRINTLN(scranPos);
	} while (isInSnake(scranPos));

	// Draw scran here cause only want draw once.
	drawScran();
}


//#pragma message "detect ate scran and related game updates should be separate."
bool detectPlayerAteScran() {
	if (snake.head() == World::scranPos) {

	DEBUG_PRINTLN_FLASH("Player ate scran");
		Score::current += 10;

		if (Score::current % 100 == 0)  
			Timing::gameUpdateTime_ms -= 30;             
		
		tone(Pin::SOUND, 2000, 10);
		placeRandomScran();

		return true;
	}

	return false;
}



bool detectSelfCollision(Point<POINT_DATA_TYPE> newHead) {

	for (uint16_t i = 0; i < snake.length(); ++i) {
		
		if (newHead == snake[i]) {

			tone(Pin::SOUND, 2000, 20);
			tone(Pin::SOUND, 1000, 20);
			DEBUG_PRINTLN_FLASH("Detected self collision.");
			return true;
		}
	}
	return false;
}



bool detectPlayerOutOfArea(Point<POINT_DATA_TYPE> newHead) {

#if (DEBUG == 1)
    auto rVal = (( newHead.x >= World::maxX  ) || ( newHead.y >= World::maxY ));
	if (rVal) {
		char pos[35];
		sprintf(pos, "head pos: (%u, %u)", newHead.x / World::Scale, newHead.y / World::Scale);
		DEBUG_PRINTLN(pos);
		sprintf(pos, "World min/max: (%u, %u), (%u, %u)", World::minX, World::minY, World::maxX, World::maxY);
		DEBUG_PRINTLN(pos);
		DEBUG_PRINTLN_FLASH("Detected out of area");
	}
	return rVal;
#else
	return (( newHead.x >= World::maxX ) || ( newHead.y >= World::maxY ));
#endif
}



void doGameOver() {
    
	using namespace Display;
	using namespace World;
	
	// Flash the snake
	bool on = false;
	uint8_t dly = 60;

	for (uint8_t i = 0; i < 17; ++i) {
		if (!on) {
			for (uint16_t i = 0; i < snake.length(); ++i) {
				auto pos = toWorld(snake[i]);
				display.fillRect(pos.x, pos.y, Scale, Scale, BLACK);
			}
		} else {
			auto head = toWorld(snake.head());
			display.fillRect(head.x, head.y, Scale, Scale, WHITE);
			for (uint16_t i = 0; i < snake.length() - 1; ++i) {
				
				auto pos = toWorld(snake[i]);
				
				if (i == snake.length() - 2) {
					display.fillRect(pos.x + 2, pos.y + 2, Scale - 2, Scale - 2, WHITE);

				} else if (i == snake.length() - 1) {
					display.fillRect(pos.x + 3, pos.y + 3, Scale - 3, Scale - 3, WHITE);

				} else {
					display.fillRect(pos.x + 1, pos.y + 1, Scale - 1, Scale - 1, WHITE);
				}
			}
		}
		display.display();
		on = !on;
		delay(dly);
		dly -= 4;
	}
	delay(350);

	uint8_t rectX1 { 38 }, rectY1 { 28 }, rectX2 { 58 }, rectY2 { 12 };
    
	display.clearDisplay();
    display.setCursor(40, 30);
    display.setTextSize(1);
    
	tone(Pin::SOUND, 2000, 50);
    display.print(F("GAME "));
    delay(500);
	tone(Pin::SOUND, 1000, 50);
    display.print(F("OVER"));

	if (Score::current > Score::high) {
		Score::high = Score::current;
		EEPROM.write(0, Score::high / 10);
	}

    for (uint8_t i = 0; i <= 16; ++i) { // this is to draw rectangles around game over

		display.drawRect(rectX1, rectY1, rectX2, rectY2, WHITE);
		display.display();

		rectX1 -= 2;      // shift over by 2 pixels
		rectY1 -= 2;
		rectX2 += 4;      // shift over 2 pixels from last point
		rectY2 += 4;

		tone(Pin::SOUND, i * 200, 3);
	}
    
	display.display();          

	rectX1 = 0;   // set start position of line
	rectY1 = 0;
	rectX2 = 0;
	rectY2 = 63;

	for (uint8_t i = 0; i <= 127; i++) {
		
		display.drawLine(rectX1, rectY1, rectX2, rectY2, BLACK); 
		++rectX1;
		++rectX2;
		display.display();                          
    }
    
	display.clearDisplay();
	lastDirectionPressed = Direction::NONE;
	doSplashScreen();		// wait for player to start game
	resetGameParameters();
}



#if (DEBUG == 1)
const __FlashStringHelper* directionToString(Direction d) {

	switch (d) {
		case Direction::UP:
			return F("Up");
			break;
		case Direction::DOWN:
			return F("Down");
			break;
		case Direction::LEFT:
			return F("Left");
			break;
		case Direction::RIGHT:
			return F("Right");
			break;
		case Direction::NONE:
			return F("None");
			break;
		default: return F("error");
	}
}
#endif
