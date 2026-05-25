/*
*  An example of exception handling under DOS/4G
*  Copyright (c) 1996 Tenberry Software, Inc.
*  All Rights Reserved
*
*	Note : This example will cause the system to become unstable under an
*			 external DPMI host.
*/

// Standard header files
#include <stdio.h>
#include <sys\types.h>
#include <setjmp.h>

// Extender header files
#include <dos32lib.h>

// Lonngjmp/Setjmp structure
jmp_buf env;

#ifdef MSVC40
extern USHORT flatds;
extern int cdecl except_handler(int handle, TSF32 *ptsf);
#else
// Stack checking must be off for the exception handler because when 
// we get here will be running on a 4G provided stack different than 
// the regular program stack.
#pragma off (check_stack)
int _far _loadds cdecl except_handler(int handle, TSF32 far *ptsf)
	{
   // We track the number of times we have the exception
   static int except_no = 0;
   if (except_no == 0)
   	{
      // The first time through, return control *after* the bad opcode
      ptsf->eip += 2;
      except_no = 1;

      // Tell internal exception handling that all is well
      return (0);
      }

   // The second time, do a long jump back into the main program
   longjmp(env,except_no);
   }
#pragma on (check_stack)
#endif

void bad_opcode()
	{
   // Announce imminent bad opcode
   printf("Here is a bad opcode...");

   // Force all output to appear
   fflush(stdout);

#ifdef MSVC40
	// MSVC specific generation of bad opcode
   _asm _emit 0xF
   _asm _emit 0xFF
#else
   #pragma aux bad_op = 0x0f 0xFF;
   bad_op();
#endif
   
   // Sometimes we come back here
   printf("Control returned after bad opcode.\n");
   fflush(stdout);
   }

main(int argc, char **argv)
{
   // Lock the exception handler and any data it uses down
   D32Lock((void *)&except_handler,4096);
   D32Lock(&env,sizeof(jmp_buf));

#ifdef MSVC40
	// Store flat ds for except_handler processing
   flatds = getds();

   // Hook the exception
   D32SetFaultProc(6,(void *)except_handler,getcs());
#else
   D32SetFaultProc(6,(void far *)except_handler);
#endif

   // Prepare to return here from bad opcode handling
   if (setjmp(env) != 0)
   	{
      // Reset the DOS/4G transfer stack pointer
      // Not recommended under external DPMI host
      D32ResetTSP();
      printf("Control returned from bad opcode via longjmp.\n");
      printf("The next one will go to the default handler.\n");
      }

   // Emit a series of bad opcodes until somebody exits
	while (1)
   	bad_opcode();
}
