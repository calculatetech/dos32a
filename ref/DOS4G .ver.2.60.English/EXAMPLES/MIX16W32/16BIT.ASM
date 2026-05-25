
;  16bit assembly code file for the MIX16W32 DOS/4G example.
;  Prints a 16bit data string and then calls another 16bit function in
;  16bit2.asm which exits using 21/4C.
;  Copyright (c) 1996 Tenberry Software, Inc.
;  All Rights Reserved

.386p

stdout 	equ 1

cr	equ 0dh	; carriage return code
lf	equ 0ah	; line feed code

; 16 bit function contained in 16bit2.asm
EXTRN doexit:FAR

; 16 bit data segment
_myDATA segment byte public 'DATA' USE16

   PUBLIC msg
msg   db cr,lf
   db 'Hello World!',cr,lf,0	; null terminated for printf
msg_len  equ   $-msg

; selectors used to access the code segment and the data segment 
; from 32 bit code in method 2
   PUBLIC  _DATA_16_SELECTOR
_DATA_16_SELECTOR       DW      seg _myDATA 

   PUBLIC  _CODE_16_SELECTOR
_CODE_16_SELECTOR       DW      seg _myTEXT

_myDATA ends


; 16 bit code segment
_myTEXT segment word public 'CODE' USE16

	public func16
	ASSUME CS:_myTEXT, DS:nothing

func16 proc far
	mov	ax, SEG _myDATA
	mov	ds, ax
	ASSUME DS:_myDATA

	mov	ah, 40h
	mov	bx, stdout
	mov	cx, msg_len
	mov	dx, OFFSET _myDATA:msg
	int	21h						; print the string
	call doexit
func16	endp

_myTEXT ends

	end
