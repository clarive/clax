/********************************************************************/
/*																	*/
/* Name		: 	dynit_a.c  											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the			*/
/*			  	dynit.h functions.  								*/
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
 
#include <dynit.h>
#include "global_a.h"
 
#ifdef GEN_PRAGMA_EXPORT
#pragma export(__dynalloc_a)
#endif
 
/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII front-end routines for DYNIT functions						*/
/*																	*/
/********************************************************************/

int __dynalloc_a(__dyn_t *dyn_parms)
{
	int dynalloc_rc;

	__toebcdic_a(dyn_parms->__ddname,dyn_parms->__ddname);
	__toebcdic_a(dyn_parms->__dsname,dyn_parms->__dsname);
	dynalloc_rc= dynalloc(dyn_parms);
	return(dynalloc_rc);
}
