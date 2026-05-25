;=======================================================================
;
;  EXCEPT.ASM -
;
;	Copyright (c) Tenberry Software, Inc. 1996
;
;=======================================================================
;
;	DESCRIPTION: Exception 6 handler
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
;                			 E X T E R N A L S
;***********************************************************************

extrn longjmp			:NEAR
extrn env				:DWORD
extrn D32GetSegBase	:NEAR

;***********************************************************************
;                E Q U A T E S   A N D   M A C R O S
;***********************************************************************

TSF32		STRUCT		; Transfer stack frame layout.
   ts32_prev   WORD  ?  ; Previous TSF pointer.
   ts32_ss     WORD  ?
   ts32_ds     WORD  ?
   ts32_es     WORD  ?
   ts32_fs     WORD  ?
   ts32_gs     WORD  ?
   ts32_edi    DWORD ?
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

TSF32PTR		TYPEDEF		NEAR PTR TSF32
TSF32FPTR	TYPEDEF		FAR PTR TSF32

;***********************************************************************
;                             D A T A
;***********************************************************************

	.DATA

except_no 		dd  0

;***********************************************************************
;                             C O D E
;***********************************************************************

	.CODE

;----------------------------------------------------------------------
;
;  
;
;----------------------------------------------------------------------

flatds			dw  0

PUBLIC flatds

except_handler       PROC C PUBLIC

         push    ds
         push    ebp
         mov     ebp, esp
         mov     ds, cs:[flatds]
         cmp     DWORD PTR [except_no], 0
         jne     L1
         les     eax, FWORD PTR [ebp+14h]
         add     DWORD PTR es:[eax+32h], 2
         mov     DWORD PTR [except_no], 1
         jmp     L2

L1:      ; load ss with flat ss since longjmp doesn't reload ss
			push    ecx
         push    edx
			mov     eax, 6
         mov     bx, ss
         movzx   ebx, bx
         int     31h				; get seg base
         shl     ecx, 16
         mov     eax, ecx
         movzx   edx, dx
         add     eax, edx
         add     eax, esp
         pop     edx
         pop     ecx
         mov     ds, cs:[flatds]
         mov     dx, ds
         mov     es, dx			; set es to flat ds to prevent later failure due to assumed flat es
         mov     ss, dx
         mov     esp, ecx		; esp converted to flatss esp position
			push    OFFSET env
         push    DWORD PTR [except_no]
         jmp     longjmp

L2:      mov     eax,0
         mov     esp,ebp
         pop     ebp
         pop     ds
         retf

except_handler ENDP

;***********************************************************************
;                       M O D U L E   E N D
;***********************************************************************

;----------------------------------------------------------------------
;
;
;----------------------------------------------------------------------

	END

