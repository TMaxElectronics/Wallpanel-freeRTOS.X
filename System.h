#ifndef SYS_H
#define SYS_H

#include <xc.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"
#include "TTerm.h"

extern StreamBufferHandle_t TERM_rxBuff;

extern const char * SYS_BOOTCODES[];
extern const char SYS_fullBar[];
extern const char SYS_emptyBar[];

extern TERMINAL_HANDLE * TERM_handle;
#define printDebug(format, ...) TERM_printDebug(TERM_handle, format, ##__VA_ARGS__)

enum BOOTLOADER_CODE{
    BOOTLOADER_EXIT_NOSD,
    BOOTLOADER_EXIT_SD_INIT_FAIL,
    BOOTLOADER_EXIT_NO_BOOTFILE,
    BOOTLOADER_EXIT_INVALID_BOOTFILE,
    BOOTLOADER_EXIT_UPDATE_COMPLETE
};

//a non scheduler dependent wait function. Used in exception handlers where no interrupts are enabled
void SYS_waitCP0(uint16_t length);

//move a variable to the cachable section of ram
uint32_t * SYS_makeNonCoherent(uint32_t * coherent);
//move a variable to the non cachable section of ram
uint32_t * SYS_makeCoherent(uint32_t * nonCoherent);

uint32_t SYS_getCPULoadFine(TaskStatus_t * taskStats, uint32_t taskCount, uint32_t sysTime);
const char * SYS_getTaskStateString(eTaskState state);

#endif