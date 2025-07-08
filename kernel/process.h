#include "types.h"
#ifndef PROCESS_H
#define PROCESS_H

neo_bool process_init(void);
void process_scheduler(void);
void process_yield(void);
void process_list(void);
neo_bool process_test(void);
neo_u32 process_get_count(void);

#endif
