#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t system_state;
    uint32_t uptime;
    uint32_t total_memory;
    uint32_t used_memory;
    uint32_t processes_count;
    uint32_t interrupts_count;
    char boot_device[32];
    char kernel_version[64];
} system_info_t;

typedef struct {
    uint32_t log_level;
    uint32_t max_entries;
    uint32_t current_entry;
    char entries[1000][128];
} kernel_log_t;

extern system_info_t system_info;
extern kernel_log_t kernel_log;

#endif
