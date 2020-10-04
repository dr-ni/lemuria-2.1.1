
/*
 * ctrl87.c
 */


#define __CONTROL87_C__


#include "../include/i386/ctrl87.h"
#include "../include/config.h"

#ifdef __GNUC__
#if defined(__i386__) || defined(__ia64__) || defined(__amd64__)
#ifndef NOASSEMBLY

/***** _control87 *****/

#if 0
unsigned short
_control87 (unsigned short newcw, unsigned short mask)
{
  unsigned short cw;

  asm volatile ("                                                    \n\
      wait                                                          \n\
      fstcw  %0                                                       ": /* outputs */ "=m" (cw)
		:		/* inputs */
    );

  if (mask)
    {				/* mask is not 0 */
      asm volatile ("                                                  \n\
        mov    %2, %%ax                                             \n\
        mov    %3, %%bx                                             \n\
        and    %%bx, %%ax                                           \n\
        not    %%bx                                                 \n\
        nop                                                         \n\
        wait                                                        \n\
        mov    %1, %%dx                                             \n\
        and    %%bx, %%dx                                           \n\
        or     %%ax, %%dx                                           \n\
        mov    %%dx, %0                                             \n\
        wait                                                        \n\
        fldcw  %1                                                     ": /* outputs */ "=m" (cw)
		    : /* inputs */ "m" (cw), "m" (newcw), "m" (mask)
		    : /* registers */ "ax", "bx", "dx"
	);
    }
  return cw;

}				/* _control87 */
#else
unsigned int _control87(unsigned int newval, unsigned int mask)
{
  unsigned int fpword = 0;
  unsigned int flags = 0;

//  TRACE("(%08x, %08x): Called\n", newval, mask);

  /* Get fp control word */
  __asm__ __volatile__( "fstcw %0" : "=m" (fpword) : );

//  TRACE("Control word before : %08x\n", fpword);

  /* Convert into mask constants */
  if (fpword & 0x1)  flags |= EM_INVALID;
  if (fpword & 0x2)  flags |= EM_DENORMAL;
  if (fpword & 0x4)  flags |= EM_ZERODIVIDE;
  if (fpword & 0x8)  flags |= EM_OVERFLOW;
  if (fpword & 0x10) flags |= EM_UNDERFLOW;
  if (fpword & 0x20) flags |= EM_INEXACT;
  switch(fpword & 0xC00) {
  case 0xC00: flags |= RC_UP|RC_DOWN; break;
  case 0x800: flags |= RC_UP; break;
  case 0x400: flags |= RC_DOWN; break;
  }
  switch(fpword & 0x300) {
  case 0x0:   flags |= PC_24; break;
  case 0x200: flags |= PC_53; break;
  case 0x300: flags |= PC_64; break;
  }
  if (fpword & 0x1000) flags |= IC_AFFINE;

  /* Mask with parameters */
  flags = (flags & ~mask) | (newval & mask);
 
  /* Convert (masked) value back to fp word */
  fpword = 0;
  if (flags & EM_INVALID)    fpword |= 0x1;
  if (flags & EM_DENORMAL)   fpword |= 0x2;
  if (flags & EM_ZERODIVIDE) fpword |= 0x4;
  if (flags & EM_OVERFLOW)   fpword |= 0x8;
  if (flags & EM_UNDERFLOW)  fpword |= 0x10;
  if (flags & EM_INEXACT)    fpword |= 0x20;
  switch(flags & (RC_UP | RC_DOWN)) {
  case RC_UP|RC_DOWN: fpword |= 0xC00; break;
  case RC_UP:         fpword |= 0x800; break;
  case RC_DOWN:       fpword |= 0x400; break;
  }
  switch (flags & (PC_24 | PC_53)) {
  case PC_64: fpword |= 0x300; break;
  case PC_53: fpword |= 0x200; break;
  case PC_24: fpword |= 0x0; break;
  }
  if (flags & IC_AFFINE) fpword |= 0x1000;

//  TRACE("Control word after  : %08x\n", fpword);

  /* Put fp control word */
  __asm__ __volatile__( "fldcw %0" : : "m" (fpword) );

  return flags;
}
#endif // 0

#endif
#endif
#endif

#ifdef __GNUC__
#ifdef __i386__
#if 0

/*
 * copy.c -- fast memcpy routines for Pentium using FPU
 * Copyright (c) 1995, 1996 Robert Krawitz <rlk@tiac.net>
 * and Gerhard Koerting (G.Koerting@techem.ruhr-uni-bochum.de)
 * Exception handling in kernel/userspace routines by Gerhard
 * Koerting.
 * May be used and redistributed under the terms of the GNU Public License
 */
#include <sys/types.h>

#define CACHELINE 32
#define CACHEMASK (CACHELINE - 1)
#define BIGMASK (~255)
#define SMALLMASK (~31)

void *
penium___zero_chunk (void *_to, size_t _bytes)
{
  unsigned long temp0, temp1;
  register unsigned long to asm ("di") = (unsigned long) _to;
  register unsigned long bytes asm ("dx") = (unsigned long) _bytes;
  char save[42];
  unsigned long zbuf[2] = { 0, 0 };
  temp0 = to & 7;
  if (temp0)
    {
      bytes -= temp0;
      asm __volatile__ ("cld\n\t"
			"rep; stosb\n\t":"=D" (to):"D" (to), "a" (0),
			"c" (temp0):"cx");
    }
  asm __volatile__ ("shrl $3, %0\n\t"
		    "fstenv %4\n\t"
		    "fstpt 32+%4\n\t"
		    "movl (%1), %2\n\t"
		    "fildq %3\n"
		    "2:\n\t"
		    "fstl (%1)\n\t"
		    "addl $8, %1\n\t"
		    "decl %0\n\t"
		    "jne 2b\n\t"
		    "fstpl %%st\n\t"
		    "fldt 32+%4\n\t"
		    "fldenv %4\n\t":"=&r" (temp0), "=&r" (to),
		    "=&r" (temp1):"m" (*(char *) zbuf), "m" (*(char *) save),
		    "0" (bytes), "1" (to));
  bytes &= 7;
  if (bytes)
    {
      asm __volatile__ ("shrl $2, %%ecx\n\t"
			"cld\n\t"
			"rep ; stosl\n\t"
			"testb $2,%%dl\n\t"
			"je 111f\n\t"
			"stosw\n"
			"111:\ttestb $1,%%dl\n\t"
			"je 112f\n\t"
			"stosb\n"
			"112:":"=D" (to):"D" (to), "a" (0), "c" (bytes),
			"d" (bytes):"cx", "memory");
    }
  return _to;
}

void *
pentium__memcpy_g (void *_to, const void *_from, size_t _bytes)
{
  register unsigned long from asm ("si") = (unsigned long) _from;
  register unsigned long to asm ("di") = (unsigned long) _to;
  register unsigned long bytes asm ("dx") = (unsigned long) _bytes;
  if (bytes >= 1024)
    {
      unsigned long temp0, temp1;
      char save[108];

      temp0 = to & 7;
      if (temp0)
	{
	  bytes -= temp0;
	  asm __volatile__ ("cld\n\t"
			    "rep; movsb\n\t":"=D" (to), "=S" (from):"D" (to),
			    "S" (from), "c" (temp0):"cx");
	}
      asm __volatile__ ("shrl $8, %0\n\t"
			"movl (%2), %3\n\t" "movl (%1), %3\n\t"
			/*"fsave %4\n" */
			"1:\n\t"
			"movl $4, %3\n"
			"2:\n\t"
			"fildq 0x0(%2)\n\t"
			"fildq 0x20(%2)\n\t"
			"fildq 0x40(%2)\n\t"
			"fildq 0x60(%2)\n\t"
			"fildq 0x80(%2)\n\t"
			"fildq 0xa0(%2)\n\t"
			"fildq 0xc0(%2)\n\t"
			"fildq 0xe0(%2)\n\t"
			"fxch\n\t"
			"fistpq 0xc0(%1)\n\t"
			"fistpq 0xe0(%1)\n\t"
			"fistpq 0xa0(%1)\n\t"
			"fistpq 0x80(%1)\n\t"
			"fistpq 0x60(%1)\n\t"
			"fistpq 0x40(%1)\n\t"
			"fistpq 0x20(%1)\n\t"
			"fistpq 0x0(%1)\n\t"
			"addl $8, %2\n\t"
			"addl $8, %1\n\t"
			"decl %3\n\t"
			"jne 2b\n\t"
			"addl $224, %2\n\t"
			"addl $224, %1\n\t" "decl %0\n\t" "jne 1b\n\t"
			/*"frstor %4\n\t" */
			:"=&r" (temp0), "=&r" (to), "=&r" (from),
			"=&r" (temp1):"m" (save[0]), "0" (bytes), "1" (to),
			"2" (from):"memory");
      bytes &= 255;
    }
  if (bytes)
    {
      asm __volatile__ ("shrl $2, %%ecx\n\t"
			"cld\n\t"
			"rep ; movsl\n\t"
			"testb $2,%%dl\n\t"
			"je 111f\n\t"
			"movsw\n"
			"111:\ttestb $1,%%dl\n\t"
			"je 112f\n\t"
			"movsb\n"
			"112:":"=D" (to), "=S" (from):"D" (to), "S" (from),
			"c" (bytes), "d" (bytes):"cx", "memory");
    }
  return _to;
}
#endif
#endif
#endif
