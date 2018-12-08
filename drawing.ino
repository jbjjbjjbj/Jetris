#include "drawing.h"
#include "sprites.h"
#include "maxCommands.h"

/* Draw sprite to screen 
 *
 *	layer:	16x8 bit buffer
 * 	x, y:	Position on screen
 * 	sprite:	Sprite to draw
 */
void drawSprite(uint8_t layer[], uint8_t x, uint8_t y, struct Sprite* sprite) {
	/* Check if in range */
	if( x + sprite->width > 8 || y + sprite->height > 16) 
		return;

	/* Or buffer with sprite */
	for (int i = y; i < sprite->height + y; i++) {
		layer[i] |= sprite->buff[i - y] >> x;
	}
}

/* Clear specified buffer */
void clearBuffer(uint8_t layer[] ) {
	for( int i = 0; i < BUFF_HEIGHT; i++ ) {
		layer[i] = 0;
	}
}

/* Render specfied screenbuffer
 *
 *	device:	Which screen
 *	where:	It render the next 8 bytes from this index
 */
void render(int device, uint8_t screenBuffer[], unsigned int where) {

	static uint8_t onScreen[BUFF_HEIGHT];
	static unsigned long reRenderLast;

	int i;
	uint8_t toWrite;

	/* Completely rerender every one second */
	bool reRender = (millis() - reRenderLast ) > REDRAW_TIME;

	/* Will draw 8 lines from /where/. */
	for(i = where; i < (where + 8); i++) {
		toWrite = screenbuffer[i];

		/* Check if whats on screen is the same */
		if(onScreen[i] == toWrite && !reRender) 
			continue;

		writeCommand(device, maxDIGIT_0 + i - where, toWrite);

		/* Keep track of whats on screen */
		onScreen[i] = toWrite;
	}

	if(reRender)
		reRenderLast = millis();
	
}

/* Write a register to specified display */
static void writeCommand(int display, uint8_t addr, uint8_t data) {
	/* Chip select low */
	digitalWrite(CS, LOW);

	/* Shift out the data, to the max shift registers
	 * MSBFIRST makes the MSB go first out */
	shiftOut(DATA_IN, CLK, MSBFIRST, addr);
	shiftOut(DATA_IN, CLK, MSBFIRST, data);

	/* If the device is number two, the commands should be shiftet through
	 * Each MAX, holds 16 bits(2 bytes) */
	serialPad(curDisplay * 2);

	digitalWrite(CS, HIGH);

	/* Before writing new commands, all the buffers should be cleaned */
	serialPad(NUM_DISPLAY * 2);

}

/* Init specified display */
void initDisplays(int display) {
	
}

/* Send x amount of empty bytes, recursively(for fun)*/
static void serialPad(int count) {
	shiftOut(DATA_IN, CLK, MSBFIRST, 0);

	if ( count )
		serialPad(count-1);
}

