/********************************************************************/
/*																	*/
/* Name		: 	dirent_a.c 											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the			*/
/*			  	dirent functions.    								*/
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
 
#include <dirent.h>
#include "global_a.h"
 
#ifdef GEN_PRAGMA_EXPORT
#pragma export(__opendir_a)
#pragma export(__readdir_a)
#endif
 
/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII front-end routines for DIRENT functions					*/
/*																	*/
/********************************************************************/

DIR *__opendir_a(const char *path)
{
	return opendir((const char *) __getEstring1_a(path));
}

struct dirent *__readdir_a(DIR *dir)
{
	struct	dirent *oreaddir;

	oreaddir = readdir(dir);
	if (oreaddir != NULL)
		__toascii_a(oreaddir->d_name, oreaddir->d_name);
	return(oreaddir);
}
