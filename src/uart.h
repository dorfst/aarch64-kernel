#ifndef UART
#define UART

#include <stdint.h>
#include <stddef.h>


// https://krinkinmu.github.io/2020/11/29/PL011.html


volatile uint32_t* getRegister(const volatile uint32_t* base, uint32_t offset);
void waitForTX();
int setupUART();
int puts(const char* data, size_t size);

#endif