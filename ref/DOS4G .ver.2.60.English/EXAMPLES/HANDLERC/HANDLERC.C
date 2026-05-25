/*
   handlerc.c:

   Copyright (c) Tenberry Software, Inc. 1996
   All Rights Reserved

   Example showing how to handle hardware interrupts under DOS/4G,
   whether the processor happens to be in real mode or protected mode
   at the time of the interrupt.

   Usage :

      Protected mode handler only using D32RmSetVector and D32RmGetVector
         c:\> handlerc -1 <-c>

      Protected mode handler only, as auto-passup, using D32DPMISetPmVector
         and D32DPMIGetPmVector
         c:\> handlerc -2 <-c>

      Bimodal handler, real mode and protected mode use same handler
      	A combination of D32RmSetVector and D32DPMIPmSetVector is used
         c:\> handlerc -3 <-c>

      Seperate real mode and protected mode handlers
         This is the most efficient way of handling an interrupt.
         c:\> handlerc -4 <-c>

      Real mode handler only located in low memory, original protected
         mode handler is passdown
         c:\> handlerc -5 <-c>

   The program installs protected-mode and real-mode handlers for
   interrupt 0xC (otherwise known as COM1 or IRQ4), or alternatively 
   interrupt 0xB (otherwise known as COM2 or IRQ5), that write either a 
   'P' or an 'R' to an absolute address in the region of the screen memory.
   So while the program runs (at 9600 baud), you can see who is handling 
   the interrupts.

   Note that an external DPMI host (such as Windows) will reflect real
   mode interrupts to a protected mode handler rather than allowing the
   real mode handler to get them; installing a real mode handler in such
   an environment is not necessary.  However, it doesn't hurt to do so,
   so this program will handle interrupts as quickly as possible
   whether or not a DPMI host exists.

   The program is written to the DPMI (INT 31h) standard, so the code
   is the same for both environments.

   A mouse attached to either COM1 pr COM2 makes a suitable demo (you 
   will have to load the mouse driver in order to enable the mouse hardware).
   You can type on the keyboard as you move the mouse around; pressing
   the Esc key ends the program.  You can also transmit data from a
   remote machine at 9600 baud; in this case, you don't need to load
   any drivers before running the program.
*/

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <time.h>

// D32API header
#include <dos32lib.h>

// Special header for interrupt handler example
#include <handlerc.h>

// Low memory pointers
USHORT FarPtr screenp;
ULONG *handler_data_ptr;

// Bios data area pointer
DWORD tickspt = 0L;                 

// Com port information
UCHAR com_id = 1;
UCHAR com_int = 0x0C;
USHORT com_port = 0x3F8;

// Original vector storage
DWORD orig_rm_vect;
#ifdef MSVC40
	FarPtr32 orig_pm_vect;
	// Temp used for vector passing
	FarPtr32 fp32;
#else
   void FarPtr orig_pm_vect;
#endif

// Used for benchmark function
ULONG time1, time2;

// Used to store low memory allocation selectors
USHORT low_sel, low_sel2;

#ifdef MSVC40
	void signalint()
	{
		_asm int 0Ch
	};
#else
	void signalint();
	#pragma aux signalint = \
		"int 0Ch";
#endif

// Returns a 16:32 void pointer
#ifdef MSVC40
	void * makeptr32(int s,int o)
#else
	void FarPtr makeptr32(int s,int o)
#endif
{
	void FarPtr ptr;

	FP_SEG32(ptr) = s;
	FP_OFF32(ptr) = o;

	return (ptr);
}

// A mode switching escape key press detector
void waitforescape(int bm)
{
	DPMI_CALLREGS dr;
	USHORT i;

   if ( bm )
   	// Benchmark only
   	{
	   memset (&dr, 0, sizeof(DPMI_CALLREGS));
      
      // Record start time
      time1 = clock();

		// Signal 1000 in real mode
      for (i=0;i<1000;i++)
	      D32RmInterrupt(com_int,&dr);

      // Signal 1000 in protected mode
      for (i=0;i<1000;i++)
      	signalint();
      
      // Record end time
      time2 = clock();
      }
	else
   	// Regular method
   	{
	   // Wait for the ESC key to be pressed.  This loop has a good mix of
   	// time spent in real mode and protected mode, so both real and 
	   // protected mode handlers will get called.  
   	while (1)
	      {
   	   // Explicitly go down to real mode to ask if a key is ready.
      	// (The kbhit() call is simpler to code, but extra transfers to
	      // real mode may be optimized out by the DOS extender.)
   	   memset (&dr, 0, sizeof(DPMI_CALLREGS));
	     	dr.eax = 0x0100;
   	   D32RmInterrupt(0x16,&dr);     // DPMI signal real mode interrupt

      	if (! (dr.flags & 0x40))      // Test zero flag
	         // If the escape key is pressed exit
            if ((dr.eax & 0xff) == 27)
      	      break;
	      }
      }
}

/*	
	This function demonstrates how to install a real mode interrupt handler
   and leave the protected mode handler pass-down (its initial, default
   state).
   Using a real-mode handler and leaving the protected-mode handler as
   pass-down allows interrupts signaled in either mode to be handled by the
   same code, which you supply.  However, a mode switch is required when the
   interrupt is signaled in protected mode.  The real-mode interrupt handler
   must be loaded into low memory and cannot use extended memory at all.
   Communications between a protected mode program and any function running
   in real mode are tricky, as this example illustrates.  If handling the
   interrupt efficiently when it is signaled in real mode is a paramont
   consideration, or if communications with the protected mode program are
   not important (i.e. no shared data), this configuration may make sense.
   Otherwise it is not recommended.
*/

// Real mode handler only located in low memory, original protected
void realmodehandler(int bm)
{
	void *lowp;

   // Save the starting real-mode and protected-mode handler addresses.
   orig_rm_vect = D32RmGetVector(com_int);

   // Allocate DOS memory for the real-mode handler. Then copy the bimodal-
   // same and realmode handler code into that segment.  The com_port value, 
   // and handler_data address are copied at the same time.
   com_port_low = com_port;
   hdp_rmseg = D32RealSeg(handler_data_ptr);
   lowp = (void*)(D32SegMoveLow((ULONG)&handle_rm_only,256,&low_sel) << 4);

   // Install the real mode handler.  The memory touched by the real mode
   // handler needs to be locked, in case we are running under VMM
   // or an external DPMI host.
   D32RmSetVector(com_int,D32LinearToReal((DWORD)lowp));
   D32Lock((void *)lowp,256);             // lock 256 bytes

   // Initialize COM port.
   com_init (com_id, com_port);

	if (!bm)
	   puts ("Move the mouse or transmit data; press ESC to cancel\n");

   // Wait for the ESC key to be pressed.  This loop has a good mix of
   //   time spent in real mode and protected mode, so the upper left
   //   hand corner of your color screen will toggle between 'R' and 'P'.
   waitforescape(bm);

   // Clean up.

   // Reset Vectors
   D32RmSetVector(com_int,orig_rm_vect);
   // Free low memory used
   D32DosMemFree(low_sel);
   D32DosMemFree(low_sel2);
}

/*	
	This function demonstrates how to install separate protected mode and real
   mode handlers for the same interrupt.
   Using separate handlers allows your program to service interrupts whether
   they are signaled in real mode or protected mode, and no mode switch
   occurs when the interrupt is signaled.  However, two separate interrupt
   handlers are required.  (This does allow interrupts signaled in real mode
   to be handled differently from those signaled in protected mode, if that
   is what your program requires).  The real mode interrupt handler must be
   loaded into low memory and cannot use extended memory at all.
   Communications between a protected mode program and any function running
   in real mode are tricky, as this example illustrates.  Don't do this
   unless you really need to.
*/

// Seperate real mode and protected mode handlers
#ifdef MSVC40
extern WORD flatds1;
#endif
void bimodal_seperate(int bm)
{
	void *lowp;

   // Save the starting real-mode and protected-mode handler addresses.
   orig_rm_vect = D32DPMIRmGetVector(com_int);
#ifdef MSVC40
   D32DPMIPmGetVector(com_int,&orig_pm_vect);
#else
   orig_pm_vect = D32DPMIPmGetVector(com_int);
#endif

   // Allocate DOS memory for the real-mode handler. Then copy the bimodal-
   // same and realmode handler code into that segment.  The com_port value, 
   // and handler_data address are copied at the same time.
   com_port_low = com_port;
   hdp_rmseg = D32RealSeg(handler_data_ptr);
   lowp = (void*)(D32SegMoveLow((ULONG)&handle_rm_only,256,&low_sel) << 4);

   // Install the new handlers.  The memory touched by the protected mode
   // handler needs to be locked, in case we are running under VMM
   // or an external DPMI host.
#ifdef MSVC40
   fp32.sel = getcs();
   fp32.off = (DWORD)handle_pm_only;
   D32DPMIPmSetVector(com_int,&fp32);
   D32DPMIRmSetVector(com_int,D32LinearToReal((DWORD)lowp));

	flatds1 = (WORD)getds();

   D32Lock((void *)lowp,256);              // lock 256 bytes
   D32Lock((void *)&com_port,2);
   D32Lock((void *)flatds1,256);
#else
   D32DPMIPmSetVector(com_int,(void FarPtr)handle_pm_only);
   D32DPMIRmSetVector(com_int,D32LinearToReal((DWORD)lowp));

   D32Lock((void *)lowp,256);              // lock 256 bytes
   D32Lock((void *)&com_port,2);
#endif

   // Initialize COM port.
   com_init (com_id, com_port);

   if (!bm)
   	puts ("Move the mouse or transmit data; press ESC to cancel\n");

   // Wait for the ESC key to be pressed.  This loop has a good mix of
   //   time spent in real mode and protected mode, so the upper left
   //   hand corner of your color screen will toggle between 'R' and 'P'.
   waitforescape(bm);

   // Clean up.

   // Reset Vectors
   D32DPMIRmSetVector(com_int,orig_rm_vect);
#ifdef MSVC40
   D32DPMIPmSetVector(com_int,&orig_pm_vect);
#else
   D32DPMIPmSetVector(com_int,orig_pm_vect);
#endif

   // Free low memory used
   D32DosMemFree(low_sel);
   D32DosMemFree(low_sel2);
}

/*	
	This function demonstrates how to install a single handler as both the
   protected mode and the real mode handler for an interrupt; this is known
   as a true bi-modal interrupt handler.
   Using a bi-modal handler allows the same code to service interrupts
   whether they are signaled in real mode or protected mode, and no mode
   switch occurs when the interrupt is signaled.  However, the bi-modal
   handler must be able to run in real mode, which means that the code must
   be loaded into low memory and that the real mode handler will not be able 
   to use extended memory at all.  Communications between a protected mode 
   program and any function running in real mode are tricky, as this example
   illustrates.  We recommend using a bimodal-same handler only if you really
   need to.
   This function illustrates a true bi-modal interrupt handler, where the 
   same code runs in both real and protected modes.  Another possible 
   configuration is a seperate-bimodal handler, where separate real- and 
   protected-mode stubs call a common (protected mode) function which does 
   most of the work.  This avoids some of the limitations of a true bi-modal 
   handler, but requires some mode switching.  
*/

// Bimodal handler, real mode and protected mode use same handler
void bimodal_same(int bm)
{
	void *lowp;
   WORD screenp_ptr_alias, screenp_alias, bmhandler_alias;
	ULONG pmscreenp;
   
   // Save the starting real-mode and protected-mode handler addresses.
   orig_rm_vect = D32RmGetVector(com_int);
#ifdef MSVC40
   D32DPMIPmGetVector(com_int,&orig_pm_vect);
#else
   orig_pm_vect = D32DPMIPmGetVector(com_int);
#endif

	// Set up the various aliases and values in the bm handler code space

   // Set the com port
   com_port_low = com_port;
   // Set the real mode handler screenp pointer
   hdp_rmseg = D32RealSeg(handler_data_ptr);
   // Alias the real mode screenp pointer
   screenp_ptr_alias = D32SegAlias((unsigned int)handler_data_ptr);
   hdp_rmseg_alias = screenp_ptr_alias;
   // Alias the real mode screenp for protected mode accessing
   pmscreenp = (ULONG)(screenp) >> 12;
   screenp_alias = D32SegAlias((unsigned int)pmscreenp);
   screen_alias = screenp_alias;

   // Allocate DOS memory for the real-mode handler. Then copy the bimodal-
   // same and realmode handler code into that segment.  The com_port value, 
	// segment aliases, and handler_data address are copied at the same time.
   lowp = (void*)(D32SegMoveLow((ULONG)&bmhandler,256,&low_sel) << 4);

	// Force code to be referenced through a USE16 code selector so that 
   // execution is in 16bit mode. Must do this to have the bimodal 16bit
   // code run correctly
	bmhandler_alias = D32CreateSegAlias((unsigned int)lowp,CODE_SEG,USE16,LINEAR);

   // Install the new handlers.  The memory touched by the protected mode
   // handler needs to be locked, in case we are running under VMM
   // or an external DPMI host.
#ifdef MSVC40
   fp32.sel = bmhandler_alias;
   fp32.off = 0L;
   D32DPMIPmSetVector(com_int,&fp32);
#else
   D32DPMIPmSetVector(com_int,(void FarPtr)makeptr32(bmhandler_alias,0));
#endif
   D32DPMIRmSetVector(com_int,D32LinearToReal((DWORD)lowp));
   D32Lock((void *)lowp,256);             // lock 256 bytes

   // Initialize COM port.
   com_init (com_id, com_port);

   if (!bm)
   	puts ("Move the mouse or transmit data; press ESC to cancel\n");

   // Wait for the ESC key to be pressed.  This loop has a good mix of
   // time spent in real mode and protected mode, so the upper left
   // hand corner of your color screen will toggle between 'R' and 'P'.
   waitforescape(bm);

   // Clean up.

   // Reset vectors
   D32DPMIRmSetVector(com_int,orig_rm_vect);
#ifdef MSVC40
   D32DPMIPmSetVector(com_int,&orig_pm_vect);
#else
   D32DPMIPmSetVector(com_int,orig_pm_vect);
#endif

   // Free low memory used
   D32DosMemFree(low_sel);
   D32DosMemFree(low_sel2);
   // Free aliasing selectors
   D32DescFree(screenp_ptr_alias);
   D32DescFree(screenp_alias);
   D32DescFree(bmhandler_alias);
}

/*	
	This function demonstrates how to install a protected-mode interrupt
   handler in the auto-passup range as a plain protected-mode interrupt
   handler (using D32DPMISetPmVector()).
   When the interrupt is signaled from protected mode, the interrupt handler
   we installed will handle it.  When the interrupt is signaled from real
   mode, the original real-mode handler will handle it.  This configuration
   may be useful if you know that an interrupt will always be signaled from
   protected mode, or if different behavior is required when the interrupt
   is signaled from protected mode than when it is signaled from real mode.
   In particular, it can be useful when you need to provide protected-mode
   replacements for standard real-mode service requests (such as INT 21h).
*/

// Protected mode handler only, as auto-passup, using D32DPMISetPmVector
extern WORD flatds2;
void pmhandler31(int bm)
{
	int handle;
   
   // Install the protected mode handler.  The memory touched by
   // the protected mode handler needs to be locked, in case we
   // are running under VMM or an external DPMI host.
#ifdef MSVC40
   handle = D32SetIntProc(com_int,(void *)handle_pm31_only,getcs());
   D32Lock((void *)&flatds2,256);  // lock 256 bytes inc flatds2
   flatds2 = (WORD)getds();
#else
   handle = D32SetIntProc(com_int,(void FarPtr)handle_pm31_only);
   D32Lock((void *)handle_pm31_only,256);             // lock 256 bytes
#endif

   // Initialize COM port.
   com_init (com_id, com_port);

   if (!bm)
   	puts ("Move the mouse or transmit data; press ESC to cancel\n");

   // Wait for the ESC key to be pressed.  This loop has a good mix of
   // time spent in real mode and protected mode, so the upper left
   // hand corner of your color screen will toggle between 'R' and 'P'.
   waitforescape(bm);

   // Clean up.
   D32CancelIntProc(handle);
}

/*	
	This function demonstrates how to install a protected-mode interrupt
   handler in the auto-passup range as a passup interrupt (using a 
   protected-mode INT 21, function 25, call via the dos_setvect() ANSI-C 
   library function).

   With a passup interrupt, whether the interrupt is signaled in real mode
   or in protected mode, it is handled by the protected mode interrupt
   handler.  Normally, this is a very good way to handle interrupts.  The
   handler can be in extended memory, it has full access to global memory
   structures, and a single piece of source code handles both real and
   protected mode interrupts.  There are only two draw-backs: 1.) When the
   interrupt is signaled in real-mode, there is some extra over-head to
   switch to protected mode before handling the interrupt and switch back
   again on return.  (The extra over-head is normally not a concern unless
   performance is critical, as in processing high-speed communications
   interrupts). 2.) If the interrupt handler must chain to the original
   real-mode interrupt handler, this introduces an additional mode switch
   (and can create complications if the passup vector is set with
   D32Passup()).
*/

// Protected mode handler only using D32RmSetVector and D32RmGetVector
void pmhandler21(int bm)
{
#ifdef MSVC40
	D32REGS r = {0};
#endif
   
   // Save the starting real-mode and protected-mode handler addresses.
   orig_rm_vect = D32RmGetVector(com_int);

   // Install the protected mode handler.  The memory touched by
   // the protected mode handler needs to be locked, in case we
   // are running under VMM or an external DPMI host.

   // D32RmSetVector can not be used here instead of _dos_setvect because
   // D32RmSetVector signals a real-mode INT 21, function 25, which, if 
   // following the syntax used below, would set only the real-mode 
   // interrupt vector with an invalid protected-mode address.
#ifdef MSVC40
	r.ds  = getcs();
   r.edx = (DWORD)&handle_pm_only;
   INT21(0x2500 | com_int,r.ebx,&r);
   D32Lock((void *)&flatds1,256);             // lock 256 bytes
	flatds1 = (WORD)getds();
#else
   _dos_setvect(com_int,handle_pm_only);
   D32Lock((void *)handle_pm_only,256);             // lock 256 bytes
#endif

   // Initialize COM port.
   com_init (com_id, com_port);

   if (!bm)
   	puts ("Move the mouse or transmit data; press ESC to cancel\n");

   // Wait for the ESC key to be pressed.  This loop has a good mix of
   // time spent in real mode and protected mode, so the upper left
   // hand corner of your color screen will toggle between 'R' and 'P'.
   waitforescape(bm);

   // Clean up.
   D32RmSetVector(com_int,orig_rm_vect);
}

void hd_setup(int rmhandler_used)
{
   // Set real-mode screen pointer to video memory address
   // -- correct address depends on video mode
   screenp = (USHORT *)(((*(char FarPtr)D32RealToLinear(tickspt) == 7)
      ? 0xB000 : 0xB800) << 16) ;

	if (rmhandler_used)
      // Move screenp low and set handler_data_ptr equal to new low 
      // memory position of screenp
	   handler_data_ptr = (ULONG*)(D32SegMoveLow((ULONG)&screenp,sizeof(screenp),&low_sel2) << 4);
}

// Benchmarks are VERY INACCURATE please ignore results
// Should be redone with a nano-second timer
void benchmarkall()
{
	// Time 1000 int 33 operations and average for each method
   // The granularity on the results from this method of getting time
   // estimates is poor. Should patch in nanosecond timer for accuracy.

	ULONG ave1, ave2, ave3, ave4, ave5;

	// Reset to use COM1 interrupt
	com_id = 1;
	com_int = 0x0C;
	com_port = 0x3F8;

	// Method 1
	hd_setup(0);
   pmhandler21(1);
   ave1 = time2 - time1;
	
   // Method 2
	hd_setup(0);
   pmhandler31(1);
   ave2 = time2 - time1;

   // Method 3
	hd_setup(1);
   bimodal_same(1);
   ave3 = time2 - time1;

   // Method 4
	hd_setup(1);
   bimodal_seperate(1);
   ave4 = time2 - time1;

   // Method 5
	hd_setup(1);
   realmodehandler(1);
   ave5 = time2 - time1;

	// Clear the screen for printout
   system("cls");

   // Print the results
   printf("AVERAGES\n");
   printf("Be aware that these are just relative time periods and that a\n");
   printf("fair bit of overhead is being included in each of these estimates\n\n");
   printf("Method 1 Clock ticks for 2000 interrupts %lu\n",ave1);
   printf("Method 2 Clock ticks for 2000 interrupts %lu\n",ave2);
   printf("Method 3 Clock ticks for 2000 interrupts %lu\n",ave3);
   printf("Method 4 Clock ticks for 2000 interrupts %lu\n",ave4);
   printf("Method 5 Clock ticks for 2000 interrupts %lu\n\n",ave5);
}

void main (int argc, char *argv[])
{
   int choice = 0;
   
   // Require the choosing of a method for handling the interrupt
   if (argc < 2)
      goto badarg;

   // Fail if more than two options on command line
   if (argc > 3)
      goto badarg;

   // Parse command line options
   while (argc-- > 1)
      {
      if (argv[argc][0] != '-')
         {
badarg:
         printf ("Invalid argument '%s'\n", argv[argc]);
         printf ("Valid options are:\n");
         printf ("\t-1  PM handler only using interrupt 21, original RM vector\n");
         printf ("\t-2  PM handler only using DPMI interrupt 31, original RM vector\n");
         printf ("\t-3  Bimodal, single handler for both RM & PM\n");
         printf ("\t-4  Seperate RM & PM handlers\n");
         printf ("\t-5  Real mode handler installed in low memory,\n");
         printf ("\t    original PM Handler pass-down\n");
//         printf ("\t-6  Primitive Benchmark of all previous 5 methods,\n");
         printf ("\t-c  To specify COM2 as your com port : default is COM1\n");
         exit (1);
         }

		//	Initialize pointers to screen used by the interrupt handler and to 
		//	the BIOS tick counter used by main()  (Just for the example).
		tickspt = (0x0040 << 16) + (0x0049) ;     // BIOS data area + video mode

      // Perform the desired interrupt handling method
      switch (argv[argc][1])
         {
         case '1':
            {
            hd_setup(0);
            choice = 1;
            break;
            }
         case '2':
            {
            hd_setup(0);
            choice = 2;
            break;
            }
         case '3':
            {
            hd_setup(1);
            choice = 3;
            break;
            }
         case '4':
            {
            hd_setup(1);
            choice = 4;
            break;
            }
         case '5':
            {
            hd_setup(1);
            choice = 5;
            break;
            }
//         case '6':
//         	{
//            choice = 6;
//            break;
//            }
         case 'c':
            {
            com_id = 2;
            com_int = 0x0B;
            com_port = 0x2F8;
            printf ("\nUsing COM2 instead of COM1 for testing\n\n");
            break;
            }
         default:
            // Fail if invalid option
            goto badarg;
         }

      // Fail if more than two characters in option string
      if (argv[argc][2])
         goto badarg;
      }

	switch (choice)
   	{
      case 1 :
      	{
         printf ("\nHandling protected mode vector %s hooked using interrupt 21\n",com_int == 0xC ? "0xC" : "0xB");
         printf ("Original real mode vector chained to from PM handler\n");
         printf ("%s Vector is auto-passup by default\n",com_int == 0xC ? "0xC" : "0xB");
         printf("\nHit any key to proceed with your choice.\nHit escape to stop the example\n");
		   getchar();
         pmhandler21(0);
         break;
         }
      case 2 :
      	{
         printf ("\nHandling protected mode vector %s hooked using DPMI interrupt 31\n",com_int == 0xC ? "0xC" : "0xB");
         printf ("Original real mode vector chained to from PM handler\n");
         printf ("%s Vector is auto-passup by default\n",com_int == 0xC ? "0xC" : "0xB");
         printf("\nHit any key to proceed with your choice.\nHit escape to stop the example\n");
		   getchar();
         pmhandler31(0);
         break;
         }
      case 3 :
      	{
         printf ("\nHandling both protected and real mode vector %s with the same\n",com_int == 0xC ? "0xC" : "0xB");
         printf ("   protected mode handler hooked using a combination of interrupt\n");
         printf ("   21 and 31\n");
         printf ("Original real mode vector chained to from pm handler\n");
         printf ("%s Vector is auto-passup by default\n",com_int == 0xC ? "0xC" : "0xB");
         printf("\nHit any key to proceed with your choice.\nHit escape to stop the example\n");
		   getchar();
         bimodal_same(0);
         break;
         }
      case 4 :
      	{
         printf ("\nHandling protected mode & real mode vector %s with seperate\n",com_int == 0xC ? "0xC" : "0xB");
         printf ("   handlers hooked using interrupt 21\n");
         printf ("Original real mode vector chained to from pm handler\n");
         printf ("%s Vector is auto-passup by default\n",com_int == 0xC ? "0xC" : "0xB");
         printf("\nHit any key to proceed with your choice.\nHit escape to stop the example\n");
		   getchar();
         bimodal_seperate(0);
         break;
         }
      case 5 :
      	{
         printf ("\nHandling real mode vector %s with only a low memory handler\n",com_int == 0xC ? "0xC" : "0xB");
         printf ("   hooked using interrupt 21\n");
         printf ("Original real mode vector chained to from pm handler\n");
         printf ("%s Vector is auto-passup by default\n",com_int == 0xC ? "0xC" : "0xB");
         printf("\nHit any key to proceed with your choice.\nHit escape to stop the example\n");
		   getchar();
         realmodehandler(0);
         break;
         }
//      case 6 :
//         {
//         printf ("\nBenchmarking all 5 interrupt handling methods\n");
//         printf ("\nDO NOT TOUCH YOUR MOUSE OR THE RESULTS MAY NOT BE ACURATE\n");
//         printf ("\nThis test is incomplete please ignore the results\n");
//         benchmarkall();
//         break;
//         }
      }
      
//	if (choice != 6)
   	// Leave the screen fresh on exit
	   system("cls");
}
