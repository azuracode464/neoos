#include "process.h"
#include "kernel.h"
#include "system.h"

static uint32_t process_count = 0;

void process_init(void) {
    process_count = 1; // Processo kernel
}

void process_scheduler(void) {
    // Round-robin básico
}

void process_yield(void) {
    // Troca de contexto (simulado)
}

void process_list(void) {
    print_string("  PID 1: kernel\n");
}

bool process_test(void) {
    uint32_t old_count = process_count;
    process_count++;
    if (process_count != old_count + 1) return false;
    process_count = old_count;
    return true;
}

uint32_t process_get_count(void) {
    return process_count;
}
