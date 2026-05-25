/***************************************************************************
 *
 *  4gstub.c -- generic DOS4G application stub
 *
 *  Copyright (c) 1996 Tenberry Software, Incorporated
 *  All Rights Reserved
 *
 * To build using Watcom C/C++:
 *    wcl /lr stub.c
 *
 * To build using Microsoft C/C++ for use with Watcom LE or LX executables
 *    cl /AS /Oas /Gs /Festub.exe stub.c
 *
 * To build using Microsoft C/C++ v8.0 for PE Executables
 *    cl /c /AL /Os /Gs /I. /Fopestub.obj stub.c
 *    link /KNOWEAS pestub,,,;
 *
 * To build using Metaware:
 *    Not Possible.
 */  

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <share.h>
#include <process.h>
#include <errno.h>
#include <dos.h>

#include "exetype.h"

// Add environment strings to be searched here
char *paths_to_check[] = {
    "PATH"};

/***************************************************************
* Code to do an exec with a command tail instead of argv[]     *
* uses MSC internal _doexec() to do work                 		*
*	or																				*
* uses Watcom execl() to do work                 					*
****************************************************************/
static int doexec (const char *name, const char *command, 
						 const char *envblock, int elen)
{
   unsigned int xh[15];
   unsigned long filesize;
   int flag = 1;  // 0 = .exe, 1 = .com
   int fd;

   if ((fd = sopen(name, O_RDONLY|O_BINARY, SH_DENYWR)) == -1)
      return -1;  // check to see if file exists

   if (read(fd, (char *) xh, 0x18) == -1)
     {
     close(fd);
     errno = 8;       // ENOEXEC
     _doserrno = 11;  // E_badfmt
     return(-1);
     }
   filesize = (lseek(fd, 0L, 2)+15) >> 4;
   close(fd);

   if (xh[0] == 0x4d5a || xh[0] == 0x5a4d)
      flag--;

#ifndef __WATCOMC__
   _doexec(flag, name, strlen(name) + 1, command, envblock, elen,
      (xh[2] << 5) + xh[5] - xh[4], xh[7], xh[8], xh[11], xh[10],
      (unsigned int)filesize);
#else
   execl(name,name,command,NULL);
#endif

   return -1; // if doexec returns, it has failed
}

#ifndef __WATCOMC__
char far *curenv()
{
   _asm 
   	{
      mov ax, _psp
      mov es, ax
      mov dx, es:[2ch]
      xor ax, ax
      }
}

char far *get_commandtail()
{
   _asm 
   	{
      mov dx, _psp
      mov ax, 80h
   	}
}

#else

void curenvasm();
#pragma aux curenvasm = \
   "mov ax, _psp" \
   "mov es, ax" \
   "mov dx, es:[2ch]" \
   "xor ax, ax";

char far *curenv()
{
   curenvasm();
}

void gctasm();
#pragma aux gctasm = \
   "mov dx, _psp" \
   "mov ax, 80h";

char far *get_commandtail()
{
   gctasm();
}
#endif

int envlen(char far *env)
{
   char far *p=env;
   while( *p || *(p+1) ) p++;
   return( (p-env)+2 );
}

myexec(char *command, char *commandtail)
{
   char far *env = curenv();
   char *newenv;
   int i;

#ifndef __WATCOMC__
   int elen = envlen(env);
#else
   int elen = 0;
   char far *env2 = env;
   while( (env2[elen] != '\0') && (env2[elen+1] != '\0') )
      elen++;
#endif

   // Room for environment, command name, count word, trailing null
   newenv = malloc(elen + strlen(command)+ 3);
   for(i = 0; i < elen; i++)
      newenv[i] = env[i];
   newenv[i++] = '\1';
   newenv[i++] = '\0';
   strcpy(&newenv[i], command);
   elen += strlen(command) + 3;
   return(doexec(command, commandtail, newenv, elen));
}

/***************************************************************
* Code for locating a standalone DOS extender                  *
****************************************************************/
char *dos4g_path()
{
   static char fullpath[256];
   int i;

   // If 4GPATH points to an executable file name, don't bother
   // searching any other paths for DOS4G.EXE.
   strcpy(fullpath,getenv("4GPATH"));
   if(fullpath)
     {
	  static char *extender = "\\dos4g.exe";
	  char lastchar;

	  if ((lastchar = fullpath[strlen(fullpath)-1]) == '\\' ||
	  			lastchar == ':')
		// don't add backslash if 4GPATH ends with backslash or colon
	  	extender++;  
     strcat(fullpath, extender);
     i = open(fullpath, O_RDONLY);
     if(i >= 0)
        {
        close(i);
        return(fullpath);
        }
     }

   // Then check local directory and then PATH(s) for dos4g.exe
   for (i = 0; i < sizeof(paths_to_check)/sizeof(paths_to_check[0]); i++)
     {
     _searchenv("dos4g.exe",paths_to_check[i],fullpath);
     if(fullpath[0])
        return(fullpath);
     }

   // Fail if dos4g.exe not found
   return((char *)0);
}

/***************************************************************
* Code to add program name to command tail                     *
****************************************************************/
int mangle_commandtail(char *argv0, char *commandtail)
{
   int len, newlen;
   char far *get_commandtail();
   char far *ct = get_commandtail();
   char *newct = commandtail;

   // Capture length of original command tail, move to text portion
   newlen = len = *ct++;
   // Operate on text portion of new command tail
   newct++;
   // If program bound to stub, need to place program-name in arglist
   if(!strstr(argv0, "STUB.EXE") && !strstr(argv0, "stub.exe"))
	   {
      // Allow for old tail + additional space + program name
      newlen = len + strlen(argv0) + 1;
      if(newlen > 127)
   	   {
         fputs("Command line too long.\n", stderr);
         return(1);
      	}
      *newct++ = ' ';
      while(*argv0) *newct++ = *argv0++;
	   }
   // Copy characters from old tail
   while(len-- >= 0)  
   	*newct++ = *ct++;
   *commandtail = newlen;
   return(0);
}

main(int argc, char **argv)
{
   char *extender;
   char commandtail[128];

   if ( !(extender = dos4g_path()) )
      {
	   fputs("Stub failed to find DOS/4G extender.\n", stderr);
   	exit(2);
      }

   if ( mangle_commandtail(argv[0], commandtail) ) 
   	goto failure;

#ifndef __WATCOMC__
   myexec(extender, commandtail);
#else
	myexec(extender, argv[0]);
#endif

failure:
   fputs("Failed to load DOS/4G extender: ", stderr);
   fputs(extender, stderr);
   fputs("\n", stderr);

	exit( 1 );						// indicate error
}
