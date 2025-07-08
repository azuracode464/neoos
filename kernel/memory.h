#ifndef MEMORY_H
#define MEMORY_H

size_t memory_get_total(void);
size_t memory_get_used(void);
bool memory_init(void);
bool memory_test(void);

#endif
