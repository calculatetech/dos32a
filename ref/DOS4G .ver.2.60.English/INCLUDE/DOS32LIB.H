/***************************************************************************
 *
 *  dos32lib.h -- Prototypes and definitions for the DOS4G Function Library
 *
 *  Copyright (c) Tenberry Software, Incorporated 1996
 *  All Rights Reserved
 *
 */

#ifndef DOS32LIB_H
#define DOS32LIB_H

#include <rsitypes.h>

#define MAX_MODULES             100

#ifndef MAKELONG
#define MAKELONG(s, o)  ( (((long) (o)) & 0xFFFFL) | (((long) (s)) << 16) )
#endif

#ifndef MAKEULONG
#define MAKEULONG(s, o)  ( (((unsigned long) (o)) & 0xFFFFL) | (((unsigned long) (s)) << 16) )
#endif

#define TSF_CFLAG(tsf32) ((tsf32).eflags & 1)

typedef char FarPtr D32LPSTR;

#define FP_SEG32(p)   (*((USHORT FarKeyword *) &(p) + 2))
#define FP_OFF32(p)   (*((ULONG FarKeyword *) &(p)))

#ifndef DOS4G_INCLUDED
/* Must match definition in run32.mac */

typedef struct tsf32_struct TSF32;

struct tsf32_struct
    {
        WORD            prev_tsf32;
        WORD            ss, ds, es, fs, gs;
        DWORD           edi, esi, ebp, esp;
        DWORD           ebx, edx, ecx, eax;
        WORD            int_id;
        DWORD           int_code;
        DWORD           eip;
        WORD            cs;
        WORD            filler1;
        DWORD           eflags;
    };

typedef struct _gdt32
    {
        WORD         limit0to15;
        WORD         base0to15;
        BYTE         base16to23;
        BYTE         access;
        BYTE         limbits;        /* limit16to19 and bits */
        BYTE         base24to31;
    } GDT32;
#endif

typedef struct _d32regs
{
    DWORD ds, es;
    DWORD edi, esi, ebp, esp;
    DWORD ebx, edx, ecx, eax;
    DWORD eflags;
} D32REGS;

typedef struct
{
    DWORD off;
    WORD sel;
} FarPtr32;

typedef struct loader_vector
{
    FarPtr16 loader_package;      /* (PACKAGE *) */
    FarPtr16 loader_actions[10];  /* (ACTION *)  */
} LOADER_VECTOR;

// Valid parameters for D32TsrExit
#define TSR_INSTALL   0
#define TSR_UNINSTALL 1

typedef struct _psp
   {
   char int20[2];
   USHORT memtop;
   char rsvd1;
   char callDOS[5];
   ULONG terminate;
   ULONG CtrlC;
   ULONG Critical;
   USHORT parent_psp;
   UCHAR unit_table[20];
   USHORT envseg;
   ULONG saved_stack;
   USHORT num_units;
   ULONG unit_table_p;
   char rsvd4[24];
   char int21[2];
   char retf[1];
   char rsvd5[9];
   char fcb1[16];
   char fcb2[20];
   char command_line[128];
   } _PSP;

// Valid memory allocation strategies for D32MemStrategy
#define MPreferExt 0            /* prefer, but not force, extended */
#define MPreferLow 1            /* prefer, but not force, conventional memory */
#define MForceExt 2             /* force extended */
#define MForceLow 3             /* force conventional */
#define MTransparent 4          /* force conventional, allocate selectors for */
                                /*   transparent addressing */
#define MTransStack 5           /* transparent, but aligned for use as stack */

/*=== DPMI LDT Management Services ====*/

#define  DPMIDescAlloc           0x0000
#define  DPMIDescFree            0x0001
#define  DPMISegtoDesc           0x0002
#define  DPMIGetNextSel          0x0003
#define  DPMIReserved1           0x0004
#define  DPMIReserved2           0x0005
#define  DPMIGetSegAddr          0x0006
#define  DPMISetSegAddr          0x0007
#define  DPMISetSegLimit         0x0008
#define  DPMISetSegAccess        0x0009
#define  DPMICSAlias             0x000A
#define  DPMIGetDesc             0x000B
#define  DPMISetDesc             0x000C
#define  DPMINamedDescAlloc      0x000D

/*=== DPMI DOS Memory Management Services ====*/

#define  DPMIDosMemAlloc         0x0100
#define  DPMIDosMemFree          0x0101

/*=== DPMI Interrupt & Exception Services ====*/

#define  DPMIGetRealModeVect     0x0200
#define  DPMISetRealModeVect     0x0201
#define  DPMIGetProtModeExcVect  0x0202
#define  DPMISetProtModeExcVect  0x0203
#define  DPMIGetProtModeVect     0x0204
#define  DPMISetProtModeVect     0x0205
#define  DPMIDisableVirtInt      0x0900
#define  DPMIEnableVirtInt       0x0901
#define  DPMIGetVirtIntState     0x0902

/*=== DPMI Translation Services ====*/

#define  DPMIRmInterrupt         0x0300
#define  DPMIRmProcRetf          0x0301
#define  DPMIRmProcIret          0x0302
#define  DPMIRmAllocCallback     0x0303
#define  DPMIRmFreeCallback      0x0304
#define  DPMIGetStateSaveAddr    0x0305
#define  DPMIGetRawModeSwAddr    0x0306

/*=== DPMI Memory Management Services ====*/

#define  DPMIMemInfo             0x0500
#define  DPMIMemAlloc            0x0501
#define  DPMIMemFree             0x0502
#define  DPMIMemResize           0x0503
#define  DPMIAllocLinear         0x0504
#define  DPMIGetPgAttrib         0x0506
#define  DPMISetPgAttrib			0x0507
#define  DPMIGetBlockInfo			0x050A


/*=== DPMI Page Management Services ====*/

#define  DPMILockLinearRegion    0x0600
#define  DPMIUnlockLinearRegion  0x0601
#define  DPMIPageSize            0x0604
#define  DPMIMarkDemandPage	   0x0702
#define  DPMIDiscardPageContents 0x0703

/*=== DPMI Miscellaneous Services ====*/

#define DPMIGetVersion           0x0400
#define DPMIMapDevBlock				0x0508
#define DPMIMapConvBlock			0x0509
#define DPMIPhysAddrMapping      0x0800
#define DPMIFreePhysMapping      0x0801
#define DPMIGetVendorAPI         0x0A00
#define DPMIGetCoprocStatus      0x0E00
#define DPMISetCoprocEmulation   0x0E01

#define DPMIExt_GET_PKG16_ENTRY     0
#define DPMIExt_CALL_PKG16_ENTRY    1
#define DPMIExt_CALL_FAR16_ENTRY    2
#define DPMIExt_GET_TEST_STUB       3
#define DPMIExt_INT_STRAT_API       4
#define DPMIExt_INT_STRAT_RESET_API 5

#define INTAPI_SET_PASSUP           0
#define INTAPI_SET_RAWPASSDN        1
#define INTAPI_EXCEPTION_HOOKABLE   5

#if defined(__HIGHC__)
_packed
#else
#pragma pack(1)
#endif
struct _dpmi_callregs
{
    DWORD edi, esi, ebp, esp;
    DWORD ebx, edx, ecx, eax;
    WORD flags;
    WORD es, ds;
    WORD fs, gs;
    WORD ip, cs;
    WORD sp, ss;
};
#if defined(__HIGHC__)
_packed
#endif
struct _dosmcb
{
    BYTE signature;
    WORD owner;
    WORD size;          /* Number of paragraphs excluding MCB itself */
    BYTE unused[3];
    BYTE name[8];
};               /* DOS arena header */
typedef struct _dosmcb DOSMCB;

#if !defined(__HIGHC__)
#pragma pack()
#endif

typedef struct _dpmi_callregs DPMI_CALLREGS;

typedef struct _meminfo
{
    DWORD   max_free_block_bytes;
    DWORD   max_unlocked_alloc_pages;
    DWORD   max_locked_alloc_pages;
    DWORD   linear_space_pages;
    DWORD   total_unlocked_pages;
    DWORD   total_free_pages;
    DWORD   total_phys_pages;
    DWORD   free_space_pages;
    DWORD   swap_area_pages;
    DWORD   reserved[3];
} MEMINFO;

typedef struct _share_mem_block
{
    ULONG   request_length;
    ULONG   actual_length;
    ULONG   shared_handle;
    ULONG   linear_address;
    ULONG   name_off32;
    ULONG   name_sel;
    ULONG   reserved2;
} SHARE_MEM_BLOCK;      /* For DPMI INT31 function 0xD00 */

typedef struct _xmc
{
    DWORD next_blk;
    DWORD blk_siz;
} EXT_MEM_CHAIN;

#if defined(__GNUC__) || defined(__HIGHC__)
#define cdecl
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern cdecl CALL16 (int, int, ...);    /* These are assembled, not compiled */
extern cdecl CALL16REGS32 ();
extern cdecl D32Run_help ();
extern cdecl getcs ();
extern cdecl getds ();
extern cdecl getes ();
extern cdecl getfs ();
extern cdecl getgs ();
extern cdecl getlim ();
extern cdecl getss ();
extern cdecl breakpoint ();
extern cdecl D32MemBcopy48 (void *, DWORD, void *, DWORD, int);
extern cdecl INT21 (int, int, D32REGS *);
extern cdecl INT31 (int, int, D32REGS *);
#ifdef __cplusplus
}
#endif

#if defined(__HIGHC__)
#pragma Alias (CALL16, "_CALL16");
#pragma Alias (CALL16REGS32, "_CALL16REGS32");
#pragma Alias (D32Run_help, "_D32Run_help");
#pragma Alias (D32MemBcopy48, "_D32MemBcopy48");
#pragma Alias (getcs, "_getcs");
#pragma Alias (getds, "_getds");
#pragma Alias (getes, "_getes");
#pragma Alias (getfs, "_getfs");
#pragma Alias (getgs, "_getgs");
#pragma Alias (getlim, "_getlim");
#pragma Alias (getss, "_getss");
#pragma Alias (INT21, "_INT21");
#pragma Alias (INT31, "_INT31");
#endif

#ifdef __WATCOMC__
    #define RSIAPI32 cdecl
#else
    #ifdef __GNUC__
        #define RSIAPI32 _System
    #else
        #define RSIAPI32
    #endif
#endif

#ifdef NOFAR
    #define PINTHANDLER48 int (cdecl *off) (int hnext, TSF32 *tsf32), int seg
#else
    #if defined (__HIGHC__)
        #define PINTHANDLER48 _Far int (*handler) (int hnext, TSF32 _Far *tsf32)
    #else
        #define PINTHANDLER48 int (cdecl FarKeyword *handler) (int hnext, TSF32 FarKeyword *tsf32)
    #endif
#endif

// low memory address splitter macros
// real segment from linear address
#define  D32RealSeg(P)  ((((DWORD) (P)) >> 4) & 0xFFFF)
// real offset from linear address
#define  D32RealOff(P)  (((DWORD) (P)) & 0xF)

// Used for D32CreateSegAlias
#define USE16 0
#define USE32 1
#define DATA_SEG 0
#define CODE_SEG 1
#define LINEAR 0
#define LOGICAL 1

// Backwards compatibility #defines
#define D321616to32(p)			 D321616toLinear32(p)
#define D32GetSegAddr(sel)		 D32GetSegBase(sel)
#define D32SegAlias(la) 		 D32CreateSegAlias(la,DATA_SEG,USE16,0)
#define D32SegAliasLogical(la) D32CreateSegAlias(la,DATA_SEG,USE16,1)

#ifdef __cplusplus
extern "C" {
#endif
extern char * RSIAPI32       D321616toLinear32 (int p1616);
extern char * RSIAPI32       D321616toLogical32 (int p1616);
extern void * RSIAPI32       D32AllocSharedMem (int size, char *name, DWORD *phnd);
extern int RSIAPI32          D32AnyLoad (char *filename, LOADER_VECTOR *lv);
extern int RSIAPI32          D32CallDebug (char *);
extern int RSIAPI32          D32CallEntry16 (FarPtr16 addr16, long argcount, ...);
extern int RSIAPI32          D32CancelFaultProc (int handle);
extern int RSIAPI32          D32CancelIntProc (int handle);
extern void RSIAPI32         D32ChainInterrupt (int hq, int off, int seg);
extern char * RSIAPI32       D32Copyright (void);
extern unsigned int RSIAPI32 D32CreateSegAlias (unsigned int la, unsigned int cs_or_ds, unsigned int usetype, unsigned int lin_or_log);
extern WORD RSIAPI32         D32DescAlloc (int count);
extern int RSIAPI32          D32DescFree (WORD sel);
extern WORD RSIAPI32         D32DosMemAlloc (DWORD size, WORD *psel);
extern int RSIAPI32          D32DosMemAvail (void);
extern int RSIAPI32          D32DosMemFree (WORD sel);
extern DWORD RSIAPI32        D32DPMIRmGetVector (int intno);
extern int RSIAPI32          D32DPMIRmSetVector (int intno, DWORD rmvect);
#ifdef MSVC40
extern int RSIAPI32    		  D32DPMIPmGetVector (int intno, FarPtr32 *pmvect);
extern int RSIAPI32          D32DPMIPmSetVector (int intno, FarPtr32 *pmvect);
#else
extern void FarKeyword *RSIAPI32  D32DPMIPmGetVector (int intno);
extern int RSIAPI32          D32DPMIPmSetVector (int intno, void FarKeyword *pmvect);
#endif
extern DWORD RSIAPI32        D32ExtAvail (MEMINFO *mp);
extern int RSIAPI32          D32FindTsrVector (char *id, int off, int len, int base_vect);
extern int RSIAPI32          D32FreeSharedMem (DWORD handle);
extern int RSIAPI32          D32GetDesc (int sel, GDT32 *dp);
extern int RSIAPI32          D32GetDPMIExtensions (int *seg, int *off);
extern int RSIAPI32          D32GetEntry16 (char *pkgname, char *pkgentry);
extern int RSIAPI32          D32GetNextSel (void);
extern DWORD RSIAPI32        D32GetSegBase (WORD sel);
extern int RSIAPI32          D32GetSelfLoader (LOADER_VECTOR *plv);
extern int RSIAPI32          D32isDOS4G (void);
extern DWORD RSIAPI32        D32LinearToReal (DWORD linaddr);
extern WORD RSIAPI32         D32LoadModule (char *filename, char *command_tail, TSF32 *initial_registers);
extern WORD RSIAPI32         D32Load (char *filename, char *command_tail, TSF32 *initial_registers);
extern int RSIAPI32          D32Lock (void *addr, int size);
extern DWORD RSIAPI32        D32MemAlloc (DWORD size, DWORD *phandle);
extern int RSIAPI32          D32MemFree (DWORD handle);
extern int RSIAPI32          D32MemInfo (MEMINFO *mp);
extern DWORD RSIAPI32        D32MemResize (DWORD *phandle, DWORD newsize);
extern int RSIAPI32          D32MemStrategy (int strat);
extern char * RSIAPI32       D32Name (void);
extern int RSIAPI32          D32NamedDescAlloc (WORD sel);
extern int RSIAPI32          D32PageSize (void);
extern int RSIAPI32          D32Passup (int intno);
extern int RSIAPI32          D32ClearPassup (int intno, DWORD old_rm_vect);
extern void RSIAPI32         D32PreparePSP (_PSP *model_psp);
extern int RSIAPI32          D32QueryModuleFileName (WORD mod_handle, char *filename, int length);
extern WORD RSIAPI32         D32QueryModuleHandle (char *modulename);
extern int RSIAPI32          D32QueryModuleName (WORD mod_handle, char *filename, int length);
extern int RSIAPI32          D32QueryModuleUsage (WORD dll_handle);
extern void * RSIAPI32       D32QueryProcAddr (WORD hmod, DWORD ordinal, char *funcname, DWORD *linaddr);
extern DWORD RSIAPI32        D32RealToLinear (DWORD rp1616);
extern int RSIAPI32          D32RelocateAddress (FarPtr32 *fp, WORD handle);
extern void RSIAPI32         D32ResetTSP (void);
extern DWORD RSIAPI32        D32RmGetVector (int intno);
extern int RSIAPI32          D32RmInterrupt (int intno, DPMI_CALLREGS *cr);
extern void RSIAPI32         D32RmSetVector (int intno, DWORD vector);
extern void RSIAPI32         D32Run (TSF32 *tsf32);
extern int RSIAPI32          D32RunReturn (TSF32 *tsf32);
extern WORD RSIAPI32         D32SegMoveLow (DWORD linaddr, DWORD segsize, WORD *psel);
extern void * RSIAPI32       D32SegRealloc (ULONG addr16);
extern int RSIAPI32          D32SegtoDesc (void);
extern int RSIAPI32          D32SetDesc (int sel, GDT32 *dp);
extern int RSIAPI32          D32SetFaultProc (int num, PINTHANDLER48);
extern int RSIAPI32          D32SetIntProc (int num, PINTHANDLER48);
extern char * RSIAPI32       D32Timestamp (void);
extern int RSIAPI32          D32TsrExit (int mode, int code);
extern void RSIAPI32         D32TsrSavePSP (WORD *psp);
extern void RSIAPI32         D32TsrSetPSP (WORD psp);
extern void RSIAPI32         D32TsrSwitchPSP (WORD newpsp, WORD *savepsp);
extern int RSIAPI32          D32UnloadModule (WORD handle);
extern int RSIAPI32          D32Unload (WORD handle);
extern int RSIAPI32          D32Unlock (void *addr, int size);
extern int RSIAPI32          D32UnrelocateAddress (FarPtr32 *fp, WORD handle);
extern char * RSIAPI32       D32Version (void);
#ifdef __cplusplus
}
#endif

extern int Rsiapi_seg, Rsiapi_off;

#endif // DOS32LIB_H
