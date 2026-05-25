/****************************************************************************
*  Sample DLL to illustrate building and using DLLs with MSVC4.x
*
*  Copyright (c) 1996 Tenberry Software, Inc.
*  All Rights Reserved
*/

#include <windows.h>
#include <stdio.h>

HANDLE ghDLLInst = 0;			// Handle to the DLL's instance.

BOOL WINAPI DllMain (HANDLE hModule, DWORD dwFunction, LPVOID lpNot)
{
    ghDLLInst = hModule;

    switch (dwFunction)
	    {
        case DLL_PROCESS_ATTACH:
			break;
        case DLL_PROCESS_DETACH:
			break;
        default:
            break;
    	}
    return TRUE;
}

// returns the sum of two values
int UTLadd(int a1, int a2)
{
	printf( "Add %d and %d.\n", a1, a2 );
	return (a1+a2);
}

// returns the greater of two values
int UTLmax(int a1, int a2)
{
	printf( "Find maximum of %d and %d.\n", a1, a2 );
	return(a1 > a2 ? a1 : a2);
}

// returns the lesser of two values
int UTLmin(int a1, int a2)
{
	printf( "Find minimum of %d and %d.\n", a1, a2 );
	return(a1 > a2 ? a2 : a1);	
}
