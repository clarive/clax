/********************************************************************/
/*																	*/
/* Name		: 	string_a.c											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the 			*/
/*			  	string/string funtions.								*/
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
#include <string.h>
#include <strings.h>
#include "global_a.h"

#ifdef GEN_PRAGMA_EXPORT
#pragma export(__strcasecmp_a)
#pragma export(__strncasecmp_a)
#pragma export(__strerror_a)
#endif

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__strncasecomp_a 
*	Function :	ASCII front-end for strcasecmp
*
*********************************************************************/
int __strcasecmp_a(const char *s1, const char *s2)
{
	return strcasecmp((const char *) __getEstring1_a(s1),
		    		 (const char *) __getEstring2_a(s2));
}

int __strncasecmp_a(const char *s1, const char *s2, size_t n)
{
	return strncasecmp((const char *) __getEstring1_a(s1),
	                  (const char *) __getEstring2_a(s2), n);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__strerror_a
*	Function :	ASCII front-end for strerror
*
*********************************************************************/
char *__strerror_a(int errnum)
{
	char	*p;

	p = strerror(errnum);
	__toascii_a(p,p);
	return(p);
}
