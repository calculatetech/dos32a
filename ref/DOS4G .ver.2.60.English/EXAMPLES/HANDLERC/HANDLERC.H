/***************************************************************************
 *
 *  demo1c.h -- Typedef for structure used to pass data to real-mode handler
 *		and external data and function declarations.
 *
 *  Copyright (c) Tenberry Software, Inc. 1996
 *  All Rights Reserved
 *
 */

// D32 API Header
#include <dos32lib.h>

//	EXTERNAL DECLARATIONS:

#ifdef DEMO_HELPER

	// pointer to BIOS timer tick count.  For the example only. 
	extern int FarPtr tickspt;
   
	extern unsigned short com_port;

	//	handler_data_ptr() is used to store the real address of handler_data, 
   // which will be moved low in cases where this variable is used.  You
	//	need to do this if you have a bi-modal or real-mode interrupt handler!
	extern unsigned long *handler_data_ptr;

   // pointer into video memory
   extern unsigned short FarPtr screenp;    

#else

	// COM port initialization function
   extern void com_init (unsigned char com_id, unsigned short com_port);

#ifdef MSVC40
   // the bi-modal interrupt handler 
	extern handle_both(void);

   // the protected-mode interrupt handler installed via Interrupt 21
   extern handle_pm_only(void);

   // the protected-mode interrupt handler installed via D32SetIntProc
   extern int handle_pm31_only(int handle, TSF32 tsf32);

	// the real-mode interrupt handler 
   extern handle_rm_only(void);

	// the bimodal-same interrupt handler 
   extern bmhandler(void);
#else
   // the bi-modal interrupt handler 
	extern _interrupt _loadds handle_both(void);

   // the protected-mode interrupt handler installed via Interrupt 21
	extern _interrupt _loadds handle_pm_only(void);

   // the protected-mode interrupt handler installed via D32SetIntProc
   extern int _loadds _cdecl _far handle_pm31_only(int handle, TSF32 tsf32);

	// the real-mode interrupt handler 
	extern _interrupt _far handle_rm_only(void);

	// the bimodal-same interrupt handler 
	extern _interrupt _far bmhandler(void);
#endif

	// low memory data pointers
   extern unsigned short hdp_rmseg;
   extern unsigned short hdp_rmseg_alias;
   extern unsigned short screen_alias;

	// low memory store for com port choice
   extern unsigned short com_port_low;
                         
#endif                   
