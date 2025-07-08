#include "kernel.h"
#include "kboard.h"
#include "memory.h"
#include "process.h"
#include "interrupt.h"
#include "timer.h"
#include "filesystem.h"
#include "network.h"
#include "sound.h"
#include "graphics.h"
#include "syscall.h"
#include "shell.h"
#include "driver.h"
#include "paging.h"
#include "debug.h"

// ============================================================================
// CONSTANTES E DEFINIÇÕES
// ============================================================================

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Cores VGA
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN 14
#define VGA_COLOR_WHITE 15

// Estados do sistema
#define SYSTEM_BOOTING 0
#define SYSTEM_RUNNING 1
#define SYSTEM_SHUTDOWN 2
#define SYSTEM_PANIC 3

// Níveis de privilégio
#define KERNEL_MODE 0
#define USER_MODE 3

// Tamanhos de buffer
#define BUFFER_SIZE 1024
#define COMMAND_BUFFER_SIZE 256

// ============================================================================
// ESTRUTURAS GLOBAIS
// ============================================================================

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

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================

static uint16_t* video_memory = (uint16_t*)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t terminal_color = VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4);
static system_info_t system_info;
static kernel_log_t kernel_log;
static char input_buffer[COMMAND_BUFFER_SIZE];
static int input_pos = 0;
static uint32_t kernel_ticks = 0;
static bool system_initialized = false;

// ============================================================================
// FUNÇÕES DE BAIXO NÍVEL
// ============================================================================

void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void io_wait(void) {
    outb(0x80, 0);
}

void cli(void) {
    asm volatile ("cli");
}

void sti(void) {
    asm volatile ("sti");
}

void hlt(void) {
    asm volatile ("hlt");
}

// ============================================================================
// FUNÇÕES DE VÍDEO E TERMINAL
// ============================================================================

void set_terminal_color(uint8_t color) {
    terminal_color = color;
}

void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void scroll_screen(void) {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        video_memory[i] = video_memory[i + VGA_WIDTH];
    }
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        video_memory[i] = (terminal_color << 8) | ' ';
    }
    cursor_y = VGA_HEIGHT - 1;
}

void clear_screen(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i] = (terminal_color << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~3;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            video_memory[cursor_y * VGA_WIDTH + cursor_x] = (terminal_color << 8) | ' ';
        }
    } else {
        video_memory[cursor_y * VGA_WIDTH + cursor_x] = (terminal_color << 8) | c;
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen();
    }

    update_cursor();
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

void print_hex(uint32_t value) {
    char hex_digits[] = "0123456789ABCDEF";
    char buffer[11] = "0x";
    
    for (int i = 7; i >= 0; i--) {
        buffer[i + 2] = hex_digits[(value >> (i * 4)) & 0xF];
    }
    buffer[10] = '\0';
    print_string(buffer);
}

void print_dec(uint32_t value) {
    char buffer[11];
    int i = 0;
    
    if (value == 0) {
        print_char('0');
        return;
    }
    
    while (value > 0) {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }
    
    for (int j = i - 1; j >= 0; j--) {
        print_char(buffer[j]);
    }
}

// ============================================================================
// SISTEMA DE LOG
// ============================================================================

void log_message(const char* level, const char* message) {
    if (kernel_log.current_entry >= kernel_log.max_entries) {
        kernel_log.current_entry = 0;
    }
    
    char* entry = kernel_log.entries[kernel_log.current_entry];
    int pos = 0;
    
    // Adiciona timestamp
    pos += sprintf(entry + pos, "[%d] ", kernel_ticks);
    
    // Adiciona nível
    pos += sprintf(entry + pos, "%s: ", level);
    
    // Adiciona mensagem
    sprintf(entry + pos, "%s", message);
    
    kernel_log.current_entry++;
    
    // Também imprime no terminal se sistema estiver inicializado
    if (system_initialized) {
        print_string("[");
        print_dec(kernel_ticks);
        print_string("] ");
        print_string(level);
        print_string(": ");
        print_string(message);
        print_string("\n");
    }
}

void log_info(const char* message) {
    log_message("INFO", message);
}

void log_warning(const char* message) {
    log_message("WARN", message);
}

void log_error(const char* message) {
    log_message("ERROR", message);
}

// ============================================================================
// GERENCIAMENTO DE SISTEMA
// ============================================================================

void init_system_info(void) {
    system_info.magic = 0xDEADBEEF;
    system_info.version = 0x00010000; // v1.0.0
    system_info.system_state = SYSTEM_BOOTING;
    system_info.uptime = 0;
    system_info.total_memory = 0;
    system_info.used_memory = 0;
    system_info.processes_count = 0;
    system_info.interrupts_count = 0;
    strcpy(system_info.boot_device, "hd0");
    strcpy(system_info.kernel_version, "NeoOS v1.0.0 [64-bit]");
}

void init_kernel_log(void) {
    kernel_log.log_level = 0;
    kernel_log.max_entries = 1000;
    kernel_log.current_entry = 0;
    
    for (int i = 0; i < 1000; i++) {
        kernel_log.entries[i][0] = '\0';
    }
}

void system_tick(void) {
    kernel_ticks++;
    system_info.uptime = kernel_ticks / TIMER_FREQUENCY;
    
    // Atualiza estatísticas do sistema
    system_info.used_memory = memory_get_used();
    system_info.processes_count = process_get_count();
    
    // Scheduler de processos
    process_scheduler();
}

// ============================================================================
// COMANDOS DO SISTEMA
// ============================================================================

void cmd_help(const char* args) {
    print_string("NeoOS Commands:\n");
    print_string("  help     - Show this help message\n");
    print_string("  clear    - Clear screen\n");
    print_string("  version  - Show system version\n");
    print_string("  uptime   - Show system uptime\n");
    print_string("  memory   - Show memory usage\n");
    print_string("  processes- List running processes\n");
    print_string("  shutdown - Shutdown system\n");
    print_string("  reboot   - Reboot system\n");
    print_string("  log      - Show kernel log\n");
    print_string("  test     - Run system tests\n");
}

void cmd_clear(const char* args) {
    clear_screen();
}

void cmd_version(const char* args) {
    print_string("NeoOS Kernel Version: ");
    print_string(system_info.kernel_version);
    print_string("\n");
    print_string("Build Date: ");
    print_string(__DATE__);
    print_string(" ");
    print_string(__TIME__);
    print_string("\n");
}

void cmd_uptime(const char* args) {
    print_string("System uptime: ");
    print_dec(system_info.uptime);
    print_string(" seconds (");
    print_dec(kernel_ticks);
    print_string(" ticks)\n");
}

void cmd_memory(const char* args) {
    print_string("Memory Usage:\n");
    print_string("  Total: ");
    print_dec(system_info.total_memory);
    print_string(" bytes\n");
    print_string("  Used: ");
    print_dec(system_info.used_memory);
    print_string(" bytes\n");
    print_string("  Free: ");
    print_dec(system_info.total_memory - system_info.used_memory);
    print_string(" bytes\n");
}

void cmd_processes(const char* args) {
    print_string("Running Processes:\n");
    process_list();
}

void cmd_shutdown(const char* args) {
    print_string("Shutting down system...\n");
    system_info.system_state = SYSTEM_SHUTDOWN;
    log_info("System shutdown initiated");
    
    // Cleanup de subsistemas
    filesystem_shutdown();
    network_shutdown();
    sound_shutdown();
    
    print_string("System halted.\n");
    while (1) {
        hlt();
    }
}

void cmd_reboot(const char* args) {
    print_string("Rebooting system...\n");
    log_info("System reboot initiated");
    
    // Reset via keyboard controller
    outb(0x64, 0xFE);
    hlt();
}

void cmd_log(const char* args) {
    print_string("Kernel Log (last 10 entries):\n");
    int start = (kernel_log.current_entry >= 10) ? kernel_log.current_entry - 10 : 0;
    
    for (int i = start; i < kernel_log.current_entry; i++) {
        print_string(kernel_log.entries[i]);
        print_string("\n");
    }
}

void cmd_test(const char* args) {
    print_string("Running system tests...\n");
    
    // Test memory
    print_string("Testing memory... ");
    if (memory_test()) {
        print_string("PASS\n");
    } else {
        print_string("FAIL\n");
    }
    
    // Test filesystem
    print_string("Testing filesystem... ");
    if (filesystem_test()) {
        print_string("PASS\n");
    } else {
        print_string("FAIL\n");
    }
    
    // Test processes
    print_string("Testing process management... ");
    if (process_test()) {
        print_string("PASS\n");
    } else {
        print_string("FAIL\n");
    }
    
    print_string("Tests completed.\n");
}

// ============================================================================
// PROCESSAMENTO DE COMANDOS
// ============================================================================

typedef struct {
    const char* name;
    void (*handler)(const char* args);
} command_handler_t;

command_handler_t commands[] = {
    {"help", cmd_help},
    {"clear", cmd_clear},
    {"version", cmd_version},
    {"uptime", cmd_uptime},
    {"memory", cmd_memory},
    {"processes", cmd_processes},
    {"shutdown", cmd_shutdown},
    {"reboot", cmd_reboot},
    {"log", cmd_log},
    {"test", cmd_test},
    {NULL, NULL}
};

void process_command(const char* command) {
    // Separa comando e argumentos
    char cmd[64];
    char args[192];
    int i = 0, j = 0;
    
    // Extrai comando
    while (command[i] && command[i] != ' ') {
        cmd[j++] = command[i++];
    }
    cmd[j] = '\0';
    
    // Pula espaços
    while (command[i] == ' ') i++;
    
    // Extrai argumentos
    j = 0;
    while (command[i]) {
        args[j++] = command[i++];
    }
    args[j] = '\0';
    
    // Procura comando
    for (int k = 0; commands[k].name; k++) {
        if (strcmp(cmd, commands[k].name) == 0) {
            commands[k].handler(args);
            return;
        }
    }
    
    // Comando não encontrado
    print_string("Unknown command: ");
    print_string(cmd);
    print_string("\nType 'help' for available commands.\n");
}

// ============================================================================
// TRATAMENTO DE ENTRADA
// ============================================================================

void handle_keyboard_input(char c) {
    if (c == '\n') {
        print_char('\n');
        if (input_pos > 0) {
            input_buffer[input_pos] = '\0';
            process_command(input_buffer);
            input_pos = 0;
        }
        print_string("> ");
    } else if (c == '\b') {
        if (input_pos > 0) {
            input_pos--;
            print_char('\b');
        }
    } else if (c >= 32 && c <= 126) {
        if (input_pos < COMMAND_BUFFER_SIZE - 1) {
            input_buffer[input_pos++] = c;
            print_char(c);
        }
    }
}

// ============================================================================
// TRATAMENTO DE PÂNICO
// ============================================================================

void panic(const char* message) {
    cli();
    
    system_info.system_state = SYSTEM_PANIC;
    set_terminal_color(VGA_COLOR_WHITE | (VGA_COLOR_RED << 4));
    clear_screen();
    
    print_string("*** KERNEL PANIC ***\n\n");
    print_string("Error: ");
    print_string(message);
    print_string("\n\n");
    
    print_string("System Information:\n");
    print_string("  Uptime: ");
    print_dec(system_info.uptime);
    print_string(" seconds\n");
    print_string("  Memory: ");
    print_dec(system_info.used_memory);
    print_string("/");
    print_dec(system_info.total_memory);
    print_string(" bytes\n");
    print_string("  Processes: ");
    print_dec(system_info.processes_count);
    print_string("\n");
    
    print_string("\nSystem halted. Please restart.\n");
    
    log_error("KERNEL PANIC occurred");
    
    while (1) {
        hlt();
    }
}

// ============================================================================
// INICIALIZAÇÃO DO KERNEL
// ============================================================================

void kernel_early_init(void) {
    // Inicializa estruturas básicas
    init_system_info();
    init_kernel_log();
    
    // Limpa tela
    clear_screen();
    
    // Banner de boot
    set_terminal_color(VGA_COLOR_LIGHT_CYAN | (VGA_COLOR_BLACK << 4));
    print_string("NeoOS Kernel [64-bit] - Initializing...\n");
    set_terminal_color(VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    
    log_info("Kernel early initialization started");
}

void kernel_init(void) {
    log_info("Initializing core subsystems");
    
    // Inicializa gerenciamento de memória
    print_string("Initializing memory management... ");
    if (memory_init()) {
        print_string("OK\n");
        system_info.total_memory = memory_get_total();
    } else {
        print_string("FAILED\n");
        panic("Memory initialization failed");
    }
    
    // Inicializa paginação
    print_string("Setting up paging... ");
    if (paging_init()) {
        print_string("OK\n");
    } else {
        print_string("FAILED\n");
        panic("Paging initialization failed");
    }
    
    // Inicializa sistema de interrupções
    print_string("Setting up interrupts... ");
    if (interrupt_init()) {
        print_string("OK\n");
    } else {
        print_string("FAILED\n");
        panic("Interrupt initialization failed");
    }
    
    // Inicializa timer
    print_string("Initializing timer... ");
    if (timer_init(TIMER_FREQUENCY)) {
        print_string("OK\n");
    } else {
        print_string("FAILED\n");
        panic("Timer initialization failed");
    }
    
    // Inicializa gerenciamento de processos
    print_string("Initializing process management... ");
    if (process_init()) {
        print_string("OK\n");
    } else {
        print_string("FAILED\n");
        panic("Process management initialization failed");
    }
    
    // Inicializa drivers
    print_string("Loading drivers... ");
    if (driver_init()) {
        print_string("OK\n");
    } else {
        print_string("FAILED\n");
        panic("Driver initialization failed");
    }
    
    // Inicializa teclado
    print_string("Initializing keyboard... ");
    if (kboard_init()) {
        print_string("OK\n");
    } else {
        print_string("FAILED\n");
        panic("Keyboard initialization failed");
    }
    
    // Inicializa sistema de arquivos
    print_string("Mounting filesystem... ");
    if (filesystem_init()) {
        print_string("OK\n");
    } else {
        print_string("FAILED\n");
        log_warning("Filesystem initialization failed, continuing without FS");
    }
    
    // Inicializa shell
    print_string("Starting shell... ");
    if (shell_init()) {
        print_string("OK\n");
    } else {
        print_string("FAILED\n");
        panic("Shell initialization failed");
    }
    
    // Habilita interrupções
    sti();
    
    system_info.system_state = SYSTEM_RUNNING;
    system_initialized = true;
    
    log_info("Kernel initialization completed successfully");
}

// ============================================================================
// PONTO DE ENTRADA PRINCIPAL
// ============================================================================

void _start(void) {
    // Inicialização precoce
    kernel_early_init();
    
    // Inicialização principal
    kernel_init();
    
    // Mensagem de boas-vindas
    print_string("\n");
    set_terminal_color(VGA_COLOR_LIGHT_GREEN | (VGA_COLOR_BLACK << 4));
    print_string("Welcome to NeoOS!\n");
    set_terminal_color(VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    print_string("Type 'help' for available commands.\n");
    print_string("> ");
    
    // Loop principal do kernel
    while (system_info.system_state == SYSTEM_RUNNING) {
        // Processa entrada do teclado
        kboard_poll();
        char c = kboard_get_char();
        if (c != 0) {
            handle_keyboard_input(c);
        }
        
        // Processa chamadas de sistema
        syscall_process();
        
        // Atualiza drivers
        driver_update();
        
        // Processa rede
        network_process();
        
        // Yield para scheduler
        process_yield();
        
        // Pequena pausa para evitar 100% CPU
        hlt();
    }
    
    // Se saiu do loop, sistema está sendo desligado
    panic("Main kernel loop exited unexpectedly");
}
