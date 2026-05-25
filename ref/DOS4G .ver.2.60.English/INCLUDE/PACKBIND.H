/***************************************************************************
 *
 *  packbind.h -- package names and entry points
 *
 *  Copyright (c) 1996, Tenberry Software, Inc.
 *  All Rights Reserved
 *
 */

#ifndef PACKBIND_H
#define PACKBIND_H

/* Exportable locations of DOS4G packages */

/* Universal entry points */
#define INIT_ENTRY                      "INIT"
#define DEINIT_ENTRY                    "DEINIT"

/* Exported from int21.c */
#define INT21_PACKAGE_NAME              "D32_INT21"

/* Exported from int31.c */
#define INT31_PACKAGE_NAME              "D32_INT31"

/* Exported from vmm4g.c */
#define VMM4G_PACKAGE_NAME              "VMM4G"
#define VM_FILE_BIND                    "VMFABIND"
#define VM_FMA_REGISTER                 "VMFMA"
#define VM_FILE_IO                      "VMFILEIO"
#define VM_ALLOC_INST                   "VMALOCINST"
#define VM_EXIT                         "VEXIT"
#define VM_RELEASE_MEM                  "VRELEASE"
#define VM_RECOVER_MEM                  "VRECOVER"
#define VM_SUPER_FREE                   "VSUPERFREE"
#define VM_SUPER_RESIZE                 "VSUPERRESIZE"
#define VM_VIRT_PAGE_ITERATE            "VMVIRTPAGESTAT"
#define VM_PHYS_PAGE_ITERATE            "VMPHYSPAGESTAT"
#define VM_GLOBAL_FAMILY_ITERATE        "VMGLOBALFAMILYSTAT"
#define VM_INSTANCE_FAMILY_ITERATE      "VMINSTFAMILYSTAT"
#define VM_GLOBAL_PTAB_ITERATE          "VMGLOBALPTABSTAT"
#define VM_INSTANCE_PTAB_ITERATE        "VMINSTPYABSTAT"
#define VM_PROCID                       "VMPROCID"
#define VM_PROCNUM                      "VMPROCNUM"
#define VM_PAGE_STAT                    "VMPAGESTAT"
#define VM_FLUSH                        "VMFLUSH"
#define VM_MAP_LINEAR                   "VMMAPP"
#define VM_SET_LRU                      "VMMSETLRU"
#define VM_SET_QUOTA                    "VMMSETQUOTA"

/* Present only in WVMM */
#define VM_MEM_ALLOC                    "VMEMALLOC"
#define VM_MEM_FREE                     "VFREEMEM"

/* Exported by all loaders */
#define LOADER_LOAD_ENTRY               "LOADER"
#define LOADER_UNLOAD_ENTRY             "UNLOAD"
#define LOADER_FREEMAP_ENTRY            "FREEMAP"
#define LOADER_RELOCATE_ENTRY           "REL"
#define LOADER_UNRELOCATE_ENTRY         "UNREL"
#define LOADER_CANLOAD_ENTRY            "CANLOAD"
#define LOADER_SHARE                    "SHARED_LOAD"
#define LOADER_GETLOADTABLE_ENTRY       "GETLOADTABLE"
#define LOADER_GETLOADNAME_ENTRY        "GETLOADNAME"

/* Exported by some loaders */
#define LOADER_GETERRSTR_ENTRY          "GETERRSTR"

/* Exported from segexe.c */
#define SEGEXE_LOADER_PACKAGE_NAME      "SEGEXE_LOADER"
#define SEGEXE_LOADMODULE               "SEGEXE_LOADMODULE"
#define SEGEXE_GETPROCADDR              "SEGEXE_GETPROCADDR"
#define SEGEXE_FREEMODULE               "SEGEXE_FREEMODULE"
#define SEGEXE_GETMODULENAME            "SEGEXE_GETMODULENAME"
#define SEGEXE_GETMODULEFILENAME        "SEGEXE_GETMODULEFILENAME"
#define SEGEXE_GETMODULENAME        	 "SEGEXE_GETMODULENAME"
#define SEGEXE_GETMODULEUSAGE           "SEGEXE_GETMODULEUSAGE"
#define SEGEXE_GETMODULEHANDLE          "SEGEXE_GETMODULEHANDLE"

/* Exported from linexe.c */
#define LINEXE_LOADER_PACKAGE_NAME      "LINEXE_LOADER"
#define LINEXE_LOADMODULE               "LINEXE_LOADMODULE"
#define LINEXE_GETPROCADDR              "LINEXE_GETPROCADDR"
#define LINEXE_GETFLATREGS              "LINEXE_GETFLATREGS"
#define LINEXE_FREEMODULE               "LINEXE_FREEMODULE"
#define LINEXE_GETMODHANDLE             "LINEXE_GETMODHANDLE"
#define LINEXE_GETMODFILENAME           "LINEXE_GETMODFILENAME"
#define LINEXE_GETMODNAME               "LINEXE_GETMODNAME"
#define LINEXE_GETMODUSAGE              "LINEXE_GETMODUSAGE"

/* Exported from pe loader */
#define PELOAD_LOADER_PACKAGE_NAME      "PELOAD_LOADER"
#define PELOAD_LOADMODULE               "PELOAD_LOADMODULE"
#define PELOAD_GETPROCADDR              "PELOAD_GETPROCADDR"
#define PELOAD_GETFLATREGS              "PELOAD_GETFLATREGS"
#define PELOAD_FREEMODULE               "PELOAD_FREEMODULE"
#define PELOAD_GETMODHANDLE             "PELOAD_GETMODHANDLE"
#define PELOAD_GETMODFILENAME           "PELOAD_GETMODFILENAME"
#define PELOAD_GETMODNAME               "PELOAD_GETMODNAME"
#define PELOAD_GETMODUSAGE              "PELOAD_GETMODUSAGE"
#define PELOAD_GETCMDLINE               "PELOAD_GETCMDLINE"
#define PELOAD_QUERYSTACKANDHEAPSIZE    "PELOAD_QUERYSTACKANDHEAPSIZE"

/* Exported from rex.c */
#define REX_LOADER_PACKAGE_NAME         "REX_LOADER"

/* Exported from expload.c */
#define EXP_LOADER_PACKAGE_NAME         "EXP_LOADER"

/* Exported from dos4g.c */
#define DOS4G_KERNEL_PACKAGE_NAME       "D32_KERNEL"
#define DOS4G_DEBUG_HOOK_ENTRY          "D32DebugHook"
#define DOS4G_NULLP_ENTRY               "D32NullPtrCheck"
#define DOS4G_INTCHAIN_ENTRY            "D32ChainInterrupt"
#define DOS4G_SET_FAULT_PROC_ENTRY      "D32SetFaultProc"
#define DOS4G_SET_INT_PROC_ENTRY        "D32SetIntProc"
#define DOS4G_CANCEL_FAULT_PROC_ENTRY   "D32CancelFaultProc"
#define DOS4G_CANCEL_INT_PROC_ENTRY     "D32CancelIntProc"

#if 0
        /* These functions have been trimmed from the API */
        #define DOS4G_SET_COOKED_PASSDN_ENTRY   "D32SetCookedPassdn"
        #define DOS4G_SET_INT_DEFAULT_ENTRY     "D32SetIntDefault"
        #define DOS4G_GET_INT_STATE_ENTRY       "D32GetIntState"
#endif

#define DOS4G_INT21_HANDLER             "D32Int21HandlerDirect"
#define DOS4G_ANY_LOAD                  "D32_ANY_LOAD"
#define DOS4G_STEPDOWN                  "D32Stepdown"
#define DOS4G_CURTASK                   "curtask"
#define DOS4G_RESET_TSP_ENTRY           "D32ResetTSP"
#define DOS4G_PROGRAM_NAME              "Name"
#define DOS4G_COPYRIGHT                 "(C)"
#define DOS4G_TIMESTAMP                 "Date"
#define DOS4G_VERSION                   "Version"
#define DOS4G_GETENV                    "DOS4G_GETENV"
#define DOS4G_GET_APPPATH               "DOS4G_GET_APPPATH"
#define DOS4G_LOWAVAIL                  "D32LowAvail"
#define DOS4G_ARGV                      "argv"
#define DOS4G_ENVP                      "envp"
#define DOS4G_MAINLOADERVECT            "DOS4GMAINLOADERVECT"
#define DOS4G_MAINMODULEHAND            "DOS4GMAINMODULEHAND"

/* Exported from the DOS16 Kernel */
#define DOS16_PACKAGE_NAME              "DOS16PKG"
#define VCPI_ENTRY                      "cp_entry"
#define BDOS_4B_FLG                     "bdos_4B_flg"
#define I21_PROC                        "int21_call_16"
#define I21_REGS                        "int21_creg_16"
#define I31_PROC                        "int31_call_16"
#define I31_REGS                        "int31_creg_16"

/* Exported from 32-bit InstantD */
#define INSTANTD32_PACKAGE_NAME         "INSTANTD32"
#define INSTANTD32_CALL_DBG             "D32_CALL_DBG"

/* Exported from DVX Process Manager */
#define DVX_PROC_MGR_NAME               "DVX_Process_Manager"
#define DVX_SEGTABLE                    "DVX_SegTable"
#define DVX_BEGIN_CRIT                  "DVX_Begin_Crit"
#define DVX_END_CRIT                    "DVX_End_Crit"
#define DVX_GLOBAL_SEG_ALLOC            "DVX_Global_Seg_Alloc"
#define DVX_GLOBAL_SEG_FREE             "DVX_Global_Seg_Free"
#define DVX_GLOBAL_SEG_BROADCAST        "DVX_Global_Seg_Broadcast"
#define DVX_GLOBAL_SEG_ASSOCIATE        "DVX_Global_Seg_Associate"
#define DVX_GLOBAL_SET_SHARE            "DVX_Global_Set_Share"
#define DVX_GLOBAL_ALLOC_SEGLINEAR      "DVX_Global_Alloc_SegLinear"
#define DVX_GLOBAL_WD40                 "DVX_Global_Gumout"
#define DVX_LOADER_OPEN                 "LOADER_OPEN"
#define DVX_WHICH_LOADER                "DVX_Which_Loader"
#define DVX_DOS4GX_GETOPTION            "DVX_Dos4gx_Get_Option"
#define DVX_DELAYEDSHUTDOWN             "DELAYEDSHUTDOWN"
#define DVX_HOME_DIRECTORY              "DVX_HOME_DIR"


/* Environment variable that controls un-needed loader expungement info */
#define LOADER_ENV_NAME                 "DOS4GLOADER"

#endif
