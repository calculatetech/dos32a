/****************************************************************************
 *
 *  TSRDEMO.C -- Responsible for
 *    1. Installing itself as a TSR
 *    2. Returning the TSR's PSP segment from 0x7B call if requested
 *    3. Printing out command line arguments from SIGRM or SIGPM program,
 *       if any
 *    and
 *    4. Generating a callback and the necessary real memory far call to
 *       call the callback if signalled from protected mode, return the
 *       address of the low memory far call, and do the necessary clean 
 *       up when the callback is issued from the termination of the 
 *       protected mode signaller.
 *
 *  Copyright (c) 1996 Tenberry Software, Inc.
 *  All Rights Reserved
 *
 */

#include <stdio.h>
#include <dos.h>
#include <dos32lib.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

// Constructs a 16:16 ptr
#define makeptr(s,o) ( (void FarPtr) ((long)(s)<<16 | (unsigned)(o)) )

// Holds the PSP IO information for the tsr and the client
USHORT client_iostate, tsr_iostate;

// Execution context for client interrupt
TSF32 client_context;

// Execution context for TSR
TSF32 tsr_context;

// Contents of registers at interrupt time: note - segments have been zeroed
TSF32 FarPtr client_regs;

#ifdef MSVC40
	// Client_regs 16:16 far ptr passed to outer_handler function
	FarPtr32 crfptr;  

	// Store for DS addressability in TSRDEMOA.ASM
	extern USHORT flatds;
#endif

// Stores newer real-mode interrupt for passup address
int passup_rminterrupt;

// Stores old real-mode interrupt for restoring at the proper moment
int old_rminterrupt;

// Stores newest real-mode interrupt to check for interloper
int new_rminterrupt;

// Store low memory 6byte block used to do far call to callback address
USHORT sel;
char *lowp;

// Store ESP for clean-up before exit when callback is called
ULONG sv_esp = 0L;

#ifdef MSVC40
	// For D32 API INT31 calls
	D32REGS r = {0};
#else
	// For int386 and int386x calls
	union REGS r = {0};
	struct SREGS s = {0};
#endif

// Moves the rm handler into low memory and returns the rm handler address
USHORT sel;    // low memory selector storage for FreeHandler
int MigrateHandler(char *handler, USHORT size)
{
   int addr1616 = D32SegMoveLow(FP_OFF32(handler), size, &sel) << 16;

   if(!addr1616)
      {
      printf("\tTSR: Couldn't allocate low memory for handler\n");
      exit(1);
      }

   // Return a 16:16 real mode pointer
   return(addr1616);
}

// Frees the low memory used for the real mode handler
FreeHandler()
{
   D32DosMemFree(sel);
}

// Switch execution context -- like a MODULA/2 TRANSFER
#ifdef MSVC40
	void cdecl tsf32_exec(TSF32 *from, USHORT fromds, TSF32 *to, USHORT tods);
	extern int cdecl outer_handler(int hNext, TSF32 *regs);
#else
	void cdecl tsf32_exec(TSF32 FarPtr from, TSF32 FarPtr to);

	int _far _loadds cdecl outer_handler(int hNext, TSF32 FarPtr regs)
	{
   	void inner_handler();

	   client_regs = regs;

   	// Switch stack to tsr stack and execute normal C code
	   // Assumes inner handler in the same code segment as main()
   	tsr_context.eip = (unsigned) &inner_handler;
	   tsf32_exec(&client_context, &tsr_context);

   	// Interrupt handled */
	   return(0);
	}
#endif

// Note: we need to terminate on the TSR's PSP
void terminate_tsr()
{
   if(new_rminterrupt != D32RmGetVector(0x7B))
      {
      printf("\tTSR: interrupt overloaded, not safe to uninstall\n");
#ifndef MSVC40
      flushall();
#endif
      return;
      }

   // Detach new handler
   D32RmSetVector(0x7B, old_rminterrupt);
   FreeHandler();

   printf("\tTSR: Terminating the TSR\n");

#ifndef MSVC40
	flushall();
#endif
   
   D32TsrExit(TSR_UNINSTALL, 0);
}

#ifndef MSVC40
	// DS -> SS and parameter 1 into ESP
	void restore_ss_esp(ULONG sv_esp);
	#pragma aux restore_ss_esp = \
		"mov ax, ds" \
		"mov ss, ax" \
   	"mov esp, ebx" \
	   parm [ax];

   // Protected mode function which is target of callback
   void _interrupt _cdecl callback_x 
      ( 
      // regs pushed in this order by prologue
      int rgs, int rfs, int res, int rds, int rdi, int rsi, 
      int rbp, int rsp, int rbx, int rdx, int rcx, int rax
      )
   {
      // restore a valid section of 32bit flat stack for final processing
      restore_ss_esp(sv_esp);
   
      // restore the TSR's PSP for clean exit
      D32TsrSwitchPSP(tsr_iostate, &client_iostate);
      
      // free callback
      r.w.ax = 0x0304;                    // DPMI free real mode callback
      r.w.dx = *((unsigned short *) &lowp[1]);
      r.w.cx = *((unsigned short *) &lowp[3]);
      int386 (0x31, &r, &r);
   
      // Free 5 byte low memory block
      D32DosMemFree(sel);
   
      // Notify mode called from
      printf("\tTSR: Inside callback function\n");
      
      // shutdown the TSR
	   terminate_tsr();
	}

	// returns the 32bit stack pointer
	ULONG getesp();
	#pragma aux getesp = \
		"mov eax, esp";
#else
	// DS -> SS and parameter 1 into ESP
   void restore_ss_esp(ULONG sv_esp)
   {
      _asm
         {
         mov ax, ds
         mov ss, ax
         mov esp, sv_esp
         }
   }

	extern void cdecl callback_x();

	// returns the 32bit stack pointer
	ULONG getesp()
	{
		_asm mov eax, esp
	}
#endif

void inner_handler()
{
   USHORT clientds, clientdx;

#ifdef MSVC40
	// Convert 16:16 far ptr (most likely) in crfptr to near ptr and assign 
   // to client_regs
   r.ebx = (DWORD)crfptr.sel;
   INT31(0x0006,r.ebx,&r);
   client_regs = (TSF32 *)(((DWORD)makeptr((USHORT)r.ecx,(USHORT)r.edx))+crfptr.off);
#endif
   
   // Switch the execution context to TSRDEMO's context
   D32TsrSwitchPSP(tsr_iostate, &client_iostate);

	// Report mode called from; cx == 1 for protected mode
   if( client_regs->ecx & 0xFFFF )
      printf ("\tTSR: Signalled From Protected Mode\n");
   else
      printf ("\tTSR: Signalled From Real Mode\n");

   // ax contains realmode ds, assigned in realmode handler
   clientds = client_regs->eax & 0xFFFF;
   clientdx = client_regs->edx & 0xFFFF;

   if(!clientds)
      {
      // Are we being asked for the TSR's PSP?
      if(clientdx == 0)
         {
         printf("\tTSR: Returning TSR's PSP segment = %04X\n",tsr_iostate);
         client_regs->edx = tsr_iostate;
         }
      else
         // Are we requesting a shutdown?
         if(clientdx == 1)
            {
            if( !(client_regs->ecx & 0xFFFF) )
            	// signalled from REAL MODE terminate now!
	            terminate_tsr();
            else
            	{
            	// signalled from PROTECTED MODE terminate through callback
				   DPMI_CALLREGS rmregs = {0};

               // Set up callback
#ifndef MSVC40
               r.w.ax = 0x0303;			// DPMI allocate real mode callback
               s.ds = FP_SEG((void far *) callback_x);
               r.x.esi = FP_OFF((void far *) callback_x);
               s.es = FP_SEG((void _far *) &rmregs);
               r.x.edi = FP_OFF((void _far *) &rmregs);
               int386x (0x31, &r, &r, &s);

               if (r.x.cflag)
#else
               r.ds  = getcs();
               r.esi = (DWORD)callback_x;
               r.es  = getss();
               r.edi = (DWORD)&rmregs;
               INT31(0x303,r.ebx,&r);

               if (r.eflags & 1)
#endif   
                  {
                  printf ("\tTSR: Failed to install callback.\n");
                  exit (1);
                  }
   
               // Allocate 5 bytes of low memory for far call to callback
               if (! (lowp = (char *) (D32DosMemAlloc (5,&sel) << 4)))
                  {
                  printf ("\tTSR: ERROR Out of DOS memory. Aborting\n");
                  exit (1);
                  }
   
               // Poke code (to call callback) into real mode handler
               lowp[0] = '\x9A';			// CALL FAR PTR (callback)
#ifndef MSVC40
               *((unsigned short *) &lowp[1]) = r.w.dx;  // offset
               *((unsigned short *) &lowp[3]) = r.w.cx;  // segment
#else
               *((unsigned short *) &lowp[1]) = r.edx & 0xFFFF;  // offset
               *((unsigned short *) &lowp[3]) = r.ecx & 0xFFFF;  // segment
#endif

   				// Return the low memory address in edx
               client_regs->edx = (DWORD)lowp << 12;
   
               printf("\tTSR: Returning FARCALL callback address\n");

               // Save esp for restore upon return from TSR
	            sv_esp = getesp();
               }
            }
         else
            // Unrecognizable DS and DX information
  	         printf("\tTSR: Error! DS = %x, DX = %x\n", clientds, clientdx);
      }
   else
      if(client_regs->ecx & 0xFFFF)
	      // DS:EDX points to linear address of argument(s) string from SIGPM
   	   printf("\tTSR: Arguments string from SIGPM ds:edx -> (%x:%X) %s\n", 
            clientds, client_regs->edx, (char *)client_regs->edx);
      else
	      // DS:DX points to real memory linear address of argument(s) string 
         // from SIGRM
   	   printf("\tTSR: Arguments string from SIGRM ds:dx -> (%x:%x) %s\n", 
         	clientds, clientdx, (clientds * 16 + clientdx) );

   // Switch the execution context back to the signalling program's context
   D32TsrSwitchPSP(client_iostate, &tsr_iostate);

   // Back to the signalling program
#ifndef MSVC40
   tsf32_exec(NULL, &client_context);
#else
   tsf32_exec(NULL, (USHORT)0, &client_context, (USHORT)getds());
#endif
}

main (int ac, char **av)
{
   extern void _cdecl rm_handler();
   extern void _cdecl passup();
   ULONG *passup_ptr = (ULONG *)passup;
	MEMINFO mi;

   // Entry notification
   printf("\n\tTSR: Attempting to install 0x7B TSR demo\n");

#ifdef MSVC40
	// Store DS for proper addressability in outer_handler and callback_x
	flatds = getds();
#endif
   
   // Remember this processes IO state
   D32TsrSavePSP(&tsr_iostate);

   // Remember the original real-mode handler
   old_rminterrupt = D32RmGetVector(0x7B);

   // Attach an interrupt handler to 0x7B
#ifndef MSVC40
   D32SetIntProc(0x7B, (void far *) &outer_handler);
#else
   D32SetIntProc(0x7B, (void *) &outer_handler, getcs());
#endif

   // Service all interrupts in protected mode
   D32Passup(0x7B);

   // Remember the passup handler handler
   passup_rminterrupt = D32RmGetVector(0x7B);

   // Patch passup interrupt where we can find it
   *passup_ptr = passup_rminterrupt;

   // Force real-mode handler into low memory
   new_rminterrupt = MigrateHandler((char *)rm_handler, 32);

   // And declare real-mode handler to be real interrupt
   D32RmSetVector(0x7B, new_rminterrupt);

   // Preserve an application execution context
#ifndef MSVC40
   tsf32_exec(&tsr_context, NULL);
#else
   tsf32_exec(&tsr_context, (USHORT)getds(), NULL, (USHORT)0);
#endif
   tsr_context.eflags &= ~0x200;       // clear interrupt flag

	// Report on memory status
	printf("\tTSR: Low memory %ld bytes\n\tTSR: Extended Memory %ld bytes\n",D32DosMemAvail(),D32ExtAvail(&mi));
   
   // Going TSR notification
   printf("\tTSR: About to go TSR!\n");

   // And go TSR
   D32TsrExit(TSR_INSTALL,0);
}
