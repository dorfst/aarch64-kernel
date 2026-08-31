#ifndef PROC
#define PROC
#include <stdint.h>
enum PROC_STATE {
    READY, RUNNING, DEAD
};

struct proc_context {
    uint64_t* l1_table;
    uint64_t general_purpose[31];
    uint64_t pc;
    uint64_t sp;
    uint64_t spsr;
};

struct proc {
    uint8_t pid;
    enum PROC_STATE state;
    uint64_t exec_time;
    struct proc_context* context;
    uint64_t* program_begin;
};

#endif
