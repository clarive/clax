/********************************************************************/
/*																	*/
/* Name		: 	grp_a.c											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the 			*/
/*			  	grp.h functions.    								*/
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
#include <grp.h>
#include "global_a.h"

#ifdef GEN_PRAGMA_EXPORT
#pragma export(__getgrnam_a)
#endif

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__getgrnam_a
*	Function :	ASCII front-end for getgrnam
*
*********************************************************************/
struct group *  __getgrnam_a(const char *name) 
{
	char 	**curr;
	struct	group *grp;

	grp = getgrnam((const char *) __getEstring1_a(name));
	if ((grp) != NULL) {
		__toascii_a(grp->gr_name,grp->gr_name);
		for (curr=grp->gr_mem; (*curr) != NULL; curr++)
			__toascii_a(*curr, *curr);
		return(grp);
	}
	else
		return(NULL);
}
