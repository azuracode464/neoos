#include "kernel.h"
#include "kboard.h"

// Variáveis globais do kernel
static uint16_t* video_memory = (uint16_t*)0xB8000;
static int cursor_x = 0;
static int cursor_y = 0;

void clear_screen() {
    for (int i = 0; i < 80 * 25; i++) {
        video_memory[i] = 0x0700; // Fundo preto, texto branco
    }
    cursor_x = 0;
    cursor_y = 0;
}

void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        video_memory[cursor_y * 80 + cursor_x] = 0x0700 | c;
        cursor_x++;
    }

    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= 25) {
        // Rolagem de tela básica
        for (int i = 0; i < 24 * 80; i++) {
            video_memory[i] = video_memory[i + 80];
        }
        for (int i = 24 * 80; i < 25 * 80; i++) {
            video_memory[i] = 0x0700;
        }
        cursor_y = 24;
    }
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

void panic(const char* message) {
    print_string("KERNEL PANIC: ");
    print_string(message);
    while (1) { /* Loop infinito */ }
}

void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void _start() {
    clear_screen();
    print_string("NeoOS Kernel [64-bit] - Initializing...\n");
    
    // Inicializa o driver de teclado
    kboard_init();
    print_string("Keyboard driver initialized\n");
    
    print_string("System ready. Start typing:\n> ");
    
    // Loop principal do kernel
    while (1) {
        // Poll do teclado
        kboard_poll();
        
        // Verifica se há caracteres disponíveis
        char c = kboard_get_char();
        if (c != 0) {
            print_char(c);
        }
    }
}
