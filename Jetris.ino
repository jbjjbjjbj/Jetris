#include "drawing.h"
#include "sprites.h"

#define LEFT_PIN	2
#define RIGHT_PIN	4
#define DROP_PIN	11
#define ROTATE_PIN	10

#define CLICK_TIME	200
#define FALL_TIME	300

#define DEBUG_TO_SERIAL	 1
#define RENDER_TO_SERIAL 0

void setup() {
#if DEBUG_TO_SERIAL == 1 or RENDER_TO_SERIAL == 1
	Serial.begin(115200);
	Serial.println("Starting \n\n");
#endif

	/* Prepare the random generator */
	randomSeed( analogRead(0) );

	/* Init pins */
	pinMode(CS,  OUTPUT);
	pinMode(CLK, OUTPUT);
	pinMode(DATA_IN, OUTPUT);
	pinMode(LEFT_PIN, INPUT_PULLUP);
	pinMode(RIGHT_PIN, INPUT_PULLUP);
	pinMode(DROP_PIN, INPUT_PULLUP);
	pinMode(ROTATE_PIN, INPUT_PULLUP);

	initDisplays(0);
	initDisplays(1);

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

void initGame() {
	initBlock();
}

void gameLoop() {

	static long loop_time;

	handleButtons();

	if( millis() - loop_time < FALL_TIME) 
		return;
	
	/* Move the cur_block */
	if( checkCollision(0, 1) ){
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

	return checkCollision(0, 0);	
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
int checkCollision(int xMove, int yMove) {
	/* New position */
	int xNew = x_pos + xMove;
	int yNew = y_pos + yMove;

	for( int i = 0; i < cur_block->height; i++ ) {
		if( (cur_block->buff[i] >> xNew) & buttonLayer[i + yNew] )
			return 1;
	}

	/* Check if border is hit */
	if( xNew + cur_block->width > 8 || xNew < 0 || yNew + cur_block->height > 16 || yNew < 0 )
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
	while( !checkCollision(0, 1) ) {
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

			/* Increase score */
			score++;
		} else {
			i++;
		}
	}
}

void endGame() {
	/* Slowly clear screen */
	for( int i = 0; i < 16; i++ ) {
		buttonLayer[15 - i] = topLayer[15 - i] = 0;
		renderAll();
		delay(100);
	}
	
	/* Fill button of screen with score */
	int xFill = score % 8;
	int yFill = score / 8;
	int y = 0;
#if DEBUG_TO_SERIAL == 1
	Serial.print("Finished with score ");
	Serial.println(score);
#endif

	for( y = 0; y < yFill; y++ ){
		buttonLayer[15 - y] = 0xFF;
	}
	for( int x = 0; x < xFill; x++ ){
		buttonLayer[15 - y] = buttonLayer[15 - y] | ( 1 << x );
	}

	/* Replicate the DVD gliding logo thing */

	cur_block = &smiley;
	/* Velocity */
	int xV = 1, yV = 1;
	y_pos = 3;

	while(1){
		if( checkCollision(xV, 0) ) {
			xV *= -1;
		}
		if( checkCollision(0, yV) ) {
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

	render(0, 0);
	render(1, 8);
}

/* 
 * Input Handling
 */
unsigned long leftLast, rightLast, dropLast, rotateLast;

void handleButtons() {
	/* Read from switches */
	int leftState = !digitalRead(LEFT_PIN);
	int rightState = !digitalRead(RIGHT_PIN);
	int dropState = !digitalRead(DROP_PIN);
	int rotateState = !digitalRead(ROTATE_PIN);

	/* Check if last click is over 100 ms */
	if( leftState && millis() - leftLast > CLICK_TIME ) {
		leftLast = millis();

		/* Move left */
		if( !checkCollision(1, 0) )
			moveBlock(1, 0);
	}

	/* Repeat for others buttons */
	if( rightState && millis() - rightLast > CLICK_TIME ) {
		rightLast = millis();
		if( !checkCollision(-1, 0) )
			moveBlock(-1, 0);
	}

	if( dropState && millis() - dropLast > CLICK_TIME + 100 ){
		dropLast = millis();
		dropBlock();
	}

	if( rotateState && millis() - rotateLast > CLICK_TIME + 100 ){
		rotateLast = millis();
		rotateBlock();
	}
}
