#ifndef PROC
#define PROC

enum PROC_STATE {
    READY, RUNNING, DEAD
};

struct proc_context {
    uint64_t* l1_table;
    uint64_t general_purpose[30];
    uint64_t pc;
    uint64_t sp;
    uint64_t pstate;
};

struct proc {
    uint8_t pid;
    enum PROC_STATE state;
    uint64_t exec_time;
    struct proc_context* context;
    uint64_t* program_begin;
};

#endif
