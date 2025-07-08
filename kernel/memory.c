#include "types.h"
#include "memory.h"
#include "kernel.h"


struct SysInfo system_info;
struct Log kernel_log;
static neo_usize total_memory = 0;
static neo_usize used_memory = 0;

neo_usize memory_get_total(void) {
    return total_memory;
}

neo_usize memory_get_used(void) {
    return used_memory;
}

neo_bool memory_init(void) {
    // Simulação: 128MB de memória total
    total_memory = 128 * 1024 * 1024;
    used_memory = sizeof(kernel_log) + sizeof(system_info); // Memória usada por estruturas
    return neo_true;
}

neo_bool memory_test(void) {
    // Teste básico de alocação (simulado)
    neo_usize test_size = 4096;
    used_memory += test_size;
    if (used_memory > total_memory) {
        used_memory -= test_size;
        return neo_false;
    }
    used_memory -= test_size;
    return neo_true;
}
