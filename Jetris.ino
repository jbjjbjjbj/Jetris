#include "drawing.h"
#include "sprites.h"

void setup() {
	Serial.begin(115200);
	Serial.println("Starting \n\n");

	/* Prepare the random generator */
	randomSeed( analogRead(0) );

	/* Init pins */
	pinMode(cs,  OUTPUT);
	pinMode(clk, OUTPUT);
	pinMode(dataIn, OUTPUT);
	pinMode(leftPin, INPUT_PULLUP);
	pinMode(rightPin, INPUT_PULLUP);
	pinMode(dropPin, INPUT_PULLUP);
	pinMode(rotatePin, INPUT_PULLUP);

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
	
	/* Move the block */
	if( checkCollision(0, 1) ){
		drawSprite(buttonLayer, xPos, yPos, cur_block);

		/* Handle the rows, from the top */
		handleFullRows(0);
		renderAll();
	}else {
		/* Free to move (one down)*/
		moveBlock(0, 1);
	}

	loop_time = millis();
}

void initBlock() {
	xPos = yPos = 0;

	block = blocks[ random(7) ];
}

void moveBlock(int x_move, int y_move) {
	x_pos += x_move;
	y_pos += y_move;

	/* Redraw ball */

	
}
