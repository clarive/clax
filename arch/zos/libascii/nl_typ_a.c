/********************************************************************/
/*																	*/
/* Name		: 	nl_typ_a.c  											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997, 1998 .                */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the			*/
/*			  	nl_types functions (catopen and catgets).  			*/
/* 																	*/
/* Compile	:	GEN_PRAGMA_EXPORT - generate PRAGMA statements to	*/
/* Options						export these entry points from the	*/
/*								DLL									*/
/*																	*/
/* Notes	:	All the procedures are name "__xxxxxxxx_a" where	*/
/*				xxxxxxxx is the name of the standard C run-time		*/
/*				function name. Unless otherwise noted, all functions*/
/* 				take the same argument,produce the same output and	*/
/*				return the same values as the standard functions.	*/
/*																	*/
/********************************************************************/
 
#include <nl_types.h>
#include "global_a.h"
 
#ifdef GEN_PRAGMA_EXPORT
 #pragma export(__catopen_a)
 #pragma export(__catgets_a)
#endif
 
/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII front-end routines for nl_types functions					*/
/*																	*/
/********************************************************************/

nl_catd __catopen_a(const char *path, int oflags)
{
	return catopen((const char *) __getEstring1_a(path) , oflags);
}

char *__catgets_a(nl_catd catd, int set_id, int msg_id, const char *string)
{

	char *msgP;

	if (msgP = catgets(catd, set_id, msg_id, string))
		{
		__toascii_a(msgP, msgP);
		return(msgP);
		}
	else
		return((char *) string);
}
