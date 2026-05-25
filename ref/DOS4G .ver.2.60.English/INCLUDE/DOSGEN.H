/***************************************************************************
 *
 *  dosgen.h -- Generic definitions for DOS/16M and DOS/4G
 *
 *  Copyright (c) 1996, Tenberry Software, Inc.
 *  All Rights Reserved
 */

#ifndef DOSGEN_INCLUDED
#define DOSGEN_INCLUDED

#include "rsitypes.h"

#ifdef KERNEL
#define NODPMI 1
#endif

#ifdef NODPMI
/* Kernel uses old constants for selectors */
#define gdt_sel            0x8   /* selector for gdt                    */
#define idt_sel            0x10  /* selector for idt                    */
#define kernel_sel         0x18  /* selector for run time kernel code   */
#define kernel_data_sel    0x20  /* selector for kernel data            */
#define psp_sel            0x28  /* selector for PSP                    */
#define env_sel            0x30  /* selector for Environment            */
#define mem0_sel           0x38  /* selector for physical memory 0      */
#define bios_data_sel      0x40  /* selector for bios data area at 40:0 */
#define gdti_sel           0x50  /* selector for gdt auxiliary info     */
#define debug_sel          0x60  /* scratch desc for debugging          */
#define debug_gdt          (debug_sel/8)
#define kernelx_sel        0x70  /* kernel code in extended memory      */
#define user_sel           0x80  /* first user selecter (user_gdt << 3) */
#define user_gdt           16    /* first user gdt index                */
#else
/* programs reference system selectors indirectly */
#define gdt_sel            __d16_selectors.gdt
#define idt_sel            __d16_selectors.idt
#define kernel_sel         __d16_selectors.kernel
#define kernel_data_sel    __d16_selectors.kerneld
#define psp_sel            __d16_selectors.psp
#define env_sel            __d16_selectors.env
#define mem0_sel           __d16_selectors.mem0
#define bios_data_sel      __d16_selectors.biosdata
#define gdti_sel           __d16_selectors.gdti
#define kernelx_sel        __d16_selectors.kernelx
#define user_sel           __d16_selectors.user
#define user_gdt           (user_sel >> 3)
#endif

#define acc_code  0x9A  /* present, priv 0, code, read, accessed  */
#define acc_cmask 0x83
#define acc_data  0x92  /* present, priv 0, data, write, accessed */
#define acc_dmask 0x83
#define acc_DPL   0x60  /* privilege level bits                   */
#define acc_LDT   0x82  /* Local Descriptor Table type            */
#define acc_INT   0x8E  /* interrupt gate                         */
#define acc_TSS   0x85  /* tss descriptor                         */
#define acc_write 0x02  /* writable data, readable code access    */
#define Present   0x80

#define is_code(a)      (((a) & 0x18) == 0x18)
#define is_data(a)      (((a) & 0x18) == 0x10)
#define is_system(a)    (((a) & 0x10) == 0)
#define is_present(a)   (((a) & Present) != 0)

#define SelMask         0xFFF8   /* lose ring and table bits */

#define MPreferExt   0  /* prefer, but not force, extended            */
#define MPreferLow   1  /* prefer, but not force, conventional memory */
#define MForceExt    2  /* force extended                             */
#define MForceLow    3  /* force conventional                         */
#define MTransparent 4  /* force conventional, allocate selectors for
                           transparent addressing                     */
#define MTransStack  5  /* transparent, but aligned for use as stack  */

#endif
