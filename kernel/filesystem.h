#include "types.h"
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

neo_bool filesystem_init(void);
void filesystem_shutdown(void);
neo_bool filesystem_test(void);

#endif
