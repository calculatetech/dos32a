
;  Second 16bit assembly code file for the MIX16W32 DOS/4G example.
;  Called from 16bit.asm. Just exits through 21/4C.
;  Copyright (c) 1996 Tenberry Software, Inc.
;  All Rights Reserved

.386p

; 16 bit codea segment
_myTEXT2 segment word public 'CODE' USE16

	public doexit
	ASSUME CS:_myTEXT2, DS:nothing

doexit proc far
	mov	ax, 4c00h
	int	21h		; exit program
doexit endp

_myTEXT2 ends

	end
