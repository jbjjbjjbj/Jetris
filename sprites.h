#ifndef sprites
#define sprites

#include <stdint.h>
#include <stddef.h>

struct Sprite {
	uint8_t buff[4];
	uint8_t width;
	uint8_t height;
	struct Sprite *rotateNext;
};



struct Sprite ball = {
	{
		0b11100000,
		0b10100000,
		0b11100000,
		0b00000000,
	}, 3, 3, NULL
};


struct Sprite iBlock = {
	{
		0b11110000,
		0b00000000,
		0b00000000,
		0b00000000,
	}, 4, 1, NULL
};
struct Sprite iBlockR = {
	{
		0b10000000,
		0b10000000,
		0b10000000,
		0b10000000,
	}, 1, 4, NULL
};


struct Sprite oBlock = {
	{
		0b11000000,
		0b11000000,
		0b00000000,
		0b00000000,
	}, 2, 2, NULL
};

struct Sprite tBlock = {
	{
		0b01000000,
		0b11100000,
		0b00000000,
		0b00000000,
	}, 3, 2, NULL
};
struct Sprite tBlockR = {
	{
		0b10000000,
		0b11000000,
		0b10000000,
		0b00000000,
	}, 2, 3, NULL
};
struct Sprite tBlockD = {
	{
		0b11100000,
		0b01000000,
		0b00000000,
		0b00000000,
	}, 3, 2, NULL
};
struct Sprite tBlockL = {
	{
		0b01000000,
		0b11000000,
		0b01000000,
		0b00000000,
	}, 2, 3, NULL
};


struct Sprite sBlock = {
	{
		0b01100000,
		0b11000000,
		0b00000000,
		0b00000000,
	}, 3, 2, NULL
};
struct Sprite sBlockR = {
	{
		0b10000000,
		0b11000000,
		0b01000000,
		0b00000000,
	}, 2, 3, NULL
};

struct Sprite zBlock = {
	{
		0b11000000,
		0b01100000,
		0b00000000,
		0b00000000,
	}, 3, 2, NULL
};
struct Sprite zBlockR = {
	{
		0b01000000,
		0b11000000,
		0b10000000,
		0b00000000,
	}, 2, 3, NULL
};


// JBLOCk
struct Sprite jBlock = {
	{
		0b10000000,
		0b11100000,
		0b00000000,
		0b00000000,
	}, 3, 2, NULL
};
struct Sprite jBlockR = {
	{
		0b11000000,
		0b10000000,
		0b10000000,
		0b00000000,
	}, 2, 3, NULL
};
struct Sprite jBlockD = {
	{
		0b11100000,
		0b00100000,
		0b00000000,
		0b00000000,
	}, 3, 2, NULL
};
struct Sprite jBlockL = {
	{
		0b01000000,
		0b01000000,
		0b11000000,
		0b00000000,
	}, 2, 3, NULL
};

//Lblock
struct Sprite lBlock = {
	{
		0b00100000,
		0b11100000,
		0b00000000,
		0b00000000,
	}, 3, 2, NULL
};
struct Sprite lBlockR = {
	{
		0b10000000,
		0b10000000,
		0b11000000,
		0b00000000,
	}, 2, 3, NULL
};
struct Sprite lBlockD = {
	{
		0b11100000,
		0b10000000,
		0b00000000,
		0b00000000,
	}, 3, 2, NULL
};
struct Sprite lBlockL = {
	{
		0b11000000,
		0b01000000,
		0b01000000,
		0b00000000,
	}, 3, 2, NULL
};

struct Sprite *blocks[] = { &iBlock, &oBlock, &tBlock, &sBlock, &zBlock, &jBlock, &lBlock };

#endif

