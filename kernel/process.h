#ifndef PROCESS_H
#define PROCESS_H

void process_init(void);
void process_scheduler(void);
void process_yield(void);
void process_list(void);
bool process_test(void);
uint32_t process_get_count(void);

#endif
