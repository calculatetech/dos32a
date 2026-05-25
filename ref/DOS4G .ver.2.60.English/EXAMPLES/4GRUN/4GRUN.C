/*
*  Load and run another program under DOS/4G
*  Copyright (c) 1996 Tenberry Software, Inc.
*  All Rights Reserved
*/

// Standard header files
#include <stdio.h>
#include <dos.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

// Extender header files
#include <dos32lib.h>

// The program to be loaded and run. Must be a DOS/4G extended program
#ifdef MSVC40
#define PART_TWO "4GRUNM2.EXE"
#else
#define PART_TWO "4GRUN2.EXE"
#endif

main(int ac, char **av)
   {
   int moduleHandle;
   TSF32 tsf, tsf2;
   int argument_index;

   printf("\nThis program demonstrates how to load-and-run a DOS/4G extended\n");
   printf("program from another DOS/4G extended program using the D32 API\n\n");

   // Load a DOS extended program under the same extender
   printf("Loading module %s using D32LoadModule().\n",PART_TWO);

#ifdef MSVC40
   // Load process returns a handle that we need to unload later
   // The second argument is the command tail which under Watcom is
   // ignored except for parameters and under Microsoft VC++ 4.x no 
   // canonical path names will be generated.
   moduleHandle = D32LoadModule(PART_TWO,"4GRUNM2.EXE",&tsf);
#else
   moduleHandle = D32LoadModule(PART_TWO,"",&tsf);
#endif

   if (moduleHandle != 0)
      {
      printf("D32LoadModule(%s) succeeded.\n",PART_TWO);
      printf("Running %s using D32RunReturn()\n",PART_TWO);

      // Now run the loaded program and return here when it terminates
      D32RunReturn(&tsf);

#ifdef __HIGHC__
      // when using the Metaware compiler
      // stdin, stdout, and stderr get closed when
      // the 2nd program terminates so reopen them
      open("con", O_BINARY | O_RDWR);
      open("con", O_BINARY | O_RDWR);
      open("con", O_BINARY | O_RDWR);
#endif

      // Now unload the program, reclaiming memory as we do so
      if (D32UnloadModule((WORD)moduleHandle))
			printf("Module %s unloaded using D32UnloadModule().\n",PART_TWO);
      else
	 		printf("Unable to unload module %s.\n",PART_TWO);
      }
   else
      {
      // Failed to load it at all. Complain and exit for good
      printf("Unable to load %s.\n",PART_TWO);
      exit(1);
      }

   // Load the program again, then execute it without ever returning
   // When the loaded program exits, this program will be exited as well
   printf("Reloading (%s)...\n",PART_TWO);

#ifdef MSVC40
	// Under the MSVC4.x compilers the arguments you pass into D32LoadModule
   // wll be the exact arguments your spzwned program will recieve when
   // executed. Under Watcom the command line is
   moduleHandle = D32LoadModule(PART_TWO,"4GRUNM2.EXE",&tsf2);
#else
   moduleHandle = D32LoadModule(PART_TWO,"",&tsf2);
#endif

   if (moduleHandle != 0)
      {
      printf(" %s is loaded.\n",PART_TWO);
      printf("Running %s using D32Run()...\n",PART_TWO);
      D32Run(&tsf2);

      // We should never get here. In case we do, we will free the loaded
      // Program, but this is probably a fatal condition anyway
      printf("Unexpected return to %s\n",PART_TWO);

      // Make an effort to free, but we don't care if it fails.
      D32UnloadModule((WORD)moduleHandle);
      }
   else
      {
      // We couldn't load the program at all. Complain bitterly.
      printf("Unable to load %s a second time, even though we did it the first time\n"
			,PART_TWO);
      }
   }
