// THings that miss
//	Functions that return something

#include "sprites.h"
#include "MaxCommands.h"

#define clk		 5
#define cs       6
#define dataIn 	 3
#define setDisplay(d) (curDisplay = d)

//Use the drawDot function to draw
uint8_t screenBuffer[16];

int curDisplay = 0;

void setup() {
	//Init Serial
	Serial.begin(9600);

	Serial.println("Starting up \n\n\n");

	//Init pins
	pinMode(cs,  OUTPUT);
	pinMode(clk, OUTPUT);
	pinMode(dataIn, OUTPUT);

	//Setup displays
	setDisplay(0);
	initDisplay();

	setDisplay(1);
	initDisplay();
	
	

}

/////////////////////////////
//       GAME LOGIC        //
/////////////////////////////

int ballX = 4;
int ballY = 0;

int ballXv = 1;
int ballYv = 1;

int last = 0;

Sprite *logo = blocks[4];

void loop() {
	delay(100);
	
	if(ballX + ballXv + logo->width > 8 || ballX + ballXv < 0){
		ballXv = -ballXv;
		logo = blocks[random(7)];
	}
	if(ballY + ballYv + logo->height > 16 || ballY + ballYv < 0){
		ballYv = -ballYv;
		logo = blocks[random(7)];
	}

	ballX += ballXv;
	ballY += ballYv;
	
	//Clear screen
	drawRegion(0xFF, 0xFFFF, false);

	//Draw ball
	drawSprite(ballX, ballY, logo);
	
	renderAll();
	//renderToSerial();


}

void renderAll(){
	setDisplay(0);
	render(screenBuffer);
	setDisplay(1);
	render(screenBuffer + 8);
}

void renderToSerial() {
	Serial.write(0x33); Serial.println("[2J");
	for(int i = 0; i < 16; i++) {
		for(int b = 0; b < 8; b++) {
			Serial.print( (  screenBuffer[i] & ( 1 << 7 - b ) ) > 0 );
		}
		Serial.println();
	}
}

/////////////////////////////
// Screendrawing routines  //
/////////////////////////////

void initDisplay() {

	//clear all registers
	for ( int i = 0; i < 0x0F; i++) {
		writeCommand(i, 0);
	}
	
	//Turn it on
	writeCommand(maxSHUTDOWN_INV, 1);

	//Darker please
	writeCommand(maxINTENSITY, 0x01);
	
	//Activate all lines
	writeCommand(maxSCAN_LIMIT, 0x07);

}

//Draw a region, where mask specifies where to draw.
//	xMask = 0b00111100, yMask = 0b01111110 will draw a small 4x6 box
void drawSprite(uint8_t x, uint8_t y, Sprite* sprite) {
	//Check if in range
	if (x + sprite->width - 1 > 7 || y + sprite->height - 1> 15 )
		return;
	
	//Clear out drawing area
	uint8_t xMask = (uint8_t)(~( (1 << ( 8 - sprite->width ) ) -1 )) >> x;
	uint16_t yMask =  ( (1 <<     sprite->height    ) -1 ) << y;

	drawRegion(xMask, yMask, false);


	for ( int i = y; i < sprite->height + y; i++) {
		//Calculate bits
		uint8_t row = screenBuffer[i];
		row |= sprite->buff[i - y] >> x;
		
		screenBuffer[i] = row;

	}
	
}

void drawDot(uint8_t x, uint8_t y, bool state) {
	//Check if in range
	if (x > 7 || y > 16) 
		return;
	//Generate masks
	uint8_t xMask = 1 << x;
	uint8_t yMask = 1 << y;

	//Draw the bit
	drawRegion(xMask, yMask, state);
}

void drawRegion(uint8_t xMask, uint16_t yMask, bool state) {
	for (int i = 0; i < 16; i++ ) {
		//If y index not in mask, go to next
		if (! ( yMask & 1 << i ) )
			continue;

		//Flip the bits
		uint8_t row = screenBuffer[i];
		if ( state ) {
			//Write 1 where on the 1's places in xMask
			row |= xMask;
		} else {
			//Write 0. ~ means bitwise NOT
			row &= ~( xMask );
		}
		screenBuffer[i] = row;
	}
}

void render(uint8_t buffer[]) {

	for(int i = 0; i < 8; i++ ) {
		writeCommand(maxDIGIT_0 + i, buffer[i]);
	}
	
	//Shift everything throught
	for( int i = 0; i < curDisplay; i++) {
		writeCommand(0, 0);
	}

}

void writeCommand(uint8_t addr, uint8_t data) {
	uint16_t uint8_tToWrite = addr << 8 | data;

	//Set Chip select low
	digitalWrite(cs, LOW);

	//MSBFIRST is a arduino standard, which says that data goes from left to right

	shiftOut(dataIn, clk, MSBFIRST, addr);
	shiftOut(dataIn, clk, MSBFIRST, data);

	//To get to other displays padding is added to step through them
	serialPad(curDisplay * 2);

	digitalWrite(cs, HIGH);
	
	//To Clear out other displays more padding is added
	serialPad(2 * 2);
}

void serialPad(int count) {

	for(int i = 0; i < count; i++) {
		//Pad with no-ops
		shiftOut(dataIn, clk, MSBFIRST, 0);
	}
}
