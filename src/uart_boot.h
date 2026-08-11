#ifndef UART_BOOT
#define UART_BOOT

#include <stdint.h>
#include <stddef.h>


// https://krinkinmu.github.io/2020/11/29/PL011.html


volatile uint32_t* getRegisterBoot(const volatile uint32_t* base, uint32_t offset);
void waitForTXBoot();
int setupBootUART();
int putsBoot(char* data, size_t size);

#endif