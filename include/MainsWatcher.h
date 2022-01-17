#include <stdint.h>

void MAINS_init();
uint8_t MAINS_watch_handleInput(TERMINAL_HANDLE * handle, uint16_t c);
void MAINS_watch_task(void *pvParameters);
uint8_t MAINS_watch(TERMINAL_HANDLE * handle, uint8_t argCount, char ** args);