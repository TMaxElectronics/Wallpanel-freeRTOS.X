/*
 * File:   startup.c
 * Author: Thorb
 *
 * Created on 27. Oktober 2020, 08:45
 */

#include <xc.h>
#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "startup.h"
#include "System.h"
#include "SerialComms.h"
#include "FS.h"
#include "SPI.h"
#include "Display.h"

unsigned deviceReady = 0;
static void startupTask();
static void crcReset();
static uint32_t crcProc(uint8_t byte);
static void prvSetupHardware();
uint32_t BL_result;
static uint8_t CMD_getBLState(TERMINAL_HANDLE * handle, uint8_t argCount, char ** args);

void startServices(){
    BL_result = TMR2;
    TMR2 = 0;
    
    prvSetupHardware();
    
    SYS_startCPULoadTask();
    
    TERM_addCommand(CMD_getBLState, "getBLState", "shows the last bootloader exit code", 0, &TERM_defaultList);
    
    SPI_HANDLE * sdCardHandle = SPI_createHandle(2);
    SPI_init(sdCardHandle, &RPB3R, 0b0101, 5, 400000);
    //create the FS task. (checks for SD card connection/removal)
    xTaskCreate(FS_task, "fs Task", configMINIMAL_STACK_SIZE + 400, sdCardHandle , tskIDLE_PRIORITY + 1, NULL);
    //TODO optimize stack usage and figure out why it needs to be this large
    
    SCOM_init();
    DISPLAY_init();
    
    //xTaskCreate(startupTask, "startTsk", configMINIMAL_STACK_SIZE, NULL , tskIDLE_PRIORITY + 2, NULL);
}

//perform tasks that need to run before we can operate normally
static void startupTask(void * params){
    /*LATASET = 1;
    
    while(1){
        vTaskDelay(1000);
        LATAINV = 1;
        LATBINV = 1;
        USB_sendChar('f');
    }*/
    
    //kill ourselves, as our job is done. Nice
    vTaskDelete(xTaskGetCurrentTaskHandle());
    
    while(1);
}

uint32_t currCrc = 0xffffffff;

static void crcReset(){
    currCrc = 0xffffffff;
}

//thank you MIN, very cool
static uint32_t crcProc(uint8_t byte){
    currCrc ^= byte;
    uint32_t j = 0;
    for(j = 0; j < 8; j++) {
        uint32_t mask = (uint32_t) -(currCrc & 1U);
        currCrc = (currCrc >> 1) ^ (0xedb88320U & mask);
    }
    return currCrc;
}

static void prvSetupHardware(){
	/* Configure the hardware for maximum performance. */
	vHardwareConfigurePerformance();

	/* Setup to use the external interrupt controller. */
	vHardwareUseMultiVectoredInterrupts();

	portENABLE_INTERRUPTS();
    
    TRISDCLR = _TRISD_TRISD0_MASK | _TRISD_TRISD1_MASK | _TRISD_TRISD4_MASK | _TRISD_TRISD5_MASK | _TRISD_TRISD11_MASK | _TRISD_TRISD14_MASK;
    TRISACLR = _TRISA_TRISA0_MASK | _TRISA_TRISA6_MASK;
    TRISECLR = _TRISE_TRISE0_MASK | _TRISE_TRISE6_MASK;
    TRISBCLR = _TRISB_TRISB2_MASK;
    
    CNPUASET = _TRISA_TRISA7_MASK;
    CNPUBSET = _TRISB_TRISB0_MASK | _TRISB_TRISB1_MASK | _TRISB_TRISB3_MASK;
    CNPUGSET = _TRISG_TRISG12_MASK | _TRISG_TRISG13_MASK | _TRISG_TRISG14_MASK | _TRISG_TRISG15_MASK;
    
    ANSELA = 0;
    ANSELB = 0;
    ANSELD = 0;
    ANSELE = 0;
    ANSELG = 0;
    
    T4CON = 0b1000000001111000;
    T5CON = 0b1000000001111000;
    
    DMACON = _DMACON_ON_MASK;
}

static uint8_t CMD_getBLState(TERMINAL_HANDLE * handle, uint8_t argCount, char ** args){
    uint8_t currArg = 0;
    for(;currArg<argCount; currArg++){
        if(strcmp(args[currArg], "-?") == 0){
            ttprintf("shows the status given to us by the bootloader");
            return TERM_CMD_EXIT_SUCCESS;
        }
    }
    
    if(BL_result <= BOOTLOADER_EXIT_UPDATE_COMPLETE){
        ttprintf("BL_EXITCODE=%s\r\n", SYS_BOOTCODES[BL_result]);
    }else{
        ttprintf("BL_EXITCODE=%d (invalid)\r\n", BL_result);
    }
    
    return TERM_CMD_EXIT_SUCCESS;
}
