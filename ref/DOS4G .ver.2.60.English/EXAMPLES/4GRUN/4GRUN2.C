/*
*  This program is called from 4GRUN.C, and demonstrates a
*  loaded program running under DOS/4G.
*	Copyright (c) 1996 Tenberry Software, Inc.
*	All Rights Reserved
*/

// Standard header files(s)
#include <stdio.h>

// Really simple program
main(int ac, char **av)
{
   // Display program name and return
   printf("\n***\n*** %s \n***\n\n",av[0]);
}
