/*
 * To build using Watcom C/C++:
 *    wcl /lr pstub.c
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <process.h>
#include <errno.h>
#include <string.h>

/* Add environment strings to be searched here */
char *paths_to_check[] = {
	"PATH"};

char *dos4gwpro_path()
{
	static char fullpath[256];
	int i;
	
	// If 4GWPROPATH points to an executable file name, don't bother
	// searching any other paths for 4GWPRO.EXE.
   strcpy(fullpath,getenv("4GWPROPATH"));
   if (fullpath)
		{
	   strcat(fullpath,"\\4gwpro.exe");
      i = open(fullpath, O_RDONLY);
      if(i >= 0)
      	{
         close(i);
         return(fullpath);
        	}
		}

   // Then check local directory and then PATH(s) for 4gwpro.exe
	for( i=0; i < sizeof(paths_to_check) / sizeof(paths_to_check[0]); i++ ) 
   	{
		_searchenv( "4gwpro.exe", paths_to_check[i], fullpath );
		if( fullpath[0] ) 
      	return( &fullpath );
		}

   // Fail if dos4gw.exe not found
   return((char *)0);
}

main( int argc, char *argv[] )
{
	char *av[4];
	auto char cmdline[128];

   if ( !(av[0] = dos4gwpro_path()) )
      {
	   fputs("Stub failed to find DOS/4GW Professional extender.\n", stderr);
   	exit(2);
      }

   av[1] = argv[0];           // name of executable to run
   av[2] = getcmd( cmdline ); // command line
   av[3] = NULL;              // end of list

	execvp( av[0], av );

   fputs("Failed to load DOS/4GW Professional extender: ", stderr);
   fputs(av[0], stderr);
	puts( strerror( errno ) );

	exit( 1 );						// indicate error
}
