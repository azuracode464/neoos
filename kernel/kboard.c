#include "kernel.h"
#include "kboard.h"

#define PORT_KEYBOARD 0x60
#define PORT_KEYBOARD_STATUS 0x64
#define BUFFER_SIZE 256

static volatile char buffer[BUFFER_SIZE];
static volatile int write_pos = 0;
static volatile int read_pos = 0;

static char scancode_to_ascii(uint8_t scancode) {
    // Tabela básica (US QWERTY)
    static const char* lower = 
        "\0\0" "1234567890-=" "\0\0" "qwertyuiop[]" "\n\0" "asdfghjkl;'`" 
        "\0\\" "zxcvbnm,./" "\0\0" "\0 ";

    if (scancode < sizeof(lower)) {
        return lower[scancode];
    }
    return 0;
}

void kboard_init() {
    write_pos = 0;
    read_pos = 0;
}

void kboard_poll() {
    // Verifica se há dados disponíveis (bit 0 do status)
    if (inb(PORT_KEYBOARD_STATUS) & 0x01) {
        uint8_t scancode = inb(PORT_KEYBOARD);
        
        // Ignora releases (scancodes com bit 7 setado)
        if (!(scancode & 0x80)) {
            char c = scancode_to_ascii(scancode);
            if (c != 0) {
                buffer[write_pos] = c;
                write_pos = (write_pos + 1) % BUFFER_SIZE;
            }
        }
    }
}

char kboard_get_char() {
    if (read_pos == write_pos) {
        return 0;
    }
    char c = buffer[read_pos];
    read_pos = (read_pos + 1) % BUFFER_SIZE;
    return c;
}
