
#include <xc.h>
#include "System.h"

void FSSUP_sdEventHandler(uint32_t evtCode){
    printDebug("SD Card Event: %d\r\n", evtCode);
}
