/*
*  Mixed 16bit and 32bit code example program built using DOS/4G and Watcom
*  Copyright (c) 1996 Tenberry Software, Inc.
*  All Rights Reserved
*/

// standard header files
#include<stdio.h>

// D32 API header file
#include<dos32lib.h>

// 16 bit function
extern void far * func16();

// 16 bit data item - must declare as function pointer to get valid address
// of data items
extern void far * msg();

// selectors from the 16 bit assembly files
extern   unsigned short  _CODE_16_SELECTOR;
extern   unsigned short  _DATA_16_SELECTOR;

// pointer construction helper macro
#define FarPtr far *
#define makeptr(s, o)  ( (void FarPtr)((long)(s) << 16 | (unsigned)(o)) )

main(int ac, char **av)
{

#ifdef METHOD1

	// Method1 utilizing function declared entry points
	// The prefered method of accessing 16 bit code & data items.
   
	GDT32 gdt32;
	int sel;

	// print the 16 bit data from 32 bit code
	printf("Printing 16 bit string from 32 bit code\n%Fs",(char far *)msg);

	// get an alias selector to the 16 bit function
	sel = D32CreateSegAlias((unsigned int)func16,CODE_SEG,USE16,LINEAR);

	// print the 16 bit data from 16 bit code
	printf("\nPrinting 16 bit string from 16 bit code\n");

   // construct a 16:16 ptr to the 16 bit function and call it
	D32CallEntry16((FarPtr16)makeptr(sel,0));

#else

	// Method2 utilizing segment selectors
   // A bit more clunky but might be useful for some.

	DWORD code_ptr = 0;
	char far *data_ptr = 0;

	// construct pointers to the beginnings of the 16 bit code and data segments
	code_ptr = (DWORD)makeptr((unsigned int)_CODE_16_SELECTOR, 0);
	FP_SEG32(data_ptr) = _DATA_16_SELECTOR;

	// print the 16 bit data from 32 bit code
	printf("Printing 16 bit string from 32 bit code\n%Fs",data_ptr);

	// print the 16 bit data from 16 bit code
	printf("\nPrinting 16 bit string from 16 bit code\n");
	D32CallEntry16((FarPtr16)code_ptr);

#endif

}

