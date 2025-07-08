#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
// Struct SysInfo com os membros esperados
struct SysInfo {
    int magic;
    int system_state;
    int total_memory;
    int used_memory;
    int processes_count;
    int interrupts_count;
    char boot_device[16];
    char kernel_version[32];
    int version;   
    int uptime;  
};

// Struct Log com os membros esperados
struct Log {
    int log_level;
    int max_entries;
    int current_entry;
    char entries[1000][256]; // ou o tamanho que você quiser por linha de log
};

// Declarações externas
extern struct SysInfo system_info;
extern struct Log kernel_log;

neo_usize memory_get_total(void);
neo_usize memory_get_used(void);
neo_bool memory_init(void);
neo_bool memory_test(void);

#endif // MEMORY_H
