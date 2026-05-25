; DLL startup code for Tenberry DOS/4G Extender
; Based on Watcom startup code

            name     dllstart
.386p
.387

extrn __InitRtns:NEAR
extrn __FiniRtns:NEAR
extrn _Extender:WORD
extrn _psp:WORD
extrn _Envseg:WORD
extrn __hook387:NEAR
extrn __unhook387:NEAR
extrn __8087cw:WORD
extrn __8087:BYTE

            assume      nothing
_TEXT       segment  use32 word public 'CODE'

__GDAptr   						dd 0 		; IGC and Intel Code Builder GDA address
__D16Infoseg               dw 0     ; DOS/4G kernel segment, loaded dynamicaly
__x386_zero_base_selector 	dw 0		; base 0 selector for X-32VM
__saved_DS  					dw	0		; save area for DS for interrupt routines
L27                        dw 0     ; used for __sys_(f)ini(t)_387_emulator
L28                        db 0     ; used for __sys_(f)ini(t)_387_emulator

	public	__GDAptr
	public	__D16Infoseg
	public	__x386_zero_base_selector
	
	public	__GETDS
__GETDS proc	near
   mov   ds,cs:__saved_DS  ; load saved DS value
	ret				; return
__GETDS endp

__exit  proc near
	public	"C",__exit
ConsoleName	db	"con",00h
ifdef __STACK__
	pop	eax			; get return code into eax
endif
	jmp	short	ok
	public	__do_exit_with_msg__

; input: ( char *msg, int rc )  always in registers
__do_exit_with_msg__:
	push	edx			; save return code
	push	eax			; save address of msg
	mov	edx,offset ConsoleName
	mov	ax,03d01h	; write-only access to screen
	int	021h
	mov	bx,ax			; get file handle
	pop	edx			; restore address of msg
	mov	esi,edx 		; get address of msg
	cld					; make sure direction forward
L4:	lodsb				; get char
	cmp	al,0			; end of string?
	jne	L4				; no
	mov	ecx,esi 		; calc length of string
	sub	ecx,edx 		; . . .
	dec	ecx			; . . .
	mov	ah,040h 		; write out the string
	int	021h			; . . .
	pop	eax			; restore return code
ok:
	push	eax			; save return code
	mov	eax,00H			; run all finalizers
	mov	edx,0FFH		; run all finalizers
	call	__FiniRtns		; call finializer routines
	pop	eax			; restore return code
	mov	ah,04cH 		; DOS call to exit with return code
	int	021h			; back to DOS
__exit  endp


; DLLstart is called automatically by extender, if available, upon load of
; the dll. Function __dll_initialize() is called second.

        public  __DLLstart_
        assume  cs:_TEXT
__DLLstart_ proc  near
IFDEF DEBUG
	; Print an ! to the screen to announce that DLL startup code called
   ; For DLL example only
   mov     ah, 2
   mov     dl, '!'
   int     21h
ENDIF

   ; for GETDS, must occur before run time init because getds calls are done
   ; during run time init
   mov  ax, ds
   mov  ds:__saved_DS,ax

	; for kernel data seg addressability. fs is set to the kernel data sel
   ; for the call of DLLstart by the linexe loader
   mov     ds:__D16Infoseg, fs

 	; for runtime library functions ie: malloc
   mov     ax, 06200h
   int     21h             ; get psp
   mov     ds:_psp, bx
   mov     es, bx
   mov     ax, word ptr es:02Ch   ; env seg in psp
   mov     ds:_Envseg, ax
   mov     _Extender, 1    ; needs to be set to allow InitRtns to work properly
   mov     eax,000000ffH
   push    ebp
   mov     ebp, 0          ; __sys_init_387_emulator requires
   call    __InitRtns
   pop     ebp

   mov  ax, 1
   ret
__DLLstart_ endp

; normal execution of the functions below when pulled in from the standard 
; math libraries results in the wrong address for __D16Infoseg being accessed
; ergo the replacement code below tailored to be DOS/4G extender specific

; Based on Watcom 10.5 library code
        public  __sys_init_387_emulator
        assume  cs:_TEXT
__sys_init_387_emulator proc near
   push    es
   push    ecx
   push    ebx
   push    edx
   finit
   push    eax
   fstcw   [esp]
   pop     eax
   cmp     ah,03H
   je      L1
   inc     ebp
L1:
	or      ebp,ebp
   je      L2
   call    L3
L2:
	finit
   fldcw   __8087cw
   fldz
   fldz
   fldz
   fldz
   pop     edx
   pop     ebx
   pop     ecx
   pop     es
   ret

L3:
	mov     byte ptr __8087,03H
   mov     byte ptr ds:L28,01H
   smsw    word ptr ds:L27
   and     word ptr ds:L27,0006H
   sub     esp,00000008H
   sidt    word ptr [esp]
   mov     ebx,+2H[esp]
   add     ebx,00000038H
   add     esp,00000008H
   mov     dx,ds:__D16Infoseg
   sub     eax,eax
   call    __hook387
   ret
__sys_init_387_emulator endp

        public  __sys_fini_387_emulator
        assume  cs:_TEXT
__sys_fini_387_emulator proc near
    cmp     byte ptr L28,01H
    je      L14
    ret
L14:
	 push    ecx
    push    ebx
    push    edx
    mov     dx,ds:__D16Infoseg
    sub     eax,eax
    call    __unhook387
    jmp     L19
L19:
	 mov     byte ptr ds:L28,00H
    pop     edx
    pop     ebx
    pop     ecx
    ret
__sys_fini_387_emulator endp

_TEXT       ends
end
