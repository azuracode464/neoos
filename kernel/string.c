#include "kernel.h"

void strcpy(char* dest, const char* src) {
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s1 == *s2) s1++, s2++;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (*str++) len++;
    return len;
}

int sprintf(char* buffer, const char* format, ...) {
    // Implementação básica (simplificada)
    int i = 0;
    const char* p = format;
    while (*p) {
        if (*p == '%') {
            p++;
            // Suporte básico a %s e %d
            if (*p == 's') {
                char* s = (char*)(&format + 1);
                while (*s) buffer[i++] = *s++;
            } else if (*p == 'd') {
                int num = *((int*)(&format + 1));
                char num_buf[20];
                char* n = num_buf;
                do {
                    *n++ = '0' + num % 10;
                    num /= 10;
                } while (num);
                while (n > num_buf) buffer[i++] = *--n;
            }
            p++;
        } else {
            buffer[i++] = *p++;
        }
    }
    buffer[i] = '\0';
    return i;
}
