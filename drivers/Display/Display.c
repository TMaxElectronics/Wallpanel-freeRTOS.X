/*
 * File:   OLedDriver.c
 * Author: Thorb
 *
 * Created on March 9, 2019, 10:44 PM
 */


#include <xc.h>
#include <proc/p32mz1024efk100.h>
#include "Display.h"
#include "System.h"
#include <math.h>

unsigned DISPLAY_initialized = 0;
unsigned DIPLAY_opacityEnabled = 0;

volatile struct{
    uint16_t    xOffset;
    uint16_t    yOffset; 
    uint8_t     latchMode : 4;
    uint8_t     force     : 1;
} Display_Offset __attribute__((packed, address(0xE0080800), persistent)) = {.xOffset = 0, .yOffset = 0, .latchMode = 0};

typedef struct{
    uint8_t     x;
    uint8_t     y; 
    uint8_t     width;
    uint8_t     height;
    uint16_t    config;
    uint16_t    addressOffset;
} SpriteConfig __attribute__((packed));

volatile SpriteConfig DISPLAY_SpriteConfig[40] __attribute__((packed, address(0xE00B0000), persistent));

volatile uint16_t displayData[256][256][3] __attribute__ ((address(0xE0000000), persistent));
volatile uint16_t gammaData[256] __attribute__ ((address(0xE0080000), persistent));
volatile uint16_t DISPLAY_brightness __attribute__ ((address(0xE0080900), persistent));
volatile uint16_t DISPLAY_writeMode __attribute__ ((address(0xE0080a00), persistent));
volatile uint16_t DISPLAY_spriteLut[512][4] __attribute__ ((address(0xE00B0200), persistent));
volatile uint16_t DISPLAY_spriteData[16384] __attribute__ ((address(0xE00A0000), persistent));

uint8_t currLayer = 0;

static uint8_t DISPLAY_setGammaCommand(TERMINAL_HANDLE * handle, uint8_t argCount, char ** args){
    for(uint8_t currArg = 0;currArg<argCount; currArg++){
        if(strcmp(args[currArg], "-?") == 0){
            ttprintf("shows the status given to us by the bootloader");
            return TERM_CMD_EXIT_SUCCESS;
        }
    }
    
    if(argCount < 1){
        ttprintf("usage: setGamma [float]");
        return 1;
    }
    
    double val = atof(args[0]);
    
    DISPLAY_setGamma(val);
    ttprintf("set gamma to %f\r\n", val);
    
    return TERM_CMD_EXIT_SUCCESS;
}

static uint8_t DISPLAY_fill(TERMINAL_HANDLE * handle, uint8_t argCount, char ** args){
    for(uint8_t currArg = 0;currArg<argCount; currArg++){
        if(strcmp(args[currArg], "-?") == 0){
            ttprintf("shows the status given to us by the bootloader");
            return TERM_CMD_EXIT_SUCCESS;
        }
    }
    
    if(argCount == 3){
        uint16_t r = atoi(args[0]);
        uint16_t g = atoi(args[1]);
        uint16_t b = atoi(args[2]);
        for(uint32_t x = 0; x < 256; x++){
            for(uint32_t y = 0; y < 256; y++){
               displayData[x][y][0] = r;
               displayData[x][y][1] = g;
               displayData[x][y][2] = b;
            } 
        }
    }else if(argCount == 1){
        uint16_t val = atoi(args[1]);
        for(uint32_t x = 0; x < 256; x++){
            for(uint32_t y = 0; y < 256; y++){
               displayData[x][y][0] = val;
               displayData[x][y][1] = val;
               displayData[x][y][2] = val;
            } 
        }
    }else return TERM_CMD_EXIT_ERROR;
    
    return TERM_CMD_EXIT_SUCCESS;
}

void DISPLAY_init(){
    EBICS0 = 0x20000000;
    EBIMSK0 = 0b00000101000;    //EBITMGR0, SRAM, 64KB
    CFGEBIA = 0x8007ffff;  //19 addr lines
    
    CFGEBIAbits.EBIPINEN = 1;
    
    CFGEBIC = 0b11000000000011;
    CFGEBICbits.EBIRDYEN1 = 1;
    CFGEBICbits.EBIRDYLVL = 0;
    
    EBISMT0 = 0;
    EBISMT0bits.RDYMODE     = 1;
    EBISMT0bits.PAGEMODE    = 0;
    EBISMT0bits.TPRC        = 0;   //0+1 cc
    EBISMT0bits.TBTA        = 0;   //1 cc
    EBISMT0bits.TWP         = 0;    //0+1 cc
    EBISMT0bits.TWR         = 0;    //1 cc
    EBISMT0bits.TAS         = 1;    //2 cc
    EBISMT0bits.TRC         = 0;    //0+1 cc
    
    EBISMCONbits.SMDWIDTH0  = 0;    //16 bits
    DISPLAY_setWriteMode(0);
    DISPLAY_initialized = 1;
    DISPLAY_setGamma(1.7);
    DISPLAY_setBrightness(15);
    
    TERM_addCommand(DISPLAY_setGammaCommand, "setGamma", "sets display gamma", 0, &TERM_defaultList); 
    TERM_addCommand(DISPLAY_fill, "fillScreen", "fills the screen", 0, &TERM_defaultList); 
}

void Display_setLatchMode(uint8_t latchMode){
    if(DIPLAY_opacityEnabled) DISPLAY_setWriteMode(0);
    uint8_t o = Display_Offset.latchMode;
    Display_Offset.latchMode = latchMode & 0xf;
}

void Display_forceLatch(){
    if(DIPLAY_opacityEnabled) DISPLAY_setWriteMode(0);
    Display_Offset.force = 1;
}

void DISPLAY_shift(uint8_t xOffset, uint8_t yOffset){
    if(DIPLAY_opacityEnabled) DISPLAY_setWriteMode(0);
    Display_Offset.xOffset = xOffset;
    Display_Offset.yOffset = yOffset;
}

void DISPLAY_setPixel(uint8_t x, uint8_t y, uint16_t red, uint16_t green, uint16_t blue){
    if(DIPLAY_opacityEnabled) DISPLAY_setWriteMode(0);
    if(red < 0x100) displayData[y][x][0] = red;
    if(green < 0x100) displayData[y][x][1] = green;
    if(blue < 0x100) displayData[y][x][2] = blue;
}

void DISPLAY_setPixelOp(uint8_t x, uint8_t y, uint8_t red, uint8_t green, uint8_t blue, uint8_t op){
    if(!DIPLAY_opacityEnabled) DISPLAY_setWriteMode(1);
    displayData[y][x][0] = red | (op << 8);
    displayData[y][x][1] = green | (op << 8);
    displayData[y][x][2] = blue | (op << 8);
}

void DISPLAY_setBrightness(uint8_t b){
    if(DIPLAY_opacityEnabled) DISPLAY_setWriteMode(0);
    DISPLAY_brightness = b;    
}

void DISPLAY_setGamma(float gamma){
    if(DIPLAY_opacityEnabled) DISPLAY_setWriteMode(0);
    uint16_t gammaArray[256];
    uint16_t i = 0;
    
    if(gamma == 0) gamma = 0.1;
    
    for(; i< 256; i++) {
        gammaArray[i] = (uint16_t) (DISPLAY_maxBright * powf((float) i / (float) 255, gamma));
    }
    
    for(i = 0; i< 256; i++) {
        gammaData[i] = gammaArray[i];
        uint16_t c = gammaData[i];
        //if(c != gammaArray[i]) UART_print("%d should be %d (%d)\r\n", c, gammaArray[i], i);
    }
}

void DISPLAY_setWriteMode(uint8_t mode){
    DIPLAY_opacityEnabled = mode;
    uint8_t o = DISPLAY_writeMode;
    uint16_t temp = mode | 0xff00;
    DISPLAY_writeMode = temp;
    //UART_print("%d => %d should be %d, op=0x%04x\r\n", o, DISPLAY_writeMode, mode, temp);
}