IFDEF MSVC40

;=======================================================================
;
;  TSFEXECA.ASM
;
;	Copyright (c) Tenberry Software, Inc. 1996
;
;=======================================================================
;
;	DESCRIPTION:
;
;  Support assembly function for tsrdemo to switch contexts
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

   INCLUDE dos32lib.mac

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

tsf32_exec   PROC  C PUBLIC
                           
   push  ebp
   mov   ebp, esp
   ; Establish addressability to save existing registers
   push  es             ; 4 +
   push  edi            ; 4 +
   pushfd               ; 4 +
   les   edi, ss:[ebp+8]
   push  eax            ; 4 =
   mov   ax, es
   verw  ax
   jz short tsf32_save
   jmp short tsf32_load

; Can write to from_tsp, save the processor's context
tsf32_save:
   ; Save existing registers
   pop   (TSF32 PTR es:[edi]).ts32_eax
   pop   (TSF32 PTR es:[edi]).ts32_eflags
   pop   (TSF32 PTR es:[edi]).ts32_edi
   pop   (TSF32 PTR es:[edi]).ts32_es
   push  (TSF32 PTR es:[edi]).ts32_es
   push  (TSF32 PTR es:[edi]).ts32_edi
   push  (TSF32 PTR es:[edi]).ts32_eflags
   push  (TSF32 PTR es:[edi]).ts32_eax
   mov   (TSF32 PTR es:[edi]).ts32_ebx, ebx
   mov   (TSF32 PTR es:[edi]).ts32_ecx, ecx
   mov   (TSF32 PTR es:[edi]).ts32_edx, edx
   mov   (TSF32 PTR es:[edi]).ts32_esi, esi
   mov   (TSF32 PTR es:[edi]).ts32_ebp, ebp
   mov   (TSF32 PTR es:[edi]).ts32_ds, ds
   mov   (TSF32 PTR es:[edi]).ts32_fs, fs
   mov   (TSF32 PTR es:[edi]).ts32_gs, gs
   mov   (TSF32 PTR es:[edi]).ts32_ss, ss
   ; Create space for return IFRAME32
   sub   esp, 12
   mov   (TSF32 PTR es:[edi]).ts32_esp, esp
   add   esp, 12
   mov   (TSF32 PTR es:[edi]).ts32_cs, cs
   mov   (TSF32 PTR es:[edi]).ts32_eip, offset tsf32_exec_continue
   
tsf32_load:
   les   edi, ss:[ebp+16]
   mov   ax, es
   verw  ax
   jnz short tsf32_exec_continue
   ; move the easy ones first
   mov   eax, (TSF32 PTR es:[edi]).ts32_eax
   mov   ebx, (TSF32 PTR es:[edi]).ts32_ebx
   mov   ecx, (TSF32 PTR es:[edi]).ts32_ecx
   mov   edx, (TSF32 PTR es:[edi]).ts32_edx
   mov   esi, (TSF32 PTR es:[edi]).ts32_esi
   mov   ebp, (TSF32 PTR es:[edi]).ts32_ebp
   mov   ds, (TSF32 PTR es:[edi]).ts32_ds
   mov   fs, (TSF32 PTR es:[edi]).ts32_fs
   mov   gs, (TSF32 PTR es:[edi]).ts32_gs
   ; Establish new stack
   mov   ss, (TSF32 PTR es:[edi]).ts32_ss
   mov   esp, (TSF32 PTR es:[edi]).ts32_esp

   ; Get interrupts right under DPMI
   push  eax
   push  (TSF32 PTR es:[edi]).ts32_eflags
   pop   eax
   and   eax, 200H
   jnz   setit
   cli
   jmp short popax
setit:   sti
popax:   pop  eax

   ; and push the rest
   push  (TSF32 PTR es:[edi]).ts32_eflags    ; Establish IRETD frame
   push  (TSF32 PTR es:[edi]).ts32_cs
   push  (TSF32 PTR es:[edi]).ts32_cs
   push  (TSF32 PTR es:[edi]).ts32_eip
   push  (TSF32 PTR es:[edi]).ts32_es
   push  (TSF32 PTR es:[edi]).ts32_es
   mov   edi, (TSF32 PTR es:[edi]).ts32_edi  ; Lose addressability to TSF
   pop   es
   db    0CFh                    ; IRET ...

tsf32_exec_continue:
	pop   eax
   popfd
   pop   edi
   pop   es
   leave
   ret

tsf32_exec ENDP

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
; tsfexeca.asm -- Support assembly function for test7b.c to switch contexts
;
; Copyright (c) 1996 Tenberry Software, Inc.
; All Rights Reserved
;

   name  tsf_exec
   .386p

   INCLUDE dos32lib.mac
   .MODEL FLAT, C
   .CODE

   public _tsf32_exec
_tsf32_exec:
   push  ebp
   mov   ebp, esp
   ; Establish addressability to save existing registers
   push  es             ; 4 +
   push  edi            ; 4 +
   pushfd               ; 4 +
   les   edi, ss:[ebp+8]
   push  eax            ; 4 =
   mov   ax, es
   verw  ax
   jz short tsf32_save
   jmp short tsf32_load

; Can write to from_tsp, save the processor's context
tsf32_save:
   ; Save existing registers
   pop   es:[edi].ts32_eax
   pop   es:[edi].ts32_eflags
   pop   es:[edi].ts32_edi
   pop   WORD PTR es:[edi].ts32_es
   push  es:[edi].ts32_es
   push  es:[edi].ts32_edi
   push  es:[edi].ts32_eflags
   push  es:[edi].ts32_eax
   mov   es:[edi].ts32_ebx, ebx
   mov   es:[edi].ts32_ecx, ecx
   mov   es:[edi].ts32_edx, edx
   mov   es:[edi].ts32_esi, esi
   mov   es:[edi].ts32_ebp, ebp
   mov   WORD PTR es:[edi].ts32_ds, ds
   mov   WORD PTR es:[edi].ts32_fs, fs
   mov   WORD PTR es:[edi].ts32_gs, gs
   mov   WORD PTR es:[edi].ts32_ss, ss
   ; Create space for return IFRAME32
   sub   esp, 12
   mov   es:[edi].ts32_esp, esp
   add   esp, 12
   mov   WORD PTR es:[edi].ts32_cs, cs
   mov   es:[edi].ts32_eip, offset tsf32_exec_continue

tsf32_load:
   les   edi, ss:[ebp+16]
   mov   ax, es
   verw  ax
   jnz short tsf32_exec_continue
   ; move the easy ones first
   mov   eax, es:[edi].ts32_eax
   mov   ebx, es:[edi].ts32_ebx
   mov   ecx, es:[edi].ts32_ecx
   mov   edx, es:[edi].ts32_edx
   mov   esi, es:[edi].ts32_esi
   mov   ebp, es:[edi].ts32_ebp
   mov   WORD PTR ds, es:[edi].ts32_ds
   mov   WORD PTR fs, es:[edi].ts32_fs
   mov   WORD PTR gs, es:[edi].ts32_gs
   ; Establish new stack
   mov   WORD PTR ss, es:[edi].ts32_ss
   mov   esp, es:[edi].ts32_esp

   ; Get interrupts right under DPMI
   push  eax
   push  es:[edi].ts32_eflags
   pop   eax
   and   eax, 200H
   jnz   setit
   cli
   jmp short popax
setit:   sti
popax:   pop  eax

   ; and push the rest
   push  es:[edi].ts32_eflags    ; Establish IRETD frame
   push  WORD PTR es:[edi].ts32_cs
   push  WORD PTR es:[edi].ts32_cs
   push  es:[edi].ts32_eip
   push  WORD PTR es:[edi].ts32_es
   push  WORD PTR es:[edi].ts32_es
   mov   edi, es:[edi].ts32_edi  ; Lose addressability to TSF
   pop   es
   db    0CFh                    ; IRET ...

tsf32_exec_continue:
   pop   eax
   popfd
   pop   edi
   pop   es
   leave
   ret

ENDIF

	END
