IFDEF MSVC40

;=======================================================================
;
;  BMRMHAND.ASM - 
;
;	Copyright (c) Tenberry Software, Inc. 1996
;
;=======================================================================
;
;	DESCRIPTION:
;
;  Interrupt handler for Bimodal Same and Real Mode INT 0xC, or 0x0B if
;  the -c command-line option is used.  See the handlerc.c example program
;  for details.
;
;=======================================================================

;***********************************************************************
;                       A T T R I B U T E S
;***********************************************************************

	.386p
	.MODEL	FLAT,C

;***********************************************************************
;                         I N C L U D E S
;***********************************************************************

;***********************************************************************
;                E Q U A T E S   A N D   M A C R O S
;***********************************************************************

;***********************************************************************
;                             D A T A
;***********************************************************************

	.DATA

;***********************************************************************
;                             C O D E
;***********************************************************************

	.CODE

;----------------------------------------------------------------------
;
;  
;
;----------------------------------------------------------------------

   PUBLIC com_port_low, hdp_rmseg, hdp_rmseg_alias, screen_alias, bmhandler

handle_rm_only     PROC FAR PUBLIC

	pusha
	push  es
   push  ds
	jmp   setreal

bmhandler	db  090h	; nop

	; mode identification code
	pusha
	push  es
   push  ds

;  mov   edi, 0                  ; store MSW in ax
	db		0BFh
   dw    0

   db    0Fh                     ; 
   db    01h                     ; 
   db 	0E0h							; smsw ax
   and   eax, 1						; mask all but PE bit
   jz    setreal						; if real mode indicated set up for RM processing
   pushfd
   pop   ebx
   and   bh, 0CFh						; IOPL to 0
   push  ebx
   popfd
   pushfd
   pop   ebx
   and   bh, 30h						; isolate IOPL
   jz    doprot                  ; if IOPL 0, not virtualizing, avoid RM processing

;  mov   eax, 1686h					; see if DPMI
	db    0B8h
   dw		01686h
   
   int   2Fh                     ; will return ax = 0 if DPMI
   and   eax, 1                  ; c flag set if DPMI 
   jnz   doprot                  ; if DPMI avoid RM processing

setreal: 
;	mov   di, 1							; set to 1 for real mode
	db    0BFh
   db    1
   db    0

doreal:
	db    0BBh							; mov bx,...
hdp_rmseg DW ?         			   ; handler_data_ptr segment
;  mov   es, bx       
	db		08Eh
   db    0C3h

;  mov   ebx, 0
	db		0BBh
   dw    0
   
;  mov   ds, WORD PTR es:[bx+2]  ; screenp segment
	db    026h
	db    08Eh
	db    05Fh
	db    002h

;  mov   bx, WORD PTR es:[bx]    ; screenp offset
	db    026h
	db    08Bh
	db    01Fh

;  mov   dx, bx                  ; restore handler_data_ptr offset & save screenp offset
	db    089h
	db    0DAh

;  mov   ebx, 0
	db		0BBh
   dw    0

;  add   WORD PTR es:[bx], 2		; increment sceenp to next screen location
	db    026h
	db    083h
	db    007h
	db    002h

;  xchg  dx, bx						; restore screenp offset
   db    087h
   db    0DAh

;	mov   ax, 'R'
   db    0B8h
   db    052h
   db    000h

;  add   ax, 04000h
   db    005h
   db    000h
   db    040h

;	mov	WORD PTR [bx],ax			; Write a red 'R' to screen memory 
   db    089h
   db    007h

	jmp   onwards

doprot:
	db    0BBh							; mov bx,...
screen_alias DW ?                ; screenp USE16 alias
;  mov   ds, bx
   db    08Eh
   db    0DBh

	db    0BBh							; mov bx,...
hdp_rmseg_alias DW ?             ; handler_data_ptr USE16 alias
;  mov   es, bx       
	db		08Eh
   db    0C3h

;  mov   bx, 0
   db    0BBh
   dw    000h

;  mov   bx, es:[bx]             ; screenp offset
	db    026h
	db    08Bh
	db    01Fh

;  mov   dx, bx
 	db    089h
	db    0DAh

;  mov   ebx, 0
	db		0BBh
   dw    000h

;  add   WORD PTR es:[bx], 2		; increment pmsceenp to next screen location
	db    026h
	db    083h
	db    007h
	db    002h

;  xchg  dx, bx						; restore screenp offset
   db    087h
   db    0DAh

;	mov   ax, 'P'
   db    0B8h
   db    050h
   db    000h

;	add   ax, 05000h
   db    005h
   db    000h
   db    050h

;	mov	WORD PTR [bx],ax			; Write a purple 'P' to screen memory 
   db    089h
   db    007h

onwards:
   db 	0BBh                    ; mov bx,...
com_port_low  DW ?               ; com port base address

;  lea   dx,[bx+2]               ; int id register
   db    08Dh
   db    057h
   db    002h

   in    al,dx                   ; Read ports so interrupts

;  mov   dx,bx                   ; can continue to be
   db    089h
   db    0DAh

   in 	al,dx                   ; generated

;  mov   dx,020h
   db    0BAh
   db    020h
   db		000h
	
;	mov	al,dl
   db    088h
   db    0D0h

   out   dx,al                   ; Send EOI
   pop   ds								; restore ds & es registers to original state
   pop   es
   cmp   edi, 1						; check if handling in protected mode
   jne   protret						; if protected mode do appropriate return

rmret:
   popa                          ; restore other registers to original state
;  iret                          ; real mode iret to RM 16:16
   iretd

protret:
   popa                          ; restore other registers to original state
;  iretd                         ; protected mode iret to PM 16:32
   iret

handle_rm_only ENDP

;***********************************************************************
;                       M O D U L E   E N D
;***********************************************************************

;----------------------------------------------------------------------
;
;
;----------------------------------------------------------------------

ELSE

;**
;** bmrmhand.ASM:
;**
;** Interrupt handler for Bimodal Same and Real Mode INT 0xC, or 0x0B if 
;** the -c command-line option is used.  See the handlerc.c example program
;** for details.
;**
;** Copyright (c) Tenberry Software, Inc. 1996
;** All Rights Reserved
;**

.386

_TEXT16	SEGMENT	BYTE PUBLIC USE16 'CODE'
	ASSUME	cs:_TEXT16

;**
;** The bimodal-same/real-mode interrupt handler is in a 16-bit code segment
;** so that the assembler will generate the right code.
;**
;** We will copy this code to a 16-bit segment in low memory
;** and install it as either a real mode only interrupt handler or
;** as both a rel mode and protected mode interrupt handler before it
;** gets executed.  
;**
;** Remember there is no distinction between code and data in real mode.
;**

   PUBLIC bmhandler_, _com_port_low, _hdp_rmseg, _hdp_rmseg_alias, _screen_alias
   PUBLIC handle_rm_only_
   
handle_rm_only_:
	pusha
	push  es
   push  ds
	jmp   setreal
   
bmhandler_:
	; mode identification code
	pusha
	push  es
   push  ds
   mov   di, 0                   ; store MSW in ax
   db    0Fh                     ; 
   db    01h                     ; 
   db 	0E0h							; smsw ax
   and   ax, 1							; mask all but PE bit
   jz    setreal						; if real mode indicated set up for RM processing
   pushf
   pop   bx
   and   bh, 0CFh						; IOPL to 0
   push  bx
   popf
   pushf
   pop   bx
   and   bh, 30h						; isolate IOPL
   jz    doprot                  ; if IOPL 0, not virtualizing, avoid RM processing
   mov   ax, 1686h					; see if DPMI
   int   2Fh                     ; will return ax = 0 if DPMI
   and   ax, 1                   ; c flag set if DPMI 
   jnz   doprot                  ; if DPMI avoid RM processing

setreal: 
	mov   di, 1							; set to 1 for real mode

doreal:
	db    0BBh							; mov bx,...
_hdp_rmseg DW ?         			; handler_data_ptr segment
   mov   es, bx       
   mov   bx, 0
   mov   ds, es:[bx+2]           ; screenp segment
   mov   bx, es:[bx]             ; screenp offset
   mov   dx, bx                  ; restore handler_data_ptr offset & save screenp offset
   mov   bx, 0
   add   WORD PTR es:[bx], 2		; increment sceenp to next screen location
   xchg  dx, bx						; restore screenp offset
	mov   ax, 'R'
	add   ax, 04000h
	mov	WORD PTR [bx],ax			; Write a red 'R' to screen memory 
	jmp   onwards

doprot:
	db    0BBh							; mov bx,...
_screen_alias DW ?               ; screenp USE16 alias
   mov   ds, bx
	db    0BBh							; mov bx,...
_hdp_rmseg_alias DW ?            ; handler_data_ptr USE16 alias
   mov   es, bx       
   mov   bx, 0
   mov   bx, es:[bx]             ; screenp offset
   mov   dx, bx
   mov   bx, 0
   add   WORD PTR es:[bx], 2		; increment pmsceenp to next screen location
   xchg  dx, bx						; restore screenp offset
	mov   ax, 'P'
	add   ax, 05000h
	mov	WORD PTR [bx],ax			; Write a purple 'P' to screen memory 

onwards:
   db 	0BBh                    ; mov bx,...
_com_port_low  DW ?              ; com port base address
   lea   dx,[bx+2]               ; int id register
   in 	al,dx                   ; Read ports so interrupts
   mov   dx,bx                   ; can continue to be
   in 	al,dx                   ; generated
   mov   dx,020h
	mov	al,dl
   out   dx,al                   ; Send EOI
   pop   ds								; restore ds & es registers to original state
   pop   es
   cmp   di, 1							; check if handling in protected mode
   jne   protret						; if protected mode do appropriate return

rmret:
   popa                          ; restore other registers to original state
	iret									; real mode iret to RM 16:16

protret:
   popa                          ; restore other registers to original state
   iretd                         ; protected mode iret to PM 16:32

_TEXT16	ENDS

ENDIF

	END
