#ifndef KBOARD_H
#define KBOARD_H

#include <stdint.h>

// Inicialização
void kboard_init();

// Polling - deve ser chamado periodicamente
void kboard_poll();

// Obtém um caractere do buffer (retorna 0 se vazio)
char kboard_get_char();

#endif // KBOARD_H
