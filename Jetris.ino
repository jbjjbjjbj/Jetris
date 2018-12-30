#include "drawing.h"
#include "sprites.h"

/* Compile settings */
#define DEBUG_TO_SERIAL	 1
#define RENDER_TO_SERIAL 1

/* Pin definitions */
#define LEFT_PIN	2
#define RIGHT_PIN	4
#define DROP_PIN	11
#define ROTATE_PIN	10
#define BUZZ_PIN	6

/* Timing */
#define CLICK_TIME	100
#define FALL_TIME	500
#define DIFF		0.98
#define REDRAW_TIME   250

void setup() {
	Serial.begin(115200);
	Serial.println("Starting \n\n");

	/* Prepare the random generator */
	randomSeed( analogRead(0) );
	
	/* First random is always 0(dont know why) */
	random(7);

	/* Init pins */
	pinMode(CS,  OUTPUT);
	pinMode(CLK, OUTPUT);
	pinMode(DATA_IN, OUTPUT);
	pinMode(LEFT_PIN, INPUT_PULLUP);
	pinMode(RIGHT_PIN, INPUT_PULLUP);
	pinMode(DROP_PIN, INPUT_PULLUP);
	pinMode(ROTATE_PIN, INPUT_PULLUP);

	/* Init displays */
	initDisplays(0);
	initDisplays(1);

	/* Init game */
	initSprites();
	initGame();
}

void loop() {
	gameLoop();
}


/* 
 * GAME LOGIC
 */

struct Sprite *cur_block;

int x_pos, y_pos, score;
int drop_speed = FALL_TIME;

void initGame() {
	initBlock();
}

void gameLoop() {

	static long loop_time;

	/* Combine inputs */
	handleInputs( inputFromButtons() | inputFromSerial() );

	if( millis() - loop_time < drop_speed) 
		return;
	
	/* Move the cur_block */
	if( checkCollision(0, 1, cur_block) ){
		drawSprite(buttonLayer, x_pos, y_pos, cur_block);

		/* Debug information */
#if DEBUG_TO_SERIAL == 1
		Serial.print("Writing block to ");
		Serial.print(x_pos);
		Serial.print(", ");
		Serial.println(y_pos);
#endif

		/* Handle the rows, from the button */
		handleFullRows(0);

		/* If full end the game */
		if( initBlock() )
			endGame();

		renderAll();
	}else {
		/* Free to move (one down)*/
		moveBlock(0, 1);
	}

	loop_time = millis();
}

/* Makes a new block and returns 1 if full */
int initBlock() {
	cur_block = blocks[ random(7) ];

	y_pos = 0;
	x_pos = 4 - cur_block->width/2;

	return checkCollision(0, 0, cur_block);	
}

void moveBlock(int x_move, int y_move) {
	x_pos += x_move;
	y_pos += y_move;

	/* Redraw ball */
	clearBuffer(topLayer);
	
	drawSprite(topLayer, x_pos, y_pos, cur_block);

	renderAll();
}

/* Returns 1 if there is collision in specified direction */
int checkCollision(int xMove, int yMove, struct Sprite *block) {
	/* New position */
	int xNew = x_pos + xMove;
	int yNew = y_pos + yMove;

	for( int i = 0; i < block->height; i++ ) {
		if( (block->buff[i] >> xNew) & buttonLayer[i + yNew] )
			return 1;
	}

	/* Check if border is hit */
	if( xNew + block->width > 8 || xNew < 0 || yNew + block->height > 16 || yNew < 0 )
		return 1;
	
	return 0;
}

int rotateBlock() {
	/* Switch to next cur_block if it exists */
	if( cur_block->rotateNext ) {
		cur_block = cur_block->rotateNext;
	}

	/* Render new cur_block(by abusing move block) */
	moveBlock(0, 0);
}

void dropBlock() {
	/* Move down until it hits something */
	while( !checkCollision(0, 1, cur_block) ) {
		moveBlock(0, 1);

		/* Add delay to make falling effect */
		delay(5);
	}
}

/* Check and delete full rows(starting at `start`) */
void handleFullRows(int start) {
	int i = start;
	while(i < 16) {
		/* If row is full */
		if( buttonLayer[15 - i] == 0xFF) {
			/* Drop all layers ontop */
			for( int j = i; j < 16; j++ ){
				buttonLayer[15 - j] = buttonLayer[14 - j];
			}

			/* Increase score and make sound */
			score++;
			tone(BUZZ_PIN, 880, 200);

			/* Lower falling speed */
			drop_speed *= DIFF;
		} else {
			i++;
		}
	}
}

void printScore(int score) {
	
	/* There are 8 dots per row. Find number of rows and whats left */
	int xFill = score % 8;
	int yFill = score / 8;
	int y = 0;

#if DEBUG_TO_SERIAL == 1
	Serial.print("Finished with score ");
	Serial.println(score);
#endif

	/* Fill out rows */
	for( y = 0; y < yFill; y++ ){
		buttonLayer[15 - y] = 0xFF;
	}
	/* Fill out leftover dots on last row */
	for( int x = 0; x < xFill; x++ ){
		buttonLayer[15 - y] = buttonLayer[15 - y] | ( 1 << x );
	}
}

const int endTone[] = {880, 622, 415, 415};

void endGame() {
	/* Slowly clear screen */
	for( int i = 0; i < 16; i++ ) {
		buttonLayer[15 - i] = topLayer[15 - i] = 0;
		renderAll();

		/* End tone*/
		if(i % 4 == 0) {
			tone(BUZZ_PIN, endTone[i/4]);
		}
		delay(100);
	}
	noTone(BUZZ_PIN);
	
	/* Fill button of screen with score */
	printScore(score);

	/* Replicate the DVD gliding logo thing */

	cur_block = &smiley;
	/* Velocity */
	int xV = 1, yV = 1;
	y_pos = 3;

	while(1){
		if( checkCollision(xV, 0, cur_block) ) {
			xV *= -1;
		}
		if( checkCollision(0, yV, cur_block) ) {
			yV *= -1;
		}

		moveBlock(xV, yV);
		delay(100);
	}
}

/* Draw first 8 bytes on first desplay and second 8 on last */
void renderAll() {
#if RENDER_TO_SERIAL == 1
	renderToSerial();
#endif

	static unsigned long reRenderLast;
	bool reRender = (millis() - reRenderLast ) > REDRAW_TIME;

	render(0, 0, reRender);
	render(1, 8, reRender);

	if( reRender ) 
		reRenderLast = millis();
}

/* 
 * Input Handling
 */
unsigned long leftLast, rightLast, dropLast, rotateLast;

#define LEFT_MASK	0b10000000
#define RIGHT_MASK	0b01000000
#define ROTATE_MASK 0b00100000
#define DROP_MASK	0b00010000

uint8_t inputFromButtons() {

	uint8_t is = 0;

	/* Read from switches */
	is |= LEFT_MASK * !digitalRead(LEFT_PIN);
	is |= RIGHT_MASK * !digitalRead(RIGHT_PIN);
	is |= ROTATE_MASK * !digitalRead(ROTATE_PIN);
	is |= DROP_MASK * !digitalRead(DROP_PIN);

	return is;
}

uint8_t inputFromSerial() {

	uint8_t is = 0;

	/* Read all input */
	while( Serial.available() ) {
		/* Read character */
		char c = Serial.read();

		switch( c ) {
			case 'd': 
				is |= LEFT_MASK;
				break;
			case 'a': 
				is |= RIGHT_MASK;
				break;
			case 'w': 
				is |= ROTATE_MASK;
				break;
			case ' ': 
				is |= DROP_MASK;
				break;
			default:
				/* Send help */
				Serial.println("a: left, d: right, w: rotate, space: drop");
		}
	}

	return is;
}

void handleInputs(uint8_t is) {

	/* Check if last click is over 100 ms */
	if( ( is & LEFT_MASK ) && millis() - leftLast > CLICK_TIME ) {
		leftLast = millis();

		/* Move left */
		if( !checkCollision(1, 0, cur_block) )
			moveBlock(1, 0);
	}

	/* Repeat for others buttons */
	if( ( is & RIGHT_MASK ) && millis() - rightLast > CLICK_TIME ) {
		rightLast = millis();
		if( !checkCollision(-1, 0, cur_block) )
			moveBlock(-1, 0);
	}

	if( ( is & DROP_MASK ) && millis() - dropLast > CLICK_TIME/2 ){
		dropLast = millis();
		if( !checkCollision(0, 1, cur_block) )
			moveBlock(0, 1);
	}

	if( ( is & ROTATE_MASK ) && millis() - rotateLast > CLICK_TIME*2 ){
		rotateLast = millis();
		if( !checkCollision(0, 0, cur_block->rotateNext ) )
			rotateBlock();
	}
}


