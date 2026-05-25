IFDEF MSVC40

;=======================================================================
;
;  tsrdemoa.asm -- Support assembly function for TSR to handle real mode
;  					0x7B interrupts
;
;	Copyright (c) Tenberry Software, Inc. 1996
;
;=======================================================================
;
;	DESCRIPTION:
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
;              S T R U C T S  A N D  E X T E R N A L S
;***********************************************************************

FarPtr32    STRUCT
   off     DWORD ?
   sel     WORD  ?
FarPtr32    ENDS

TSF32		STRUCT		; Transfer stack frame layout.
   ts32_prev   WORD  ?  ; Previous TSF pointer.
   ts32_ss     WORD  ?
   ts32_ds     WORD  ?
   ts32_es     WORD  ? 
   ts32_fs     WORD  ?
   ts32_gs     WORD  ?
   ts32_edi 	DWORD ?
   ts32_esi    DWORD ?
   ts32_ebp    DWORD ?
   ts32_esp    DWORD ?
   ts32_ebx    DWORD ?
   ts32_edx    DWORD ?
   ts32_ecx    DWORD ?
   ts32_eax    DWORD ?
   ts32_id     WORD  ?  ; Interrupt number ID.
   ts32_code   DWORD ?  ; Place for exception type code.
   ts32_eip 	DWORD ?  ; IRETD frame.
   ts32_cs  	WORD  ?  ;      "
   ts32_proc   WORD  ?  ; [used as next-proc handle]
   ts32_eflags DWORD ?  ;      "
TSF32    ENDS

D32REGS STRUCT
	rds     DWORD  ?
	res     DWORD  ?
	redi    DWORD  ?
	resi    DWORD  ?
	rebp    DWORD  ?
	resp    DWORD  ?
	rebx    DWORD  ?
	redx    DWORD  ?
	recx    DWORD  ?
	reax    DWORD  ?
	reflags DWORD  ?
D32REGS ENDS

extrn crfptr            :FarPtr32
extrn tsr_context       :TSF32
extrn client_context    :TSF32
extrn tsf32_exec        :NEAR
extrn sv_esp            :DWORD
extrn client_iostate    :WORD
extrn tsr_iostate       :WORD
extrn D32TsrSwitchPSP   :NEAR
extrn lowp              :DWORD
extrn r                 :D32REGS
extrn INT31             :NEAR
extrn D32DosMemFree     :NEAR
extrn sel               :WORD
extrn printf            :NEAR
extrn terminate_tsr     :NEAR
extrn inner_handler		:NEAR

;***********************************************************************
;                             D A T A
;***********************************************************************

	.DATA

cbstring db 09h, 'TSR: Inside callback function', 0Ah, 0Dh

;***********************************************************************
;                             C O D E
;***********************************************************************

	.CODE

;----------------------------------------------------------------------
;
;  
;
;----------------------------------------------------------------------

callfar macro p
	public	p
	db 09ah			; OP for 16:16 far call
p	dd 0
	endm

PUBLIC passup

; Writen with db's because MASM6.11 will not generate the appropriate code
; to allow this procedure to run in real mode

rm_handler     PROC FAR PUBLIC

;  push  ax
	db 050h
;  mov   ax, ds
   db 08Ch
   db 0D8h
;  pushf
	db 09Ch
   callfar passup
   ; Return values will come in ax:dx, too
   ; and return in ds:dx
;  mov   ds, ax
   db 08Eh
   db 0D8h
;  pop   ax
   db 058h
;	iret
	iretd		; resolves to CF instead of 66 CF which iret generates

rm_handler ENDP

;----------------------------------------------------------------------
;
;
;----------------------------------------------------------------------
flatds	dw ?

PUBLIC flatds

outer_handler  PROC FAR PUBLIC

   push    ds
   mov     ax, cs:flatds
   mov     ds, ax
   push    ebx
   push    esi
   push    edi
   push    ebp
   mov     ebp, esp
   sub     esp, 4
   mov     eax, [ebp+24H]
   mov     crfptr.sel, ax
   mov     eax, [ebp+20H]
   mov     crfptr.off, eax
                                                           
   mov     DWORD PTR tsr_context+32H, OFFSET inner_handler
   mov     ax, ds
   mov     edx, OFFSET tsr_context
   and     eax, 0000ffffH
   push    eax
   push    edx
   mov     edx, OFFSET client_context
   push    eax
   push    edx
   call    tsf32_exec
   add     esp, 16

   mov     DWORD PTR [ebp-4H], 0
   mov     eax, [ebp-4H]
   mov     esp, ebp
   pop     ebp
   pop     edi
   pop     esi
   pop     ebx
   pop     ds
	retf

outer_handler	ENDP

callback_x  PROC FAR PUBLIC

   pushad
   push    ds
   push    es
   push    fs
   push    gs
   mov     ebp, esp
   sub     esp, 0
   cld
   mov     ax, cs:flatds
   mov     ds, ax
   mov     ebx, sv_esp
   mov     ax, ds
   mov     ss, ax
   mov     esp, ebx
   mov     eax, OFFSET client_iostate
   push    eax
   xor     eax, eax
   mov     ax, tsr_iostate
   push    eax
   call    D32TsrSwitchPSP
   add     esp, 8
                                     
	mov    eax, lowp
	xor    ecx, ecx
	mov    cx, WORD PTR [eax+1]
   mov    r.redx, ecx
  	mov    cx, WORD PTR [eax+3]
   mov    r.recx, ecx
	mov    eax, OFFSET r
	push   eax
   push   r.rebx
	push   304h
	call   INT31

   mov     eax, OFFSET cbstring
   push    eax
   call    printf
   add     esp, 4
   call    terminate_tsr
   pop     gs
   pop     fs
   pop     es
   pop     ds
   popad
   iretd
  
callback_x  ENDP
   
;***********************************************************************
;                       M O D U L E   E N D
;***********************************************************************

;----------------------------------------------------------------------
;
;
;----------------------------------------------------------------------

ELSE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; tsrdemoa.asm -- Support assembly function for TSR to handle real mode
;						0x7B interrupts
;
; Copyright (c) 1996 Tenberry Software, Inc.
; All Rights Reserved
;

	name		tsrdemoa
TSR_TEXT 	segment byte use16 public 'CODE'
   assume  	cs:TSR_TEXT

callfar macro p
	public	p
	db 09ah			; OP for 16:16 far call
p	dd 0
	endm
	
   public  _rm_handler
_rm_handler:                    ; real-mode interrupt catcher
	push	ax
	mov	ax, ds
   pushf
	callfar _passup
	; Return values will come in ax:dx, too
	; and return in ds:dx
	mov	ds, ax
	pop	ax
	iret
TSR_TEXT ends

   end

ENDIF

	END
