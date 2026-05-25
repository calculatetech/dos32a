/****************************************************************************
*  Sample DLL to illustrate building and using DLLs with DOS/4G and Watcom
*
*  Copyright (c) 1996 Tenberry Software, Inc.
*  All Rights Reserved
*/

#include <stdio.h>
#include <stdlib.h>

char *string = "FROM MYDLL";

// A for attach
void signal();
#pragma aux signal = \
   "mov      ah, 2" \
   "mov      dl, 'A'" \
   "int      21h";

// D for detach
void signalx();
#pragma aux signalx = \
   "mov      ah, 2" \
   "mov      dl, 'D'" \
   "int      21h";

// simple dll initializer function
int __export __dll_initialize( void )
{
   signal();
	return 1;	// success
}

// simple dll deinitializer function
int __export __dll_terminate( void )
{
   signalx();
	return 1;	// success
}

// returns the sum of two values
int _loadds __export   UTLadd(int a1, int a2)
{ 
	float x = 123.45678, y=910.1112131415;
   char *path;

	printf("HELLO %s\n",string);

	// Test runtime and float/double support
   printf("MALLOC(1MB) returns %lX\n",malloc(1*1024L*1024L));
   printf("FLOATX(%f) * FLOATY(%f) = %f\n",x,y,x*y);
   path = getenv("PATH");
   printf("PATH=%s\n",path);

	return (a1+a2);
}

// returns the greater of two values
int _loadds __export  UTLmax(int a1, int a2)
{
	return(a1 > a2 ? a1 : a2);
}

// returns the lesser of two values
int _loadds __export  UTLmin(int a1, int a2)
{
	return(a1 > a2 ? a2 : a1);	
}
