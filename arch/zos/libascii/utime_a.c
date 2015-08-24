/********************************************************************/
/*																	*/
/* Name		: 	utime_a.h  											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end for the  		*/
/*			  	utime functions.     								*/
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
 
#include <utime.h>
#include "global_a.h"
 
#ifdef GEN_PRAGMA_EXPORT
#pragma export(__utime_a)
#pragma export(__utimes_a)
#endif
 
/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII front-end routines for UTIME functions						*/
/*																	*/
/********************************************************************/
 
int __utime_a(const char *path, const struct utimbuf *times)
{
	return utime((const char *) __getEstring1_a(path), times);
}
 
int __utimes_a(const char *path, const struct timeval *times)
{
	return utimes((const char *) __getEstring1_a(path), times);
}
