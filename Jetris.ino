// THings that miss
//	Functions that return something
//  Make more safety
//  Malloc etc.

#include "sprites.h"
#include "MaxCommands.h"

#define clk		 5
#define cs       6
#define dataIn 	 3
#define setDisplay(d) (curDisplay = d)
#define leftPin     2
#define rightPin    4
#define dropPin  11
#define rotatePin 10
#define clickTime 200

//Use the drawDot function to draw
uint8_t buttonLayer[16];
uint8_t topLayer[16];

int curDisplay = 0;

void setup() {
	//Init Serial
	Serial.begin(9600);

	Serial.println("Starting up \n\n\n");

	randomSeed(analogRead(0));

	//Init pins
	pinMode(cs,  OUTPUT);
	pinMode(clk, OUTPUT);
	pinMode(dataIn, OUTPUT);
	pinMode(leftPin, INPUT_PULLUP);
	pinMode(rightPin, INPUT_PULLUP);
	pinMode(dropPin, INPUT_PULLUP);
	pinMode(rotatePin, INPUT_PULLUP);

	//ButtonInterrupt
	//attachInterrupt(digitalPinToInterrupt(left), buttonHandle, FALLING);

	//Setup displays
	setDisplay(0);
	initDisplay();

	setDisplay(1);
	initDisplay();
	
	initGame();	

}

/////////////////////////////
//       GAME LOGIC        //
/////////////////////////////

struct Sprite block;

int xPos;
int yPos;


unsigned long loopTime;

void initGame() {
	initBlock();
}

void loop() {
	
	handleButtons();

	if(millis() - loopTime < 500) 
		return;
	
	//Move
	if(checkCollision(0, 1)) {
		drawSprite(buttonLayer, xPos, yPos, &block);	
		renderAll();
		initBlock();
	}else{
		moveBlock(0, 1);
	}

	loopTime = millis();

}

void initBlock(){
	xPos = 0;
	yPos = 0;

	block = *blocks[random(7)];
}

void moveBlock(int xMove, int yMove) {
	xPos += xMove;
	yPos += yMove;

	//Clear screen
	drawRegion(topLayer, 0xFF, 0xFFFF, false);

	//Draw ball
	drawSprite(topLayer, xPos, yPos, &block);

	//Render
	renderAll();

}

//Checks if input and button layer have bits in common, or if a block hits button. Returns true if collision
int checkCollision(int xMove, int yMove) {
	//Newolute position on map
	int yNew = yPos + yMove;
	int xNew = xPos + xMove;

	for(int i = 0; i < block.height; i++) {
		if(( block.buff[i + block.yOff] >> xNew - block.xOff ) & buttonLayer[i + yNew])
			return 1;
	}

	//Check if out of bounds
	if(xNew + block.width > 8 || xNew < 0 || yNew + block.height > 16) 
		return 1;

	return 0;
}

void dropBlock() {
	//Move down until it hits something
	int count = 0;
	while( !checkCollision(0, 1) ) {
		moveBlock(0, 1);

	}

	drawSprite(buttonLayer, xPos, yPos, &block);
}

//Rotate sprite with the clock
void rotateSprite(struct Sprite *sprite, int count) {
	uint8_t oldBuffer[8];

	Serial.println(oldBuffer[0]);
	Serial.println(oldBuffer[2]);
	Serial.println(sprite->buff[0]);
	Serial.println(sprite->buff[2]);

	//VULN TODO
	memcpy(oldBuffer, sprite->buff, sizeof(oldBuffer) );

	Serial.println("Took copy");
	Serial.println(oldBuffer[0]);
	Serial.println(sprite->buff[0]);

	for(int i = 0; i < 8; i++) {
		sprite->buff[i] = 0;
	}

	//Cannot explain how this works in text TODO
	for(int i = 0; i < 8; i++) {
		uint8_t row = oldBuffer[7 - i];
		for(int j = 0; j < 8; j++) {
			sprite->buff[j] |= row & ( 1 << i );
		}
	}

	Serial.println("Rotated array");
	
	//Switch height/width
	uint8_t temp = sprite->height;
	sprite->height = sprite->width;
	sprite->width = temp;

	Serial.println("Changed dimensions");

}

void renderAll(){
	setDisplay(0);
	render(0);
	setDisplay(1);
	render(8);
}

void renderToSerial() {
	Serial.write(0x33); Serial.println("[2J");
	for(int i = 0; i < 16; i++) {
		for(int b = 0; b < 8; b++) {
			Serial.print( (  buttonLayer[i] & ( 1 << 7 - b ) ) > 0 );
		}
		Serial.println();
	}
}

/////////////////////////////
//   Control Handling      //
/////////////////////////////

unsigned long leftLast, rightLast, dropLast, rotateLast;

void handleButtons() {
	//Read from switch
	int leftState = !digitalRead(leftPin);
	int rightState = !digitalRead(rightPin);
	int dropState = !digitalRead(dropPin);
	int rotateState = !digitalRead(rotatePin);
	
	//Check if last click was over 100 ms ago
	if(millis() - leftLast > clickTime && leftState) {
		leftLast = millis();

		//Move 
		if( !checkCollision(1, 0) ) {
			moveBlock(1, 0);
		}
	}

	if(millis() - rightLast > clickTime && rightState) {
		rightLast = millis();
		//Move 
		if( !checkCollision(-1, 0) ) {
			moveBlock(-1, 0);
		}
	}

	if(millis() - dropLast > clickTime && dropState) {
		dropLast = millis();

		dropBlock();

	}

	if(millis() - rotateLast > clickTime && rotateState) {
		rotateLast = millis();

		//TODO collision checks
		rotateSprite(&block, 1);

		//Redraw moveblock
		moveBlock(0, 0);

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
	writeCommand(maxINTENSITY, 0x00);
	
	//Activate all lines
	writeCommand(maxSCAN_LIMIT, 0x07);

}

//Draw a region, where mask specifies where to draw.
//	xMask = 0b00111100, yMask = 0b01111110 will draw a small 4x6 box
void drawSprite(uint8_t layer[], uint8_t x, uint8_t y, struct Sprite* sprite) {
	//Check if in range
	if (x + sprite->width - 1 > 7 || y + sprite->height - 1> 15 )
		return;
	
	for ( int i = y; i < sprite->height + y; i++) {
		//Calculate bits
		uint8_t row = layer[i];
		row |= sprite->buff[i - y + sprite->yOff] >> x - sprite->xOff;
		
		layer[i] = row;

	}
	
}

void drawRegion(uint8_t layer[], uint8_t xMask, uint16_t yMask, bool state) {
	for (int i = 0; i < 16; i++ ) {
		//If y index not in mask, go to next
		if (! ( yMask & 1 << i ) )
			continue;

		//Flip the bits
		uint8_t row = layer[i];
		if ( state ) {
			//Write 1 where on the 1's places in xMask
			row |= xMask;
		} else {
			//Write 0. ~ means bitwise NOT
			row &= ~( xMask );
		}
		layer[i] = row;
	}
}

void render(int where) {

	for(int i = where; i < (where + 8); i++ ) {
		writeCommand(maxDIGIT_0 + i - where, buttonLayer[i] | topLayer[i]);
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
