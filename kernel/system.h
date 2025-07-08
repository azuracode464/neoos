#include "types.h"
#ifndef SYSTEM_H
#define SYSTEM_H

typedef struct {
    neo_u32 magic;
    neo_u32 version;
    neo_u32 system_state;
    neo_u32 uptime;
    neo_u32 total_memory;
    neo_u32 used_memory;
    neo_u32 processes_count;
    neo_u32 interrupts_count;
    char boot_device[32];
    char kernel_version[64];
} system_info_t;

typedef struct {
    neo_u32 log_level;
    neo_u32 max_entries;
    neo_u32 current_entry;
    char entries[1000][128];
} kernel_log_t;

extern system_info_t system_info;
extern kernel_log_t kernel_log;

#endif
