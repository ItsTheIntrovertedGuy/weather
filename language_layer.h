#pragma once

#define global_variable static
#define local_persist static
#define internal static

#include <inttypes.h>
#include <stddef.h>
typedef   int8_t    i8;
typedef  int16_t   i16;
typedef  int32_t   i32;
typedef  int64_t   i64;
typedef  uint8_t    u8;
typedef uint16_t   u16;
typedef uint32_t   u32;
typedef uint64_t   u64;
typedef      i32   b32;
typedef    float   f32;
typedef   double   f64;
typedef size_t     memory_index;
typedef uintptr_t  umm;
typedef intptr_t   imm;

#define PFi8  PRIi8
#define PFi16 PRIi16
#define PFi32 PRIi32
#define PFi64 PRIi64
#define PFimm PRIiPTR

#define PFu8  PRIu8
#define PFu16 PRIu16
#define PFu32 PRIu32
#define PFu64 PRIu64
#define PFumm PRIuPTR

#define SFi8  SCNi8
#define SFi16 SCNi16
#define SFi32 SCNi32
#define SFi64 SCNi64
#define SFimm SCNiPTR

#define SFu8  SCNu8
#define SFu16 SCNu16
#define SFu32 SCNu32
#define SFu64 SCNu64
#define SFumm SCNuPTR


#define KIBIBYTES(n) ((n)*(u64)1024)
#define MEBIBYTES(n) ((u64)1024*KIBIBYTES(n))
#define GIBIBYTES(n) ((u64)1024*MEBIBYTES(n))
#define TEBIBYTES(n) ((u64)1024*GIBIBYTES(n))

#define PI (3.14159265358979323846)
#define ARRAYCOUNT(Array) (sizeof(Array) / sizeof((Array)[0]))

#define MIN(a,b) ((a) < (b) ? (a) :  (b))
#define MAX(a,b) ((a) > (b) ? (a) :  (b))
#define ABS(a)   ((a) >  0  ? (a) : -(a))
#define SIGN(a)  ((a) >  0  ?  1  :  ((a) < 0 ? -1 : 0))
#define CLAMP(min,a,max) MIN((max), MAX((min), (a)))
