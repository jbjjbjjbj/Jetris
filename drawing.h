/* Ensure only one definition */
#ifndef drawing
#define drawing

#define BUFF_HEIGHT   16
#define REDRAW_TIME   1000
#define NUM_DISPLAY   2
#define CLK           5
#define CS            6
#define DATA_IN       3

/* Layers */
uint8_t buttonLayer[16];
uint8_t topLayer[16];

/* Drawing to buffer */
void drawSprite(uint8_t layer[], uint8_t x, uint8_t y, struct Sprite* sprite);
void clearBuffer(uint8_t layer[] );

/* Screen interface commands */
void render(int display, unsigned int where);
void renderToSerial();
void initDisplays(int display);

#endif
