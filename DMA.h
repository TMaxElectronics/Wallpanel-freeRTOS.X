#ifndef DMA_INC
#define DMA_INC

#include <stdint.h>

typedef struct __DMA_ChannelHandle__ DMA_ChannelHandle;

void DMA_init();
DMA_ChannelHandle * DMA_initChannel();

void DMA_setSource(DMA_ChannelHandle * handle, void * src);

void DMA_setDestination(DMA_ChannelHandle * handle, void * src);

void DMA_setChannelAttributes(DMA_ChannelHandle * handle, uint32_t autoEnable, uint32_t priority, uint32_t allowEvtWhileDisabled);

void DMA_setChainingConfig(DMA_ChannelHandle * handle, uint32_t chainEnable, uint32_t chainDirection);

void DMA_setInterruptConfig(DMA_ChannelHandle * handle, uint32_t autoEnable, uint32_t priority);

void DMA_setPatternMatchConfig(DMA_ChannelHandle * handle, uint32_t matchEnabled, uint32_t patternLength);

void DMA_setTriggerAttributes(DMA_ChannelHandle * handle, uint32_t startIRQEn, uint32_t startIRQ);

void DMA_setAbortAttributes(DMA_ChannelHandle * handle, uint32_t abortIRQEn, uint32_t abortIRQ);

void DMA_setTransferAttributes(DMA_ChannelHandle * handle, uint32_t sourceSize, uint32_t destinationSize, uint32_t cellSize);

void DMA_setChannelEnabled(DMA_ChannelHandle * handle, uint32_t on);

void DMA_forceTransfer(DMA_ChannelHandle * handle);

void DMA_abortTransfer(DMA_ChannelHandle * handle);

typedef union{
    struct{
        uint32_t CHPRI:2;
        uint32_t CHEDET:1;
        uint32_t :1;
        uint32_t CHAEN:1;
        uint32_t CHCHN:1;
        uint32_t CHAED:1;
        uint32_t CHEN:1;
        uint32_t CHCHNS:1;
        uint32_t :2;
        uint32_t CHPATLEN:1;
        uint32_t :1;
        uint32_t CHPIGNEN:1;
        uint32_t :1;
        uint32_t CHBUSY:1;
        uint32_t :8;
        uint32_t CHPIGN:8;
    };
    struct{
        uint32_t w:32;
    };
} DCHxCON_t;

typedef union{ 
    struct{
        uint32_t :3;
        uint32_t AIRQEN:1;
        uint32_t SIRQEN:1;
        uint32_t PATEN:1;
        uint32_t CABORT:1;
        uint32_t CFORCE:1;
        uint32_t CHSIRQ:8;
        uint32_t CHAIRQ:8;
    };
    struct{
        uint32_t w:32;
    };
} DCHxECON_t;

typedef union{ 
    struct {
        uint32_t CHERIF:1;
        uint32_t CHTAIF:1;
        uint32_t CHCCIF:1;
        uint32_t CHBCIF:1;
        uint32_t CHDHIF:1;
        uint32_t CHDDIF:1;
        uint32_t CHSHIF:1;
        uint32_t CHSDIF:1;
        uint32_t :8;
        uint32_t CHERIE:1;
        uint32_t CHTAIE:1;
        uint32_t CHCCIE:1;
        uint32_t CHBCIE:1;
        uint32_t CHDHIE:1;
        uint32_t CHDDIE:1;
        uint32_t CHSHIE:1;
        uint32_t CHSDIE:1;
    };
    struct {
        uint32_t w:32;
    };
} DCHxINT_t;

struct __DMA_ChannelHandle__{
    volatile DCHxCON_t  * DCHxCON;
    volatile DCHxECON_t * DCHxECON;
    volatile DCHxINT_t  * DCHxINT;
    
    volatile uint32_t * DCHxSSA;
    volatile uint32_t * DCHxDSA;

    volatile uint32_t * DCHxSSIZ;
    volatile uint32_t * DCHxDSIZ;
    volatile uint32_t * DCHxCSIZ;

    volatile uint32_t * DCHxSPTR;
    volatile uint32_t * DCHxDPTR;
    volatile uint32_t * DCHxCPTR;
    
    volatile uint32_t * DCHxDAT;
};

#define DCHCON handle->DCHxCON->w
#define DCHCONbits (*handle->DCHxCON)

#define DCHECON handle->DCHxECON.w
#define DCHECONbits (*handle->DCHxECON)

#define DCHINT handle->DCHxINT.w
#define DCHINTbits (*handle->DCHxINT)

#define DCHSSA *(handle->DCHxSSA)
#define DCHDSA *(handle->DCHxDSA)

#define DCHSSIZ *(handle->DCHxSSIZ)
#define DCHDSIZ *(handle->DCHxDSIZ)
#define DCHCSIZ *(handle->DCHxCSIZ)

#define DCHSPTR *(handle->DCHxSPTR)
#define DCHDPTR *(handle->DCHxDPTR)
#define DCHCPTR *(handle->DCHxCPTR)

#define DCHDAT *(handle->DCHxDAT)

#endif