/****************************************************************************
 *
 *  SIGRM.C -- Real Mode signaller program
 *
 *    Responsible for
 *
 *    1. Checking if an INT 0x7B TSR installed
 *    2. Passing an argument string to the TSR for printing
 *       or
 *    	Requesting and displaying the TSR's PSP segment,
 *    	Patching up the TSR's PSP, 
 *       Signalling the TSR to shutdown,
 *       Catching the return from the TSR, and exiting.
 *
 *  Copyright (c) 1996 Tenberry Software, Inc.
 *  All Rights Reserved
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

// PSP structure
typedef  struct
   {
   char int20[2];
   unsigned short memtop;
   char rsvd1;
   char callDOS[5];
   short terminate[2];
   short CtrlC[2];
   short Critical[2];
   unsigned short parent_psp;
   unsigned char unit_table[20];
   unsigned short envseg;
   char far *saved_stack;
   unsigned short num_units;
   unsigned char far *unit_table_p;
   char rsvd4[24];
   char int21[2];
   char retf[1];
   char rsvd5[9];
   char fcb1[16];
   char fcb2[20];
   char command_line[128];
   } PSP;

unsigned sv_sp = 0;
extern void retfunc();

#define makeptr(s,o) ( (void far *)((long)(s) << 16 | (unsigned)(o)) )

// Signals the TSR to print out arguments
void print_args(char far *p);
#pragma aux print_args = \
   "push  ds" \
   "mov   ax, es" \
   "mov   ds, ax" \
   "mov   cx, 0" \
   "int   7Bh" \
   "pop   ds" \
   parm [es dx];

// Requests the TSR's PSP address
void get_tsrpsp(unsigned short *pspseg_ptr);
#pragma aux get_tsrpsp = \
   "push  ds" \
   "push  si" \
   "push  ds" \
   "xor   dx, dx" \
   "mov   ds, dx" \
   "mov   cx, 0" \
   "int   7Bh" \
   "pop   ds" \
   "pop   si" \
   "mov   [si], dx" \
   "pop   ds" \
   parm [si];

// Patch up TSR's PSP and signal the TSR to shutdown
void shutdown(PSP far *pp, unsigned retaddr, unsigned *sv_sp);
#pragma aux shutdown = \
   "mov   word ptr es:[di+12], cs" \
   "mov   word ptr es:[di+10], bx" \
   0x60 \
   "mov   [si], sp" \
   "xor   dx, dx" \
   "mov   ds, dx" \
   "inc   dx" \
   "mov   cx, 0" \
   "int   7Bh" \
	parm [es di] [bx] [si];

// Switches to SIGRM's PSP
void retore_psp(unsigned pspseg);
#pragma aux restore_psp = \
   "mov   ax, 0x5000h" \
   "int   21h" \
   parm [bx];

// Reports the return to th real mode signaller after the TSR shutdown and
// exits cleanly
void notify_and_exit()
{
   restore_psp(_psp);

   printf("SIGRM: Returned from TSR shutdown\n");
   printf("SIGRM: Exiting SIGRM Program\n");

   // Must exit by calling exit() explicitly or may crash system
   exit(0);
}

void main (int argc, char *argv[])
{
   unsigned int arg_index;
   unsigned short pspseg;
   char buffer[128];
   char far *p;
   long far *intvect = makeptr(0, 0x7B*4);

   // Check if INT 0x7B handler installed
   if(*intvect == 0L)
      {
      // Fail due to lack of 0x7B handler
      printf("\nSIGRM: No INT 7B handler installed\n");
      exit(1);
      }

   printf("\nSIGRM: Starting SIGRM Processing\n");

   // Concatenate argument strings; if any
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
      p = NULL;

   if(p)
      {
      printf("SIGRM: Passing up concatenated argument(s) pointer to TSR\n");

      // Pass up the pointer to the concatenated args in DS:DX
      print_args(p);
      }
   else
      {
      PSP far *pp;

      printf("SIGRM: Attempting to get TSR's PSP\n");

      // First signal to get TSR's PSP
      get_tsrpsp(&pspseg);
      pp = makeptr(pspseg, 0);
      printf("SIGRM: TSR's PSP at %04X\n",pspseg);

      printf("SIGRM: Attempting to shutdown TSR\n");

      // Signal the TSR to terminate and prepare to catch the TSR's exit call
      shutdown(pp,(unsigned)&retfunc,&sv_sp);
      }
}
