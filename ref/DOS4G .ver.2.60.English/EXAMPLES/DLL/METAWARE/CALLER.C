/****************************************************************************
*  DOS/4G Metaware DLL test program
*
*  Copyright (c) 1996 Tenberry Software, Inc.
*  All Rights Reserved
*/

// Standard include files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// D32 API header
#include <dos32lib.h>

#define PATH_MAX 259

#ifdef _USE_IMPLIB_
// prototype to clean up warnings
int UTLadd(int a1, int a2);
int UTLmax(int a1, int a2);
int UTLmin(int a1, int a2);
#endif

void main(int argc, char **argv)
{
	int retcode;
	int i, j;
/*
 * This program builds two ways.  /D_USE_IMPLIB_ causes it to use 
 * an implib to load the DLL in the usual manner, at program load-time.
 * If _USE_IMPLIB_ is not specified, then the dynamic loading and
 * calling of the DLL comes into play.
 */	 
	
#ifndef _USE_IMPLIB_	 // Use D32LoadModule() and D32QueryProcAddr()
	int procHandle;
	char DLLname[64];
	char DLLpathname[PATH_MAX+64];		
	char modName[9];
	char fName[64];

   TSF32 tsfregs;
   DWORD (*fp)();
#endif

	// Set up i and j with default values unless command line values specified
   if (argc<3) 
   	{
      i = 40;
      j = 41;
	   } 
   else 
   	{
      i = atoi(argv[1]);
      j = atoi(argv[2]);
   	}

#ifndef _USE_IMPLIB_	

	//
	// Load the DLL dynamically...
	//
	strcpy(DLLname,"MYDLL.DLL"); 
	procHandle = D32LoadModule(DLLname,"\x00\r" ,&tsfregs);

	if (procHandle) 
   	{
      // report the path and filename of the dll loaded
		D32QueryModuleFileName(procHandle,DLLpathname,PATH_MAX+64);
      printf("D32QueryModuleFileName: %s\n",DLLpathname);

		// report the dll's module name
      D32QueryModuleName(procHandle,modName,9);
      printf("D32QueryModuleName: %s\n",modName);
      
		// report the handle of the loaded dll module
      printf("D32QueryModuleHandle: %d\n",D32QueryModuleHandle(modName));
		}
	else
		printf("D32LoadModule: failed to load module %s\n",DLLname);		

	if (procHandle) // use the module via the handle returned by D32LoadModule()
		{
		strcpy(fName, "UTLadd");

		// Find the DLL entry point by name...
      // Note that the ordinals are ignored when a function name is passed in
	   D32QueryProcAddr((int)procHandle, (DWORD)0, (char*)fName, (DWORD*)&fp);
		if (fp) 
      	{
			printf("D32QueryProcAddr(%d,0,%s) returned %#X\n",
				procHandle,fName,fp);

	      // call the function UTLadd in dllmw.dll
			retcode = (*fp) (i,j);
			printf("%s(%d,%d), got back %d.\n", fName,i,j,retcode);	
			fflush(stdout);		
			} 
	   else 
   		printf("Couldn't find %s.%s\n",DLLname,fName);
	
		strcpy(fName, "UTLmax");
   	D32QueryProcAddr((int)procHandle, (DWORD)1, (char*)fName, (DWORD*)&fp);
		if (fp) 
   		{
			printf("D32QueryProcAddr(%d,0,%s) returned %#X\n",
				procHandle,fName,fp);

	      // call the function UTLmax in dllmw.dll
			retcode = (*fp) (i,j);
			printf("%s(%d,%d), got back %d.\n", fName,i,j,retcode);	
			} 
	   else 
   		printf("Couldn't find %s.%s\n",DLLname,fName);
	
		strcpy(fName, "UTLmin");
	   D32QueryProcAddr((int)procHandle, (DWORD)1, (char*)fName, (DWORD*)&fp);
		if (fp) 
   		{
			printf("D32QueryProcAddr(%d,0,%s) returned %#X\n",
				procHandle,fName,fp);

	      // call the function UTLmin in dllmw.dll
			retcode = (*fp) (i,j);
			printf("%s(%d,%d), got back %d.\n", fName,i,j,retcode);	
			} 
	   else 
   		printf("Couldn't find %s.%s\n",DLLname,fName);

		// unload the DLL and exit gracefully
		if (!D32UnloadModule(procHandle))
			printf("Couldn't unload %d.\n", procHandle);
		}
		
#else

	// Since DLL functions loaded implicitly just call away

   retcode = UTLadd (i,j);
   printf("UTLadd(%d,%d), got back %d.\n", i,j,retcode);
   retcode = UTLmax (i,j);
   printf("UTLmax(%d,%d), got back %d.\n", i,j,retcode);
   retcode = UTLmin (i,j);
   printf("UTLmin(%d,%d), got back %d.\n", i,j,retcode);

#endif 

	printf("%s: Exiting...",argv[0]);	
}
