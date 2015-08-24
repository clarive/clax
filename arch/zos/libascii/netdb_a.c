/********************************************************************/
/*																	*/
/* Name		: 	netdb_a.c  											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997, 1998.                 */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the			*/
/*			  	netdb functions for X/OPEN format calls				*/
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
/* Changes	:	mm/dd/yy who v.r.m description						*/
/*				01/27/97 pfk 1.1.6 added gethostbyaddr				*/
/*			    01/27/97 pfk 1.1.6 Converted char strings returned	*/
/*								   by gethostbyaddr/gethostbyname	*/
/*								   to ASCII before returning		*/
/*																	*/
/********************************************************************/

#define _XOPEN_SOURCE_EXTENDED 1
#include <netdb.h>
#include "global_a.h"
 
#ifdef GEN_PRAGMA_EXPORT
#pragma export(__gethostbyaddr_a)
#pragma export(__gethostbyname_a)
#pragma export(__getservbyname_a)
#endif

extern void Convert_hostent_to_ascii(struct hostent *);
 
/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII front-end routines for NETDB functions						*/
/*																	*/
/********************************************************************/

struct hostent *__gethostbyaddr_a(const void * address, size_t address_len, int domain)
{
	struct hostent*	my_hostent;
	my_hostent = gethostbyaddr(address, address_len, domain);
	if (my_hostent)
		Convert_hostent_to_ascii(my_hostent);
	return my_hostent;
}

struct hostent *__gethostbyname_a(const char *name)
{
	struct hostent*	my_hostent;
	my_hostent = gethostbyname((const char *) __getEstring1_a(name));
	if (my_hostent)
		Convert_hostent_to_ascii(my_hostent);
	return my_hostent;
}

struct servent *__getservbyname_a(const char * name, const char * proto)
{
	return getservbyname((const char *) __getEstring1_a(name),
						 (const char *) __getEstring2_a(proto));
}
