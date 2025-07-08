#ifndef KBOARD_H
#define KBOARD_H

neo_bool kboard_init(void);
void kboard_poll(void);
char kboard_get_char(void);

#endif
