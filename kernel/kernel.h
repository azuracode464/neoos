#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

// Definições básicas
#define NULL ((void*)0)
#define true 1
#define false 0
typedef unsigned char bool;

// Protótipos do kernel
void kernel_main();
void panic(const char* message);

// Funções de I/O (agora apenas protótipos)
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

// Funções de vídeo
void clear_screen();
void print_char(char c);
void print_string(const char* str);

#endif // KERNEL_H
