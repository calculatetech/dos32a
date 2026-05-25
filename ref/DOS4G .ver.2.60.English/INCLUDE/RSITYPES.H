/***************************************************************************
 *
 *  rsitypes.h -- Generic Structures
 *
 *  Copyright (c) 1996, Tenberry Software, Inc.
 *  All Rights Reserved
 *
 */

#ifndef RSITYPES_H
#define RSITYPES_H

/* Types for making the WinDvr code work */
#define HUGE   _huge

#if !defined(_WINNT_) && !defined(MSVC40)
   #ifndef __OS2DEF__   // Watcom os2def.h compat
		#define FAR    _far
		#define NEAR   _near
	#endif
#else
	#undef FAR
	#undef NEAR
	#define FAR
	#define NEAR
#endif

/* Compiler friendly types */
#define FarPtr16    ULONG
#define HugePtr16   ULONG

#if !defined(_WINNT_) && !defined(MSVC40)
#ifdef NOFAR
    #define FarKeyword
    #define FarPtr      *
    #define HugePtr     *
    #define cdecl
    #define Near
    #define far
    #define huge
#else
    #ifdef __HIGHC__
        #define FarKeyword  _Far
        #define FarPtr      * _Far
        #define HugePtr     * _huge
        #define NearPtr     * _near
        #define cdecl
        #define Near
    #endif
    #ifdef MSC32
        #define NOFAR 1
        #define FarKeyword
        #define FarPtr      *
        #define NearPtr     *
        #define Near
    #endif
    #ifdef __GNUC__
        #define NOFAR   1
        #define FarKeyword
        #define FarPtr      *
        #define HugePtr     *
        #define NearPtr     *
        #define Near        *
    #endif
    #if !defined(MSC32) && !defined(__HIGHC__) && !defined(__GNUC__)
        #define FarKeyword  far         /* this includes WATCOM */
        #define FarPtr      far *
        #define HugePtr     huge *
        #define NearPtr     near *
        #define Near        near
    #endif
#endif
#else
   #define NOFAR 1
   #define FarKeyword
   #define FarPtr      *
   #define NearPtr     *
   #define Near
#endif

#if defined(__WATCOMC__)
#if !defined(__386__)
/* define pragma for 'cdecl' keyword to match Microsoft 16bit conventions */
#pragma aux cdecl "_*" parm caller []\
   value struct float struct routine [ax]  modify [ax bx cx dx es];
#endif
#endif

#if !defined(_WINNT_) && !defined(MSVC40)
	#ifdef WATCOM32
		#define API __far16 _pascal _loadds
	#else
		#define API _pascal _loadds
	#endif
#else
	#define API
#endif

/* Base types */
// DWORD first to avoid conflict with thunk patch DWORD defs
typedef unsigned long   DWORD, ULONG;  
/* ThunkType void  */
/* Data thunking prototypes for void */
#ifdef THUNKING
typedef DWORD Syspvoid;
Syspvoid UsrToSyspvoid(void *);
void * SysToUsrpvoid(Syspvoid);
#else
#define Syspvoid void *
#define UsrToSyspvoid(usrp) (usrp)
#define SysToUsrpvoid(sysp) (sysp)
#endif

/* ThunkType char  */
/* Data thunking prototypes for char */
#ifdef THUNKING
typedef DWORD Syspchar;
Syspchar UsrToSyspchar(char *);
char * SysToUsrpchar(Syspchar);
#else
#define Syspchar char *
#define UsrToSyspchar(usrp) (usrp)
#define SysToUsrpchar(sysp) (sysp)
#endif

typedef unsigned char   BYTE, UCHAR;
/* ThunkType BYTE  */
/* Data thunking prototypes for BYTE */
#ifdef THUNKING
typedef DWORD SyspBYTE;
SyspBYTE UsrToSyspBYTE(BYTE *);
BYTE * SysToUsrpBYTE(SyspBYTE);
#else
#define SyspBYTE BYTE *
#define UsrToSyspBYTE(usrp) (usrp)
#define SysToUsrpBYTE(sysp) (sysp)
#endif

#ifndef __OS2DEF__               // watcom os2def.h compat
	typedef char            CHAR;
	typedef short           SHORT;
	/* ThunkType SHORT */
/* Data thunking prototypes for SHORT */
#ifdef THUNKING
typedef DWORD SyspSHORT;
SyspSHORT UsrToSyspSHORT(SHORT *);
SHORT * SysToUsrpSHORT(SyspSHORT);
#else
#define SyspSHORT SHORT *
#define UsrToSyspSHORT(usrp) (usrp)
#define SysToUsrpSHORT(sysp) (sysp)
#endif

	#ifndef HMI_SOUND
     	#ifndef _WINNT_
			#ifndef T32_H							// t32.h compat
				typedef unsigned short  BOOL;
         #endif
			typedef unsigned short  UINT;
		#endif
	#else
		#define BOOL unsigned short
		#define UINT unsigned short
	#endif
	typedef long            LONG;
	/* ThunkType LONG  */
/* Data thunking prototypes for LONG */
#ifdef THUNKING
typedef DWORD SyspLONG;
SyspLONG UsrToSyspLONG(LONG *);
LONG * SysToUsrpLONG(SyspLONG);
#else
#define SyspLONG LONG *
#define UsrToSyspLONG(usrp) (usrp)
#define SysToUsrpLONG(sysp) (sysp)
#endif

#endif
typedef unsigned short  USHORT;
#ifndef HMI_SOUND
	typedef unsigned short  WORD;
#else
	#define WORD unsigned short
#endif

/* Windows(TM) compatibility types */
typedef char NEAR *  PSTR;
#ifndef _WINNT_
	typedef char FAR *   DLGPROC;
#endif
#ifndef HMI_SOUND
	typedef char FAR *   LPSTR;
	#ifndef _WINNT_
		typedef USHORT    HANDLE;
    #endif
#else
	#define LPSTR char FAR *
   #define HANDLE USHORT
#endif
/* ThunkType LPSTR  */
/* Data thunking prototypes for LPSTR */
#ifdef THUNKING
typedef DWORD SyspLPSTR;
SyspLPSTR UsrToSyspLPSTR(LPSTR *);
LPSTR * SysToUsrpLPSTR(SyspLPSTR);
#else
#define SyspLPSTR LPSTR *
#define UsrToSyspLPSTR(usrp) (usrp)
#define SysToUsrpLPSTR(sysp) (sysp)
#endif

#ifndef _WINNT_
typedef char FAR *   LPCSTR;
#endif
/* ThunkType LPCSTR */
/* Data thunking prototypes for LPCSTR */
#ifdef THUNKING
typedef DWORD SyspLPCSTR;
SyspLPCSTR UsrToSyspLPCSTR(LPCSTR *);
LPCSTR * SysToUsrpLPCSTR(SyspLPCSTR);
#else
#define SyspLPCSTR LPCSTR *
#define UsrToSyspLPCSTR(usrp) (usrp)
#define SysToUsrpLPCSTR(sysp) (sysp)
#endif

#ifndef _WINNT_
	typedef UINT         WPARAM, ATOM;
#else
	typedef UINT         WPARAM;
#endif
typedef LONG         LPARAM, LRESULT;
#ifndef __OS2DEF__               // watcom os2def.h compat
	typedef HANDLE       HMODULE;
#endif
typedef HANDLE       HTASK, HGLOBAL, HLOCAL, HINSTANCE;
typedef HANDLE       HCURSOR, HDRVR, HHOOK;
#ifndef _WINNT_
	#ifndef __BSEDOS__               // watcom bsedos.h compat
		typedef HANDLE       HFILE;
	#endif
#endif
typedef DWORD        COLORREF;
typedef DWORD        FOURCC;
typedef void FAR *   LPVOID;
typedef void NEAR *  NPVOID;
#ifndef TSIOS2 // for os2def.h compatibility
	typedef HANDLE       HWND, HBITMAP, HDC;
#else
	#define HWND LPVOID
	#define HBITMAP LPVOID
	#define HDC LPVOID
#endif

#ifndef __HIGHC__
	#ifndef _WINNT_
		typedef void (API * FARPROC)(void);
		typedef void (API * CALLBACK)(WORD, DWORD);
	#endif
	typedef void (API * INTCALLBACK)(WORD, WORD, WORD, WORD, WORD, WORD);
	typedef void (API * DRVCALLBACK)(HDRVR, UINT, DWORD, DWORD, DWORD);
	#if !defined(_WINNT_) && !defined(MSVC40)
		typedef void (FAR _pascal * TIMERPROC)(HWND, UINT, UINT, DWORD);
//   #else
//		typedef void (* TIMERPROC)(HWND, UINT, UINT, DWORD);
	#endif
#endif

/* File handle or far * to file name */
typedef char FarPtr FDORNAME;
#define FDORNAME_DEFINED

/* Addressing types */
typedef USHORT SELECTOR;
typedef USHORT OFFSET16;
typedef ULONG  OFFSET32;
typedef ULONG  LINEAR;
typedef ULONG  PHYSICAL;

#endif


