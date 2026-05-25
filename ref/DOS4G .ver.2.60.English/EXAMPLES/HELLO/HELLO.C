/*
*  Standard Hello World program 
*  Copyright (c) 1996 Tenberry Software, Inc.
*  All Rights Reserved
*/

// Standard header files
#include <stdio.h>

// Test DOS4GOPTIONS string export by preventing the startup banner from 
// being displayed
char DOS4GOPTIONS[] =
	"dos4g=StartupBanner:OFF\n";
   
main(int ac, char **av)
{
	// Print the standard response to the screen
   printf("Hello, World!\n");
}
