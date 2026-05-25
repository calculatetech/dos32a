/****************************************************************************
 *
 *  SIGPM.C -- Protected Mode signaller program
 *
 *    Responsible for
 *
 *    1. Checking if an INT 0x7B TSR installed
 *    2. Passing any arguments to the TSR for printing by the TSR
 *       or
 *       Requesting and displaying the TSR's PSP segment,
 *       Signalling the TSR to be prepared to terminate, 
 *			Patching it's own PSP to terminate to the callback function in 
 *       TSRDEMO, and exiting.
 *
 *  Copyright (c) 1996 Tenberry Software, Inc.
 *  All Rights Reserved
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <dos32lib.h>

#ifndef MSVC40
	#include <i86.h>
#endif

// Constructs a 16:16 ptr
#define makeptr(s,o) ( (void FarPtr) ((long)(s)<<16 | (unsigned)(o)) )

void main (int argc, char *argv[])
{
   USHORT arg_index;
   USHORT pspseg;
   char buffer[128];
	char FarPtr p;
	DWORD intvect;
   _PSP *pp;
   USHORT signaller_iostate;
#ifndef MSVC40
	union REGS r = {0};
	struct SREGS s = {0};
#else
	DWORD edxstore;
#endif
	MEMINFO mi;
      
   // Check whether the 0x7B vector contains a non-zero vector
   intvect = D32RmGetVector(0x7B);

	// Stop if it doesn't	
	if(intvect == 0L)
		{
		printf("\nSIGPM: No INT 7B handler installed\n");
		exit(1);
		}

   printf("\nSIGPM: Starting SIGPM Processing\n");

	// Concatenate the arguments passed to SIGPM if there are any
	if(argc > 1)
		{
		buffer[0] = 0;
		for (arg_index = 1; arg_index < argc; arg_index++)
			{
			strcat (buffer, argv[arg_index]);
			strcat (buffer, " ");
			}
		p = buffer;
		}
	else 
		p = NULL;	// No args therefor signal TSR demo shut down

	if(p)
		{
      // Have the TSR print out arguments passed to SIGPM
		// Pass up the concatenated arguments pointer DS:EDX
      printf("SIGPM: Passing up argument pointer in DS:EDX\n");
#ifndef MSVC40
      s.ds = FP_SEG32(p);
      r.x.ecx = 1L;
      r.x.edx = FP_OFF32(p);
		int386x(0x7B,&r,&r,&s);
#else
      _asm
       	{
         push ds
         push ecx
         push edx
         mov  dx, ss
         mov  ds, dx
         mov  ecx, 1
         mov  edx, p
         int  07Bh
         pop  edx
         pop  ecx
         pop  ds
         }
#endif
      printf("SIGPM: Exiting after parameter passup returned\n");
		}
	else
		{
		// Terminate the TSR
      
		// First signal to get TSR's PSP : Zero ds & edx to prevent argument
      // string print out processing by TSR
      printf("SIGPM: Requesting TSR's PSP segment\n");
#ifndef MSVC40
      s.ds = 0L;
      r.x.edx = 0L;
      r.x.ecx = 1L;
		int386x(0x7B,&r,&r,&s);
		pspseg = r.x.edx & 0xFFFF;
#else
      _asm
       	{
         push ds
         push ecx
         push edx
         mov  dx, 0
         mov  ds, dx
         mov  ecx, 1
         mov  edx, 0
         int  07Bh
         mov  pspseg, dx
         pop  edx
         pop  ecx
         pop  ds
         }
#endif

		// Print out the returned TSR PSP segment
		printf("SIGPM: TSR's PSP at segment %04X\n",pspseg);

		// Signal TSR to shutdown
		printf("SIGPM: Requesting TSR to shutdown\n");
#ifndef MSVC40
      s.ds = 0L;
      r.x.edx = 1L;
      r.x.ecx = 1L;
		int386x(0x7B,&r,&r,&s);
#else
      _asm
       	{
         push ds
         push ecx
         push edx
         mov  dx, 0
         mov  ds, dx
         mov  ecx, 1
         mov  edx, 1
         int  07Bh
         mov  edxstore, edx
         pop  edx
         pop  ecx
         pop  ds
         }
#endif

		D32TsrSavePSP(&signaller_iostate);

		// Make a linear pointer to SIGPM's low memory PSP
		pp = (_PSP *)D32RealToLinear((DWORD)makeptr(signaller_iostate,0));

      // Patch the terminate address of SIGPM's PSP to point at the
      // TSR allocated callback signaller
#ifdef MSVC40
      pp->terminate = edxstore;
#else
      pp->terminate = r.x.edx;
#endif

		// Report on memory status
		printf("SIGPM: Low memory %ld bytes\nSIGPM: Extended Memory %ld bytes\n",D32DosMemAvail(),D32ExtAvail(&mi));

		// Say goodnight Gracie
		printf("SIGPM: Exiting after PSP->terminate patchup set to TSR's callback function\n");

		// Exit back through the PSP->terminate to the TSR callback_x function
      }
}
