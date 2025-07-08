#include "types.h"
#ifndef KERNEL_H
#define KERNEL_H
// Definições básicas
#define NULL 0
#define TIMER_FREQUENCY 1000  // 1000 Hz

// Funções de utilidade
void cli(void);
void sti(void);
void hlt(void);
void outb(neo_u16 port, neo_u8 value);
neo_u8 inb(neo_u16 port);
void io_wait(void);

// Funções de string
void strcpy(char* dest, const char* src);
int strcmp(const char* s1, const char* s2);
neo_usize strlen(const char* str);
int sprintf(char* buffer, const char* format, ...);

// Ponto de entrada do kernel
void _start(void);
void print_string(const char* str);
#endif
