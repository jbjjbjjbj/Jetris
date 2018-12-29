#include "drawing.h"
#include "sprites.h"

#define LEFT_PIN	2
#define RIGHT_PIN	4
#define DROP_PIN	11
#define ROTATE_PIN	10
#define CLICK_TIME	200

#define RENDER_TO_SERIAL 0

void setup() {
	Serial.begin(115200);
	Serial.println("Starting \n\n");

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

int x_pos, y_pos;

void initGame() {
	initBlock();
}

void gameLoop() {

	static long loop_time;

	handleButtons();

	if( millis() - loop_time < 500) 
		return;
	
	/* Move the cur_block */
	if( checkCollision(0, 1) ){
		drawSprite(buttonLayer, x_pos, y_pos, cur_block);

		/* Handle the rows, from the button */
		handleFullRows(0);

		initBlock();
		renderAll();
	}else {
		/* Free to move (one down)*/
		moveBlock(0, 1);
	}

	loop_time = millis();
}

void initBlock() {
	x_pos = y_pos = 0;

	cur_block = blocks[ random(7) ];
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
	if( xNew + cur_block->width > 8 || xNew < 0 || yNew + cur_block->height > 16 )
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

	/* Move to cur_block to the backlayer */
	drawSprite(buttonLayer, x_pos, y_pos, cur_block);
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
		} else {
			i++;
		}
	}
}

/* Draw first 8 bytes on first desplay and second 8 on last */
void renderAll() {
#if RENDER_TO_SERIAL == 1
	renderToSerial();
	return;
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
