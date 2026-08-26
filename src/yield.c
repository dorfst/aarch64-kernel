#include "uart.h"

void yield_handler() {
    puts("yielding", sizeof("yielding"));
}
