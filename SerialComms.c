/*
 * File:   SerialComms.c
 * Author: Thorb
 *
 * Created on 16. Januar 2022, 22:19
 */


#include <xc.h>
#include "FreeRTOS.h"
#include "task.h"

#include "UART.h"
#include "TTerm.h"
#include "System.h"


static void SCOM_task(void * params);

UART_PortHandle * uph = NULL;

void SCOM_init(){
    uph = UART_init(5, 115200, &RPD11R, 0b0011);
    UART_setModuleOn(uph, 1);
    TERM_handle = TERM_createNewHandle(UART_termPrint, (void *) uph, 1, &TERM_defaultList, 0, "root");
    xTaskCreate(SCOM_task, "Term", configMINIMAL_STACK_SIZE + 500, TERM_handle, tskIDLE_PRIORITY + 2, NULL);
}

static void SCOM_task(void * params){
    TERMINAL_HANDLE * termPtr = params;
    uint8_t c = 0;
    while(1){
        if(UART_available(uph)){
            c = UART_readChar(uph);
            TERM_processBuffer(&c, 1, termPtr);
            LATAINV = _LATA_LATA6_MASK;
        }
        vTaskDelay(1);
    }
}