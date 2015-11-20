#if OPT_A2
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <elf.h>

struct pidStruct
{
    pid_t cur_pid;
    pid_t parent_pis;
    bool exited;
    int exitcode;
    struct cv *


