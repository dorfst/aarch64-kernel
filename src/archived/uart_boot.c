#include "../uart.h"

// https://krinkinmu.github.io/2020/11/29/PL011.html
// const virtual_address_offset = 0xffffffff00000000;
const volatile uint32_t* UART_BASE_BOOT = (uint32_t*) 0x09000000;
const uint32_t DR_OFFSET_BOOT= 0x000;
const uint32_t FR_OFFSET_BOOT = 0x018;
const uint32_t IBRD_OFFSET_BOOT = 0x024;
const uint32_t FBRD_OFFSET_BOOT = 0x028;
const uint32_t LCR_OFFSET_BOOT = 0x02c;
const uint32_t CR_OFFSET_BOOT = 0x030;
const uint32_t IMSC_OFFSET_BOOT = 0x038;
const uint32_t DMACR_OFFSET_BOOT = 0x048;


volatile uint32_t* getRegisterBoot(const volatile uint32_t* base, uint32_t offset) {
    return (volatile uint32_t*)((void*)base + offset);
}

void waitForTXBoot() {
    while((*getRegisterBoot(UART_BASE_BOOT, FR_OFFSET_BOOT) & (1 << 3)) != 0) {}
}

// enable uart, enable tx, disable dma controller, set baud rate
int setupBootUART() {
    uint32_t cr = *getRegisterBoot(UART_BASE_BOOT, CR_OFFSET_BOOT);
    uint32_t lcr = *getRegisterBoot(UART_BASE_BOOT, LCR_OFFSET_BOOT);
    uint32_t ibrd, fbrd;

    *getRegisterBoot(UART_BASE_BOOT, CR_OFFSET_BOOT) = cr | 0x0;

    waitForTXBoot();

    *getRegisterBoot(UART_BASE_BOOT, LCR_OFFSET_BOOT) = (lcr & ~(1 << 4));

    const uint32_t div = 4 * 24000000 / 115200;

    fbrd = div & 0x3f;
    ibrd = (div >> 6) & 0xffff;
    *getRegisterBoot(UART_BASE_BOOT, IBRD_OFFSET_BOOT) = ibrd;
    *getRegisterBoot(UART_BASE_BOOT, FBRD_OFFSET_BOOT) = fbrd;

    lcr = 0x0;
    lcr |= (7 & 0x3) << 5;

    *getRegisterBoot(UART_BASE_BOOT, IMSC_OFFSET_BOOT) = 0x7ff;
    *getRegisterBoot(UART_BASE_BOOT, DMACR_OFFSET_BOOT) = 0x0;
    *getRegisterBoot(UART_BASE_BOOT, CR_OFFSET_BOOT) = (1 << 8);
    *getRegisterBoot(UART_BASE_BOOT, CR_OFFSET_BOOT) = (1 << 8) | 1;

    return 0;
}

int putsBoot(char* data, size_t size) {
    waitForTXBoot();

    for (size_t i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            *getRegisterBoot(UART_BASE_BOOT, DR_OFFSET_BOOT) = '\r';
            waitForTXBoot();
        }
        *getRegisterBoot(UART_BASE_BOOT, DR_OFFSET_BOOT) = data[i];
        waitForTXBoot();
    }

    return 0;

}