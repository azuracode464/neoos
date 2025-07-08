#ifndef TYPES_H
#define TYPES_H

// Evita redefinir neo_bool se já existir (pra clang freestanding)
#ifndef __cplusplus
  #ifndef __bool_true_false_are_defined
    typedef unsigned char neo_bool;
    #define neo_true 1
    #define neo_false 0
    #define __bool_true_false_are_defined 1
  #endif
#endif

// Tipos inteiros básicos com tamanho fixo
typedef signed char neo_i8;
typedef unsigned char neo_u8;

typedef signed short neo_i16;
typedef unsigned short neo_u16;

typedef signed int neo_i32;
typedef unsigned int neo_u32;

typedef signed long long neo_i64;
typedef unsigned long long neo_u64;

// Tipos de tamanho natural para a arquitetura 64 bits
typedef unsigned long neo_usize;
typedef long neo_ssize;

// Padrão para NULL
#ifndef NULL
#define NULL ((void*)0)
#endif

// Definição para neo_iptr e neo_uptr
typedef long neo_iptr;
typedef unsigned long neo_uptr;

// Definição para wchar_t (se precisar, pode adaptar)
typedef unsigned short wchar_t;

// Constantes de limites (exemplo básico)
#define INT8_MAX 127
#define INT8_MIN (-128)
#define UINT8_MAX 255

#define INT16_MAX 32767
#define INT16_MIN (-32768)
#define UINT16_MAX 65535

#define INT32_MAX 2147483647
#define INT32_MIN (-2147483648)
#define UINT32_MAX 4294967295U

#define INT64_MAX 9223372036854775807LL
#define INT64_MIN (-9223372036854775807LL - 1)
#define UINT64_MAX 18446744073709551615ULL

#endif // TYPES_H
