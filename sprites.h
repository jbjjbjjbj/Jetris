#ifndef sprites
#define sprites

struct Sprite {
	uint8_t buff[8];
	uint8_t width;
	uint8_t height;
};

const Sprite smiley = { 
	{ 
		0b11001100,
		0b11001100,
		0b00000000,
		0b00110000,
		0b10000000,
		0b11111100,
		0b00000000,
		0b00000000
	}, 6, 6
};

const Sprite checker = {
	{
		0b10101010,
		0b01010101,
		0b10101010,
		0b01010101,
		0b10101010,
		0b01010101,
		0b10101010,
		0b01010101
	}, 8, 8
};

const Sprite ball = {
	{
		0b11100000,
		0b10100000,
		0b11100000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000
	}, 3, 3
};

const Sprite miniSmiley = {
	{
		0b01010000,
		0b10000000,
		0b11110000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000
	}, 4, 3
};

#endif
