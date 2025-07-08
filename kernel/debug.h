#ifndef DEBUG_H
#define DEBUG_H

// Funções de depuração
void log_info(const char* message);
void log_warning(const char* message);
void log_error(const char* message);
void panic(const char* message);

#endif
