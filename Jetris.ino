// THings that miss
//	Functions that return something
//  Make more safety

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

	initSprites();
	
	initGame();	

}

/////////////////////////////
//       GAME LOGIC        //
/////////////////////////////

Sprite *block;

int xPos;
int yPos;


long loopTime;

void initGame() {
	initBlock();
}

void loop() {
	
	handleButtons();

	if(millis() - loopTime < 500) 
		return;
	
	//Move
	if(checkCollision(0, 1)) {
		drawSprite(buttonLayer, xPos, yPos, block);	
		handleFullRows(0);
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

	block = blocks[random(7)];
}

void moveBlock(int xMove, int yMove) {
	xPos += xMove;
	yPos += yMove;

	//Clear screen
	drawRegion(topLayer, 0xFF, 0xFFFF, false);

	//Draw ball
	drawSprite(topLayer, xPos, yPos, block);

	//Render
	renderAll();

}

//Checks if input and button layer have bits in common, or if a block hits button. Returns true if collision
int checkCollision(int xMove, int yMove) {
	//Newolute position on map
	int yNew = yPos + yMove;
	int xNew = xPos + xMove;

	for(int i = 0; i < block->height; i++) {
		if(( block->buff[i] >> xNew) & buttonLayer[i + yNew])
			return 1;
	}

	//Check if out of bounds
	if(xNew + block->width > 8 || xNew < 0 || yNew + block->height > 16) 
		return 1;

	return 0;
}

void rotateBlock() {
	Serial.println("Rotate");

	//If next block exist, switch to that
	if( block->rotateNext ) {
		Serial.println((uint16_t)block);
		Serial.println((uint16_t)block->rotateNext);

		block = block->rotateNext;
		Serial.println((uint16_t)block);
	}

	//And render
	moveBlock(0, 0);
}

void dropBlock() {
	//Move down until it hits something
	int count = 0;
	while( !checkCollision(0, 1) ) {
		moveBlock(0, 1);
		delay(10);
	}

	drawSprite(buttonLayer, xPos, yPos, block);
}

void renderAll(){
	setDisplay(0);
	render(0);
	setDisplay(1);
	render(8);
}

void handleFullRows(int start) {
	for(int i = start; i < 16; i++ ) {
		if(buttonLayer[15 - i] == 0xFF) {
			for(int j = i; j < 16; j++) {
				buttonLayer[15 - j] = buttonLayer[14 - j];
			}
			handleFullRows(i);
			return;
		}
	}
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

	if(millis() - rotateLast > clickTime+100 && rotateState) {
		rotateLast = millis();

		rotateBlock();
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
void drawSprite(uint8_t layer[], uint8_t x, uint8_t y, Sprite* sprite) {
	//Check if in range
	if (x + sprite->width - 1 > 7 || y + sprite->height - 1> 15 )
		return;
	
	for ( int i = y; i < sprite->height + y; i++) {
		//Calculate bits
		uint8_t row = layer[i];
		row |= sprite->buff[i - y] >> x;
		
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

unsigned long reRenderLast;

void render(int where) {

	static uint8_t onScreen[16];

	bool reRender = (millis() - reRenderLast ) > 1000;

	for(int i = where; i < (where + 8); i++ ) {
		uint8_t toWrite = buttonLayer[i] | topLayer[i];
		if(onScreen[i] == toWrite && !reRender) 
			continue;

		writeCommand(maxDIGIT_0 + i - where, buttonLayer[i] | topLayer[i]);

		onScreen[i] = toWrite;
	}

	if(reRender) {
		reRenderLast = millis();
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
