/*
*  Sample DLL to illustrate building and using Metaware DLLs with DOS/4G
*
*  Copyright (c) 1996 Tenberry Software, Inc.
*  All Rights Reserved
*/

// DLL Initialization routine
unsigned long _DLL_InitTerm(unsigned long modhandle, unsigned long flag)
{
	if ((flag == 0) || (flag == 1))
		return(1);
	else
		return(0);
}

// return sum of two paramaters
int UTLadd(int a1, int a2)
{ 
	return (a1+a2);
}

// return greater of two paramaters
int UTLmax(int a1, int a2)
{
	return(a1 > a2 ? a1 : a2);
}

// return lesser of two paramaters
int UTLmin(int a1, int a2)
{
	return(a1 > a2 ? a2 : a1);	
}
