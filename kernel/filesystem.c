#include "types.h"
#include "filesystem.h"
#include "kernel.h"

neo_bool filesystem_init(void) {
    return neo_false; // Sem sistema de arquivos por enquanto
}

void filesystem_shutdown(void) {
    // Nada para desmontar
}

neo_bool filesystem_test(void) {
    return neo_false; // Sem testes
}
