#ifndef UART
#define UART

#include <stdint.h>
#include <stddef.h>


volatile uint32_t* getRegister(const volatile uint32_t* base, uint32_t offset);
void waitForTX();
int setupUART();
int puts(const char* data, size_t size);

#endif