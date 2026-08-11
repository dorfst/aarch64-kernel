#include "proc.h"
#include <stdint.h>


enum PROC_STATE {
    READY, RUNNING, DEAD
};

struct proc_context {};

struct proc {
    uint8_t pid;
    enum PROC_STATE state;
    uint64_t exec_time;
    struct proc_context* context;
    uint64_t* program_begin;
};

