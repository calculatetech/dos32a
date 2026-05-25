/*
 * usererr.c -- Sample package for remapping DOS/4G error messages.
 * This should be built as a 16-bit EXP and spliced into DOS/4G
 * with the 4GBIND utility.  We recommend compiling large model.
 *
 * Copyright (c) 1993, 1996 Tenberry Software, Incorporated
 * All Rights Reserved
 */

#define	_MT									/* to get _far va_list */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#undef	_MT

#define RSIAPI				_cdecl _loadds _far
#define CCOMMONAPI		_cdecl _loadds _far
#define ARRAY_LENGTH(x)	(sizeof(x)/sizeof(x[0]))

#include <package.h>

typedef enum
	{
	VerbMsg  = -1,								/* Printed only when VERBOSE set */
	Msg      =  0,								/* Normal informational message */
	WarnMsg  =  1,								/* Warning */
	ErrMsg   =  2,								/* Serious error */
	FatalMsg =  3								/* Fatal error */
	} MessageType;



/* This local sprintf formats a buffer using a vsprintf imported
	from DOS/4G.  It returns 0 if vsprintf can't be found, 1 otherwise.
*/
static int _near _cdecl lsprintf (char _far * buffer, char _far * format, ...)
	{
	int (CCOMMONAPI * vsprintf) (char _far * buffer, char _far * format, va_list varargs);
	PACKAGE _far * usererr;
	va_list varargs;

	if ((! (usererr = D16FindPackage ("ERROUT"))) ||
		(! (vsprintf = (void (*)()) D16FindAction (usererr, "VSPRINTF"))))
		return (0);
	va_start (varargs, format);
	vsprintf (buffer, format, varargs);
	va_end (varargs);
	return (1);
	}

/*	This function will be called before almost every message in
	DOS/4G is printed.  (Exceptions are messages printed by the
	real mode stub and those printed by the DOS/16M kernel
	on which DOS/4G is built.)

	Arguments passed to err_translate by DOS/4G are as follows:

	char _far *err_buf
		A pointer to a static buffer that already contains the entire
		formatted message.  Return the modified message in this buffer.
		Return 0 with the buffer unchanged for default handling.
	int buf_len
		The length of the static buffer.  Don't exceed it when writing
		back the modified message.
	char _far *program
		A pointer to the DOS extender name, for instance "DOS/4G".
	char _far *component
		A pointer to the name of the DOS extender component which issued
		the message, for example "LINEXE" for the linear executable
		loader.
	MessageType type
		The severity of the message -- see MessageType above.
	int number
		The error message number.  Will always be zero for Msg and
		VerbMsg; may be zero for other message types to indicate
		a continuation of the previous message.  If a message has no
		number, you can still recognize it by looking at the format
		or err_buf arguments.
	char _far *format
		DOS/4G's default format string for the message, which specifies
		how the other arguments have been passed on the stack.  Note
		that %s and %p specifiers are always _far.  For example, the
		format for error 2001 is "exception %02Xh (%s) at %X:%08lX\n".
	va_list varargs
		Optional additional arguments specified by format string.
		In the case of error 2001, the extra arguments are a 16-bit
		integer (interpreted as an exception ID), a 32-bit far pointer
		(to a string identifying the exception), a 16-bit integer
		(the selector where the exception occurred) and a 32-bit
		long integer (the offset where the exception occurred).
*/
static int CCOMMONAPI err_translate (char _far * err_buf, int buf_len,
	char _far * program, char _far * component,
	MessageType type, int number, char _far * format, va_list varargs)
	{
	/* This example performs the following translations:

		Messages of type VerbMsg are left alone, because they're
		intended for the (English-speaking) DOS/4G developers
		only.  The end user should not see these messages.

		Messages from the linear executable loader ("LINEXE") are
		appended to a text file called usererr.out, and not
		written to the screen.

		Message 2001 is replaced by different text.

		All other messages are converted to upper case -- mainly
		to remind us that we've bound this example package into
		DOS/4G, since the copyright is one of those messages.
	*/
	if (type == VerbMsg)
		return (0);

	if (! strcmp (component, "LINEXE"))
		{
		FILE *fp;

		/* Open USERERR.OUT for write/append, log the message, and
			reclose the file.  You would presumably want to leave
			the file open until your program exits, and call fflush()
			after each message, instead of reopening and reclosing
			the file.
		*/
		if (fp = fopen ("usererr.out", "w+"))
			{
			fputs (err_buf, fp);
			fclose (fp);
			return (1);							/* Nonzero = message was printed */
			}
		else
			return (0);
		}

	if (number == 2001)
		{
		unsigned short id;
		char _far *desc;
		unsigned short sel;
		unsigned long off;

		/* Peel off arguments from the stack according to the default
			format string, "exception %02Xh (%s) at %X:%08lX\n".
			Note that we don't have to call va_start or va_end
			because we've already been handed a va_list.
		*/
		id = (unsigned short) va_arg (varargs, unsigned short);
		desc = (char _far *) va_arg (varargs, char _far *);
		sel = (unsigned short) va_arg (varargs, unsigned short);
		off = (unsigned long) va_arg (varargs, unsigned long);

		/* Reformat the message.  Note that we are changing the order
			of the arguments as well as the text.

			The \a is the C escape for a bell character.
		*/
		lsprintf (err_buf, "\aException trapped at %x:%lx: %s\n",
			sel, off, desc);
		}
	else
		strupr (err_buf);						/* upper-case the formatted message */

	/* Return zero to tell ERROUT to print the message.  If we return
		nonzero, ERROUT won't print anything.  This allows a USERERR
		package to define its own output stream.
	*/
	return (0);
	}


/*	USERERR is the package name (don't change this; it has to be named 
	USERERR). It exports the package entry point ERRXLAT (don't change
	this either), which is the external name for the err_translate
	function.
*/
static ACTION_PACK user_actions[] =
	{
		{ "ERRXLAT", (ACTION *) err_translate }
	};

static PACKAGE usererr_package =
	{
	0,												/* Next package */
	"USERERR",									/* Package title */
	1, 0,											/* Package revision */
	0,												/* System action count */
	(ACTION_PACK _far *) 0,					/* System actions */
	ARRAY_LENGTH (user_actions),			/* User action count */
	(ACTION_PACK _far *) user_actions	/* User action pointer */
	};

void main (void)
	{
	if (D16FindPackage (usererr_package.package_title))
		return;									/* already installed */
	D16PackageRegister (&usererr_package);
	D16PackageTsr (1);
	}
