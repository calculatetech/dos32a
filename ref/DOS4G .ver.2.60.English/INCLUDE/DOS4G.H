/***************************************************************************
 *
 *  dos4g.h -- Definitions and structures for protected mode
 *
 *  Copyright (c) 1996, Tenberry Software, Inc.
 *  All Rights Reserved
 *
 */

#ifndef DOS4G_INCLUDED
#define DOS4G_INCLUDED

#include "dosgen.h"                             /* Generic definitions */

/**********************************************************************/
/* printf() formats appropriate to print selectors and offsets */

/* mininal selector */
#define PSELS "%04X"
/* maximal selector */
#define PSELSX "%04X"
/* minimal offset */
#define POFFS "%lX"
/* maximal offset */
#define POFFSX "%08lX"

/**********************************************************************/
/* A 32-bit Mode Switch Transfer Stack Frame.   */
/* Must match definition in run32.mac           */

typedef struct tsf32_struct TSF32;

struct tsf32_struct
   {
   USHORT   prev_tsf32;
   SELECTOR ss, ds, es, fs, gs;
   ULONG    edi, esi, ebp, esp;
   ULONG    ebx, edx, ecx, eax;
   SHORT    int_id;
   LONG     int_code;
   ULONG    eip;
   SELECTOR cs;
   USHORT   filler1;
   ULONG    eflags;
   };

typedef struct _gdt32
   {
   USHORT   limit0to15;
   USHORT   base0to15;
   BYTE     base16to23;
   BYTE     access;
   BYTE     limbits;        /* limit16to19 and bits */
   BYTE     base24to31;
   } GDT32;

// format for GDTR and IDTR 
typedef struct _gdtr
	{
	USHORT	limit;
	ULONG 	baseaddr;
	} GDTR;

typedef GDT32 FarPtr GDT32P;

typedef struct _idt32
   {
   USHORT idtoffset;
   USHORT idtsel;
   BYTE   idtun1;
   BYTE   idtaccess;
   USHORT idtoffsethi;
   } IDT32;

typedef struct _intvect32
   {
   OFFSET32 off;
   SELECTOR sel;
   } INTVECT32;

#define idt32getoff(i) (((long)(i.idtoffset))|(((long)i.idtoffsethi)<<16))
#define idt32setoff(i,o) {i.idtoffset = (o&0xFFFF), \
                          i.idtoffsethi = ((o>>16)&0xFFFF);}

#define gdt32getlim16to19(g)  ((g).limbits & 0xF)
#define gdt32getavlbit(g)     (((g).limbits & 0x10) != 0)
#define gdt32getobit(g)       (((g).limbits & 0x20) != 0)
#define gdt32getbbit(g)       (((g).limbits & 0x40) != 0)
#define gdt32getdbit(g)       (((g).limbits & 0x40) != 0)
#define gdt32getgbit(g)       (((g).limbits & 0x80) != 0)

#define gdt32setlim16to19(g,v)  ((g).limbits=(unsigned char) (((g).limbits&~0xF)|((v)&0xF)))
#define gdt32setavlbit(g,v)     ((v) ? (g).limbits|=0x10 : (g).limbits&=~0x10)
#define gdt32setobit(g,v)       ((v) ? (g).limbits|=0x20 : (g).limbits&=~0x20)
#define gdt32setbbit(g,v)       ((v) ? (g).limbits|=0x40 : (g).limbits&=~0x40)
#define gdt32setdbit(g,v)       ((v) ? (g).limbits|=0x40 : (g).limbits&=~0x40)
#define gdt32setgbit(g,v)       ((v) ? (g).limbits|=0x80 : (g).limbits&=~0x80)

/* Extract the LIMIT field of a GDT32 */
#define gdt32limit(g) ((ULONG)(gdt32getlim16to19(g))<<16 | (g).limit0to15)

/* Convert the LIMIT field of a GDT32 to length in bytes */
#define gdt32len(g)\
   (gdt32getgbit(g) ? (ULONG) gdt32limit(g)<<12 | 0xFFF :\
                  (ULONG) gdt32limit(g))

/* Extract the base field of a GDT32 */
#define gdt32base(g)\
  ((ULONG)(((g).base24to31)<<8 | (g).base16to23)<<16 | (g).base0to15)

#define VMBASEADR 0x10000000

#ifndef DOS4G_C
extern char dos4g_verbose;
#endif

#endif
