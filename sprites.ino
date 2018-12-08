#include "sprites.h"

void initSprites() {
	iBlock.rotateNext = &iBlockR;
	iBlockR.rotateNext = &iBlock;

	oBlock.rotateNext = &oBlock;

	tBlock.rotateNext = &tBlockR;
	tBlockR.rotateNext = &tBlockD;
	tBlockD.rotateNext = &tBlockL;
	tBlockL.rotateNext = &tBlock;

	sBlock.rotateNext = &sBlockR;
	sBlockR.rotateNext = &sBlock;

	zBlock.rotateNext = &zBlockR;
	zBlockR.rotateNext = &zBlock;

	jBlock.rotateNext =  &jBlockR;
	jBlockR.rotateNext = &jBlockD;
	jBlockD.rotateNext = &jBlockL;
	jBlockL.rotateNext = &jBlock;

	lBlock.rotateNext =  &lBlockR;
	lBlockR.rotateNext = &lBlockD;
	lBlockD.rotateNext = &lBlockL;
	lBlockL.rotateNext = &lBlock;
}

