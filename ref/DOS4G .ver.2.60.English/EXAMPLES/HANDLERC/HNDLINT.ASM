;=======================================================================
;
;  HNDLINT.ASM - 
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
;                			 E X T E R N A L S
;***********************************************************************

extrn screenp:DWORD
extrn D32RealToLinear:NEAR
extrn com_port:WORD
extrn intcontinue:NEAR
extrn handler_data_ptr:DWORD

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

      align   4
flatds1 dw     0

PUBLIC flatds1

handle_pm_only     PROC C PUBLIC

      pushad
      push    ds
      push    es
      push    fs
      push    gs
      mov     ebp, esp
      sub     esp, 0
      mov     ax, cs:flatds1
      mov     ds, ax
      cld
   
      cmp     DWORD PTR handler_data_ptr, 0
      jne     L1
      mov     eax, screenp
      push    eax
      call    D32RealToLinear
      add     esp, 4
      mov     WORD PTR [eax], 5050H
      add     DWORD PTR screenp, 2
      jmp     L2
L1:   mov     eax, handler_data_ptr
      push    [eax]
      call    D32RealToLinear
      add     esp, 4
      mov     WORD PTR [eax], 5050H
      mov     eax, handler_data_ptr
      add     DWORD PTR [eax], 2
L2:   push    com_port
      call    intcontinue
      add     esp, 2
      pop     gs
      pop     fs
      pop     es
      pop     ds
      popad
      iretd
      
handle_pm_only ENDP

;----------------------------------------------------------------------
;
;
;
;----------------------------------------------------------------------

        align   4
flatds2 dw     0

PUBLIC flatds2

handle_pm31_only     PROC FAR PUBLIC
   
      push    ds
      push    ebp
      mov     ebp, esp
      sub     esp, 4
      mov     ax, cs:flatds2
      mov     ds, ax
      mov     eax, screenp
      push    eax
      call    D32RealToLinear
      add     esp, 4
      mov     WORD PTR [eax], 5050H
      
      add     DWORD PTR screenp, 2
      push	  com_port
      call    intcontinue
   
      mov     DWORD PTR [ebp-4], 0
      mov     eax, [ebp-4]
      mov     esp, ebp
      pop     ebp
	   pop     ds
	   retf

handle_pm31_only ENDP

;***********************************************************************
;                       M O D U L E   E N D
;***********************************************************************

;----------------------------------------------------------------------
;
;
;----------------------------------------------------------------------

	END
