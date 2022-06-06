#ifndef SCOM_INC
#define SCOM_INC

typedef enum{
    GET_INFO = 1,   
    ETH_INIT_FAIL = 2,
    ETH_INIT_DONE = 3,
    ETH_LINK_UP = 4,
    ETH_LINK_DOWN = 5,
    ETH_DHCP_SUCCESS = 6,
    ETH_DHCP_FAIL = 7,
    FS_CARD_CONNECTED = 8,
    FS_CARD_DISCONNECTED = 9,
    FS_CARD_INIT_SUCCESSFUL = 10,
    FS_CARD_INIT_FAILED = 11,
}Event;

void SCOM_init();
void SCOM_eventHook(Event evt);

#define FS_SD_EVENT_HANDLER(X) SCOM_eventHook(X)

#endif