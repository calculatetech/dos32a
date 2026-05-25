;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; sigrma.asm -- Support assembly function for sigrm.c to catch return from
;               TSR demo
;
; Copyright (c) 1996 Tenberry Software, Inc.
; All Rights Reserved
;

   .386

   name  sigrma

   ; externals in sigrm.c
   extrn notify_and_exit_:far
   extrn _sv_sp:word
   extrn __psp:word

INT7B_TEXT   segment byte USE16 public 'CODE'

   assume   cs:TSR_TEXT

   public   retfunc_
   ; assembly function to switch stacks and go to clean-up function
retfunc_:
   mov      ax,  seg _sv_sp
   mov      ds,  ax
   mov      ss,  ax
   mov      sp,  [_sv_sp]
   popa
   call     notify_and_exit_

INT7B_TEXT ends
   end

