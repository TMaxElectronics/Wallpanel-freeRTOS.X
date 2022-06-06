#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <sys/kmem.h>

#include "FreeRTOSConfig.h"
#include "System.h"
#include "task.h"
#include "stream_buffer.h"
#include "TTerm.h"
#include "UART.h"
#include "sys/attribs.h"

const char SYS_fullBar[] = "=======================================================";
const char SYS_emptyBar[] = "-------------------------------------------------------";
const char * SYS_BOOTCODES[] = {"BOOTLOADER_EXIT_NOSD", "BOOTLOADER_EXIT_SD_INIT_FAIL", "BOOTLOADER_EXIT_NO_BOOTFILE", "BOOTLOADER_EXIT_INVALID_BOOTFILE", "BOOTLOADER_EXIT_UPDATE_COMPLETE"};

StreamBufferHandle_t TERM_rxBuff;
TERMINAL_HANDLE * TERM_handle = NULL;

//a non scheduler dependent wait function. Used in exception handlers where no interrupts are enabled
void SYS_waitCP0(uint16_t length){
    uint32_t currCount = _CP0_GET_COUNT();
    uint32_t targetTime = currCount + length * (configCPU_CLOCK_HZ / 2000);
    while(_CP0_GET_COUNT() < targetTime);
}

//move a variable to the non cachable section of ram
uint32_t * SYS_makeCoherent(uint32_t * nonCoherent){
    //make sure variable is in non coherent ram, else return NULL
    if(nonCoherent < __KSEG0_DATA_MEM_BASE || nonCoherent >= __KSEG0_DATA_MEM_BASE + __KSEG0_DATA_MEM_LENGTH) return NULL;
    return KVA0_TO_KVA1(nonCoherent);
}

//move a variable to the cachable section of ram
uint32_t * SYS_makeNonCoherent(uint32_t * coherent){
    //make sure variable is in non coherent ram, else return NULL
    if(coherent < KVA0_TO_KVA1(__KSEG0_DATA_MEM_BASE) || coherent >= KVA0_TO_KVA1(__KSEG0_DATA_MEM_BASE) + __KSEG0_DATA_MEM_LENGTH) return NULL;
    return KVA1_TO_KVA0(coherent);
}

uint32_t SYS_getCPULoadFine(TaskStatus_t * taskStats, uint32_t taskCount, uint32_t sysTime){
    uint32_t currTask = 0;
    volatile TaskStatus_t * ct = &taskStats[0];
    for(;currTask < taskCount; currTask++){
        ct = &taskStats[currTask];
        //configASSERT(ct > 0xa0000000 && ct < 0xa0010000);
        if(strncmp(taskStats[currTask].pcTaskName, "IDLE", configMAX_TASK_NAME_LEN) == 0){
            return 1000 - taskStats[currTask].avgCPULoad;
        }
    }
    return -1;
}

const char * SYS_getTaskStateString(eTaskState state){
    switch(state){
        case eRunning:
            return "running";
        case eReady:
            return "ready";
        case eBlocked:
            return "blocked";
        case eSuspended:
            return "suspended";
        case eDeleted:
            return "deleted";
        default:
            return "invalid";
    }
}

static void SYS_cpuLoadTask(void * params){
    uint32_t lastInstructionCount = portGET_INSTRUCTION_COUNTER_VALUE();
    
    while(1){
        vTaskDelay(pdMS_TO_TICKS(1000));
        vTaskDoCPULoadCalculationLoop();
        /*uint32_t currInstructionCount = portGET_INSTRUCTION_COUNTER_VALUE();
        uint32_t instructionsSinceLastTick = currInstructionCount - lastInstructionCount;
        
        
        //allocate memory
        uint32_t taskCount = uxTaskGetNumberOfTasks();
        uint32_t sysTime;
        TaskStatus_t * taskStats = pvPortMalloc( taskCount * sizeof( TaskStatus_t ));
        
        if(taskStats){
            //load task info for all tasks
            taskCount = uxTaskGetSystemState(taskStats, taskCount, &sysTime);
            
            //calculate load for each task
            vTaskEnterCritical(); //we can't be interrupted by any task at this point, otherwise the instruction count would be significantly wrong
            for(uint32_t currTask = 0;currTask < taskCount; currTask++){
                //load = instrThisTask / totalInstrCount;
                uint32_t cpuLoad = (1000 * taskStats->xHandle->ulInstructionsThisCycle) / instructionsSinceLastTick;
                taskStats->xHandle->ulAvgCPULoad = (taskStats->xHandle->ulAvgCPULoad * 15 + cpuLoad) >> 4;
                taskStats->xHandle->ulCurrentCPULoad = cpuLoad;
                taskStats->xHandle->ulInstructionsThisCycle = 0;
            }
            vTaskExitCritical();
            
            vPortFree(taskStats);
        }*/
    }
}

void SYS_startCPULoadTask(){
    xTaskCreate(SYS_cpuLoadTask, "CPU Load", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
}

uint32_t SYS_isInKSEG0RAM(void* ptr){
    return ((ptr >= __KSEG0_DATA_MEM_BASE) && (ptr < (__KSEG0_DATA_MEM_BASE + __KSEG0_DATA_MEM_LENGTH)));
}

uint32_t SYS_isInKSEG1RAM(void* ptr){
    return ((ptr >= KVA0_TO_KVA1(__KSEG0_DATA_MEM_BASE)) && (ptr < (KVA0_TO_KVA1(__KSEG0_DATA_MEM_BASE) + __KSEG0_DATA_MEM_LENGTH)));
}

uint32_t SYS_isInRAM(void* ptr){
    return SYS_isInKSEG0RAM(ptr) || SYS_isInKSEG1RAM(ptr);
}