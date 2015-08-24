/********************************************************************/
/*																	*/
/* Name		: 	utsname_a.c											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the 			*/
/*			  	sys/utsname.h functions.							*/
/* 																	*/
/* Compile	:	GEN_PRAGMA_EXPORT - generate PRAGMA statements to	*/
/* Options						export these entry points from the	*/
/*								DLL									*/
/*																	*/
/* Notes	:	1) All the procedures are name "__xxxxxxxx_a" where	*/
/*				xxxxxxxx is the name of the standard C run-time		*/
/*				function name. Unless otherwise noted, all functions*/
/* 				take the same argument,produce the same output and	*/
/*				return the same values as the standard functions.	*/
/*																	*/ 
/********************************************************************/
#include <sys/utsname.h>
#include "global_a.h"

#ifdef GEN_PRAGMA_EXPORT
#pragma export(__uname_a)
#endif

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__uname_a
*	Function :	ASCII front-end for uname
*
*********************************************************************/
int __uname_a(struct utsname *name)
{
	int	rc;

	rc = uname(name);
	if (rc) {
		__toascii_a(name->sysname,name->sysname);
		__toascii_a(name->nodename,name->nodename);
		__toascii_a(name->release,name->release);
		__toascii_a(name->version,name->version);
		__toascii_a(name->machine,name->machine);
	}
	return rc;
}
