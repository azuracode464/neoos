#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdbool.h>

// Definições básicas
#define NULL 0
#define TIMER_FREQUENCY 1000  // 1000 Hz

// Funções de utilidade
void cli(void);
void sti(void);
void hlt(void);
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void io_wait(void);

// Funções de string
void strcpy(char* dest, const char* src);
int strcmp(const char* s1, const char* s2);
size_t strlen(const char* str);
int sprintf(char* buffer, const char* format, ...);

// Ponto de entrada do kernel
void _start(void);

#endif
