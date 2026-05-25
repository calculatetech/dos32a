/***************************************************************************
 *
 *  HNDLINT.C - Source file for interrupt handlers used in the HANDLERC.C
 *		examples.
 *
 *  Copyright (c) Tenberry Software, Inc. 1996
 *  All Rights Reserved
 *
 */

#define DEMO_HELPER 1
#include <handlerc.h>

#ifdef MSVC40
void intcontinue(unsigned short com_port)
	{
   _asm
   	{
      mov   bx, [com_port]
      movzx ebx, bx
	   lea   edx, [ebx+2]
   	in    al,dx
	   mov   dx,bx
   	in    al,dx
		mov   dx,020h
	   mov   al,dl
   	out   dx,al
      }
   }
#else
// The Watcom inline assembly code shown here could just as easily have 
// been put into an assembly file, and most of it is in bmrmhand.asm, but 
// it was decided to also show how to do primitive inline assembly with 
// Watcom.
/* intcontinue2 with comments
	push	edx
	mov	bx,[_com_port]
	lea	dx,[bx+2]
	in	al,dx				; Read ports so interrupts
	mov	dx,bx			; can continue to be
	in	al,dx				; generated
	mov	dx,020h
	mov	al,dl
	out	dx,al			; Send EOI
	pop	edx
	pop	eax
	pop	ebx
*/
void intcontinue2(unsigned short com_port);
#pragma aux intcontinue2 = \
   "push  eax" \
   "push  edx" \
   "lea   edx,[bx+2]" \
   "in    al,dx" \
   "mov   dx,bx" \
   "in    al,dx" \
   "mov   dx,020h" \
   "mov   al,dl" \
   "out   dx,al" \
   "pop   edx" \
   "pop   eax" \
	parm [bx];
void intcontinue(unsigned short com_port)
{
	intcontinue2(com_port);
}
#endif

/* com_init2 with comments
   mov   ax,0F3h           ; 9600,n,8,1
   sub   dh,dh   
   mov   cx, dx            ; store com_id for later
   dec   dx                ; 0 = COM1, 1 = COM2
   int   14h               ; Initialize device
   lea   dx,[bx+5]         ; line status register
   in    al,dx   
   lea   dx,[bx+4]         ; modem control register
   in    al,dx   
   or    al,8              ; enable OUT2 interrupt
   out   dx,al   
   lea   dx,[bx+2]         ; int id register
   in    al,dx   
   mov   dx,bx             ; data receive register
   in    al,dx   
   mov   dl,NOT 10h        ; mask for IRQ4
   cmp   cx,1              ; compare to com_id
   je    maskit   
   mov   dl,NOT 8h         ; mask for IRQ3
maskit:   
   in    al,21h            ; interrupt mask register
   and   al,dl             ; force IRQ unmasked
   out   21h,al   
   lea   dx,[bx+1]         ; int enable register
   mov   al,1   
   out   dx,al             ; enable received-data int
   ret
*/

#ifdef MSVC40
void com_init(unsigned char com_id, unsigned short com_port)
{
	_asm
   	{
      mov   dl, com_id
      mov   bx, com_port
      movzx ebx,bx
      mov   ax,0F3h
      sub   dh,dh
      mov   cx, dx
      dec   dx
      int   14h
      lea   edx,[ebx+5]
      in    al,dx
      lea   edx,[ebx+4]
      in    al,dx
      or    al,8
      out   dx,al
      lea   edx,[ebx+2]
      in    al,dx
      mov   dx,bx
      in    al,dx
      mov   dl,0EFh
      cmp   cx,1
      je    maskit
      mov   dl,0F7h
maskit:
      in    al,21h
      and   al,dl
      out   21h,al
      lea   edx,[ebx+1]
      mov   al,1
      out   dx,al
      }
}
#else
void com_init2(unsigned char com_id, unsigned short com_port);
#pragma aux com_init2 = \
   "mov   ax,0F3h" \        
   "sub   dh,dh" \
   "mov   cx, dx" \         
   "dec   dx" \             
   "int   14h" \            
   "lea   edx,[bx+5]" \
   "in    al,dx" \
   "lea   edx,[bx+4]" \      
   "in    al,dx" \
   "or    al,8" \           
   "out   dx,al" \
   "lea   edx,[bx+2]" \
   "in    al,dx" \
   "mov   dx,bx" \          
   "in    al,dx" \
   "mov   dl,0EFh" \     
   "cmp   cx,1" \           
   "je    maskit" \
   "mov   dl,0F7h" \      
"maskit:" \
   "in    al,21h" \         
   "and   al,dl" \          
   "out   21h,al" \
   "lea   edx,[bx+1]" \
   "mov   al,1" \
   "out   dx,al" \          
	parm [dl] [bx];
void com_init(unsigned char com_id, unsigned short com_port)
{
	com_init2(com_id, com_port);
}
#endif

//	BI-MODAL INTERRUPT HANDLER:
//	The handler itself, and a function which determines which mode the
//	handler is running in (essential for any bi-modal handler).

// Copyright (c) Tenberry Software, Inc. 1995
// All Rights Reserved

/*
Function to determine the mode of execution (real or protected; returns
0 for real mode and 1 for protected mode).  This is essential for any
bi-modal interrupt handler.  This function must be in the same segment
as the handler so that it can be accessed by a near call; otherwise a
pointer to the function which is valid in one mode will be invalid in
the other.  (This function cannot be coded in pure 'C' without major
difficulties and highly compiler-specific tricks, if at all).
*/

/* cpu_mode2 with comments
   push    bx   
   smsw    ax                 ; store MSW in ax
   and     ax, 1              ; mask all but PE bit
   jz      return
   pushf
   pop     bx
   and     bh, 0CFh           ; IOPL to 0
   push    bx
   popf
   pushf
   pop     bx
   and     bh, 30h            ; isolate IOPL
   jz      return             ; if IOPL 0, return ax=1: not virtualizing
   mov     ax, 1686h          ; see if DPMI
   int     2Fh                ; will return ax = 0 if DPMI
   cmp     ax, 1              ; c flag set if DPMI
   sbb     ax, ax             ; zero ax unless c flag is set
return:
   pop     bx
   movzx   eax, ax            ; zero extend eax for return
   mov     edx, 0             ; zero edx for return
   ret
*/

#ifdef MSVC40
int cpu_mode()
{
	_asm
   	{
      push    bx
      smsw    ax
      and     ax, 1
      jz      return
      pushf
      pop     bx
      and     bh, 0CFh
      push    bx
      popf
      pushf
      pop     bx
      and     bh, 30h
      jz      return
      mov     ax, 1686h
      int     2Fh
      cmp     ax, 1
      sbb     ax, ax
return:
      pop     bx
      movzx   eax, ax
      mov     edx, 0
      }
};
#else
int cpu_mode2();
#pragma aux cpu_mode2 = \
   "push    bx" \
   "smsw    ax" \              
   "and     ax, 1" \           
   "jz      return" \
   "pushf" \
   "pop     bx" \
   "and     bh, 0CFh" \        
   "push    bx" \
   "popf" \
   "pushf" \
   "pop     bx" \
   "and     bh, 30h" \         
   "jz      return" \          
   "mov     ax, 1686h" \       
   "int     2Fh" \             
   "cmp     ax, 1" \           
   "sbb     ax, ax" \          
"return:" \
   "pop     bx" \
   "movzx   eax, ax" \         
   "mov     edx, 0";
int cpu_mode()
{
	return (cpu_mode2());
}
#endif

#ifndef MSVC40
/*
PROTECTED MODE INTERRUPT HANDLER:
This C protected-mode handler can be really simple because there are no
cross-mode communication/addressing issues.  All the code is specific to
the example - there's nothing you need to copy in this function except
the declaration of the function itself.
*/
void _interrupt _loadds handle_pm_only()
{
	if (!handler_data_ptr)
   	// No real mode handler was installed
   	{
		//	Just for the example - print a purple 'P' for protected-mode
		*((unsigned short *)D32RealToLinear((DWORD)screenp)) = 0x5000+'P' ;

		/*	
   	Just for the example - advance pointers to video memory (This moves 
	   what will be printed by the next call to the bimodal handler one space 
   	across the screen, so that a string of characters is printed instead of 
	   over-printing the	same character).
		*/
	   screenp++ ;
      }
   else
   	// A real mode handler was installed
   	{
		//	Just for the example - print a purple 'P' for protected-mode
		*((unsigned short *)D32RealToLinear(*handler_data_ptr)) = 0x5000+'P' ;

		/*	
	   Just for the example - advance pointers to video memory (This moves 
	   what will be printed by the next call to the bimodal handler one space 
	   across the screen, so that a string of characters is printed instead of 
	   over-printing the	same character).
		*/
	   *handler_data_ptr+=2 ;
      }

	// Allow interrupts to continue
   intcontinue(com_port);
}

/*
PROTECTED MODE INTERRUPT HANDLER:
This C protected-mode handler can be really simple because there are no
cross-mode communication/addressing issues.  All the code is specific to
the example - there's nothing you need to copy in this function except
the declaration of the function itself.
Note that the function must be declared as below for use with D32SetIntProc()
*/
int _loadds _cdecl _far handle_pm31_only(int handle, TSF32 tsf32)
{
	//	Just for the example - print a purple 'P' for protected-mode
	*((unsigned short *)D32RealToLinear((DWORD)screenp)) = 0x5000+'P' ;

   /*
   Just for the example - advance pointers to video memory (This moves
   what will be printed by the next call to the bimodal handler one space
   across the screen, so that a string of characters is printed instead of
   over-printing the same character).
   */
   screenp++ ;

	// Allow interrupts to continue
   intcontinue(com_port);

	// Must return either 1 to chain onwards or 0 to end processing of interrupt
   return (0);      // don't chain onwards to default handler
}
#endif
