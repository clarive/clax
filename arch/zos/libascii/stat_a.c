/********************************************************************/
/*																	*/
/* Name		: 	stat_a.c 											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the			*/
/*			  	stat functions.    									*/
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
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "global_a.h"
 
#ifdef GEN_PRAGMA_EXPORT
#pragma export(__chmod_a)
#pragma export(__mkdir_a)
#pragma export(__mknod_a)
#pragma export(__stat_a)
#pragma export(__statvfs_a)
#endif
 
/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII front-end routines for STAT functions						*/
/*																	*/
/********************************************************************/
 
int __chmod_a(const char *path, mode_t mode)
{
	return chmod((const char *) __getEstring1_a(path), mode);
}
 
int __mkdir_a(const char *path, mode_t mode)
{
	return mkdir((const char *) __getEstring1_a(path), mode);
}
 
int __mknod_a(const char *path, mode_t mode, dev_t dev_identifier)
{
	return mknod((const char *)__getEstring1_a(path),
				 mode, dev_identifier);
}
 
int __stat_a(const char *path, struct stat *buf)
{
	return stat((const char *) __getEstring1_a(path), buf);
}
 
int __statvfs_a(const char *path, struct statvfs *fsinfo)
{
	int rc;
 
	rc = statvfs((const char *) __getEstring1_a(path), fsinfo);
	if (rc==0)
		__toascii_a(fsinfo->f_OEcbid, fsinfo->f_OEcbid);
	return rc;
}
