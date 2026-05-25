/***************************************************************************
 *
 *  exetype.h -- Finding the EXE type of a protected mode executable.
 *
 *  Copyright (c) 1995 Tenberry Software, Inc.
 *  All Rights Reserved
 *
 */

#define UNKNOWN_EXE 	0
#define EXP_EXE 		1
#define SEG_EXE 		2
#define LIN_EXE 		3
#define LX_EXE  		4
#define AOUT_EXE 		5
#define DX_EXE       	6  // NTPATCHed SEG_EXE
#define PE_EXE  		7
                     
int exe_type(int fd, long fpos);

