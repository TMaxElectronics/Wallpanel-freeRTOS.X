/*
 * File:   SerialComms.c
 * Author: Thorb
 *
 * Created on 16. Januar 2022, 22:19
 */


#include <xc.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "SerialComms.h"
#include "UART.h"
#include "TTerm.h"
#include "System.h"


static void SCOM_task(void * params);

static UART_PortHandle * uph = NULL;

void SCOM_init(){
    uph = UART_init(5, 115200, &RPD11R, 0b0011);
    UART_setRxDMAEnabled(uph, 1);
    UART_setModuleOn(uph, 1);
    
    TERM_handle = TERM_createNewHandle(UART_termPrint, (void *) uph, 1, &TERM_defaultList, 0, "root");
    xTaskCreate(SCOM_task, "Term", configMINIMAL_STACK_SIZE + 500, TERM_handle, tskIDLE_PRIORITY + 2, NULL);
}

void SCOM_eventHook(Event evt){
    char cBuffer[16];
    uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
    
    char buffer[50];
    
    switch(evt){
        
        case FS_CARD_CONNECTED:
            TERM_printDebug(TERM_handle, "SD card connected\r\n");
            break;
            
        case FS_CARD_DISCONNECTED:
            TERM_printDebug(TERM_handle, "SD card disconnected\r\n");
            break;
            
        case FS_CARD_INIT_SUCCESSFUL:
            TERM_printDebug(TERM_handle, "SD card initialised\r\n");
            break;
            
        case FS_CARD_INIT_FAILED:
            TERM_printDebug(TERM_handle, "SD card initialisation failed\r\n");
            break;
            
        default:
            TERM_printDebug(TERM_handle, "unknown Event received: %d\r\n", evt);
    }
}

static void SCOM_task(void * params){
    TERMINAL_HANDLE * termPtr = params;
    uint8_t c = 0;
    while(1){
        xStreamBufferReceive(uph->rxStream, &c, 1, portMAX_DELAY);
            
        TERM_processBuffer(&c, 1, termPtr);
        LATAINV = _LATA_LATA6_MASK;
    }
}