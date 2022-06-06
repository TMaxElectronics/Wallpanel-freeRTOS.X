#ifndef disp
#define disp

#include <stdint.h>

#define DISPLAY_maxBright 0x3ff

void DISPLAY_init();

//Sets the pixel at x,y to the RGB value. Rotation compensated
void DISPLAY_setPixel(uint8_t x, uint8_t y, uint16_t red, uint16_t green, uint16_t blue);

//Set pixel value at x.y with opacity (0-255)
void DISPLAY_setPixelOp(uint8_t x, uint8_t y, uint8_t red, uint8_t green, uint8_t blue, uint8_t op);

void DISPLAY_setGamma(float gamma);
void DISPLAY_setWriteMode(uint8_t mode);
void Display_setLatchMode(uint8_t latchMode);
void Display_forceLatch();
void DISPLAY_shift(uint8_t xOffset, uint8_t yOffset);
void DISPLAY_setBrightness(uint8_t b);

#endif