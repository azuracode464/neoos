#include "types.h"
#include "process.h"
#include "kernel.h"
#include "system.h"

static neo_u32 process_count = 0;

neo_bool process_init(void) {
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

neo_bool process_test(void) {
    neo_u32 old_count = process_count;
    process_count++;
    if (process_count != old_count + 1) return neo_false;
    process_count = old_count;
    return neo_true;
}

neo_u32 process_get_count(void) {
    return process_count;
}
