/* a10 425
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/fpu.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 425 */



#ifndef KNOS_DEMOS_LIB_FPU_H
#define KNOS_DEMOS_LIB_FPU_H

#include <libc/stdint.h>

#if !defined(__i386__) && !defined(__gcc__)

#define fpu_setround() 1
#define fpu_restore(a) ({ a; })
#define double_to_int(f) ((int)(f))
#define float_to_int(f) ((int)(f))

#else 

/*
  very crude x86 float to int conversions
  assumes the FPU is in round mode! 

  comes from libvorbis code.
*/

static inline 
int16_t fpu_setround (){
    int16_t ret;
    int16_t temp;
    __asm__ __volatile__("fnstcw %0\n\t"
			 "movw %0,%%dx\n\t"
			 "orw $62463,%%dx\n\t"
			 "movw %%dx,%1\n\t"
			 "fldcw %1\n\t":"=m"(ret):"m"(temp): "dx");
    return ret;
}

static inline 
void fpu_restore(int16_t fpu){
  __asm__ __volatile__("fldcw %0": : "m"(fpu));
}

static inline 
int32_t double_to_int(double f) {
/* yes, double!  Otherwise,
   we get extra fst/fld to
   truncate precision */
  int32_t i;
  __asm__("fistl %0": "=m"(i) : "t"(f));
  return(i);
}

static inline 
int32_t float_to_int(float val)
{
    return double_to_int ((double) val);
}

#endif


#if 0
#include <libc/endian.h>

/*
References: posted by Andy Mucho (and David Waugh and Oskari Tammelin (?)), original inventor (?) 'Sree Kotay'

Notes:
-Platform independant, literally. You have IEEE FP numbers, this will work, as long as your not expecting a signed integer back larger than 16bits :)
-Will only work correctly for FP numbers within the range of [-32768.0,32767.0]
-The FPU must be in Double-Precision mode
*/

static const double _double2fixmagic = 68719476736.0*1.5;
static const int32_t _shiftamt        = 16;

#if BYTE_ORDER == BIG_ENDIAN
        #define iexp_                           0
        #define iman_                           1
#else
        #define iexp_                           1
        #define iman_                           0
#endif

// double_to_int
static inline 
int32_t double_to_int(double val)
{
    val = val + _double2fixmagic;
    return ((int32_t*)&val)[iman_] >> _shiftamt;
}

// float_to_int
static inline 
int32_t float_to_int(float val)
{
    return double_to_int ((double) val);
}

#endif

#endif
