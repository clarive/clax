/********************************************************************/
/*																	*/
/* Name		: 	ipc_a.c												*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the 			*/
/*			  	sys/ipc functions.    								*/
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
#include <sys/ipc.h>
#include "global_a.h"

#ifdef GEN_PRAGMA_EXPORT
#pragma export(__ftok_a)
#endif

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__ftok_a 
*	Function :	ASCII front-end for ftok  
*
*********************************************************************/
key_t __ftok_a(const char *path, int id)
{
	return ftok((const char *) __getEstring1_a(path), id );
}
