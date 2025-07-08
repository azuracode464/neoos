#include "memory.h"
#include "kernel.h"

static size_t total_memory = 0;
static size_t used_memory = 0;

size_t memory_get_total(void) {
    return total_memory;
}

size_t memory_get_used(void) {
    return used_memory;
}

bool memory_init(void) {
    // Simulação: 128MB de memória total
    total_memory = 128 * 1024 * 1024;
    used_memory = sizeof(kernel_log) + sizeof(system_info); // Memória usada por estruturas
    return true;
}

bool memory_test(void) {
    // Teste básico de alocação (simulado)
    size_t test_size = 4096;
    used_memory += test_size;
    if (used_memory > total_memory) {
        used_memory -= test_size;
        return false;
    }
    used_memory -= test_size;
    return true;
}
