/********************************************************************/
/*																	*/
/* Name		: 	pwd_a.c												*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the 			*/
/*			  	pwd.h functions.    								*/
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
#include <pwd.h>
#include "global_a.h"

#ifdef GEN_PRAGMA_EXPORT
#pragma export(__getpwnam_a)
#pragma export(__getpwuid_a)
#endif

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__getpwname_a
*	Function :	ASCII front-end for getpwnam
*
*********************************************************************/
struct passwd *__getpwnam_a(const char *name) 
{
	struct passwd  * p;

	p = getpwnam((const char *) __getEstring1_a(name));
	if ((p) != NULL) {
		__toascii_a(p->pw_name,p->pw_name);
		__toascii_a(p->pw_dir,p->pw_dir);
		__toascii_a(p->pw_shell,p->pw_shell);
		return(p);
	}
	else
		return(NULL);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__getpwuid_a
*	Function :	ASCII front-end for getpwuid
*
*********************************************************************/
struct passwd *__getpwuid_a(uid_t uid) 
{
	struct passwd  * p;

	p = getpwuid(uid);
	if ((p) != NULL) {
		__toascii_a(p->pw_name,p->pw_name);
		__toascii_a(p->pw_dir,p->pw_dir);
		__toascii_a(p->pw_shell,p->pw_shell);
		return(p);
	}
	else
		return(NULL);
}
