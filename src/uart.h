#ifndef UART
#define UART

#include <stdint.h>
#include <stddef.h>


volatile uint32_t* getRegister(const volatile uint32_t* base, uint32_t offset);
void waitForTX();
int setupUART();
char nibble_to_hex(uint8_t nibble);
void uint64_t_to_string(char* str, uint64_t val);
int puts(const char* data, size_t size);

#endif