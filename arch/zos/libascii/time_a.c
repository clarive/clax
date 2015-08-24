/********************************************************************/
/*																	*/
/* Name		: 	time_a.c											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the 			*/
/*			  	time functions.     								*/
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
#include <time.h>
#include "global_a.h"

#ifdef GEN_PRAGMA_EXPORT
#pragma export(__asctime_a)
#pragma export(__ctime_a)
#endif

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__asctime_a
*	Function :	ASCII front-end for asctime
*
*********************************************************************/
char *__asctime_a(const struct tm *timeptr)
{
	char	*tmp_out;

	tmp_out = asctime(timeptr);         /* call asctime           	*/ 
	__toascii_a(tmp_out,tmp_out);       /* convert output to ascii  */
	return ((char *)tmp_out);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__ctime_a
*	Function :	ASCII front-end for ctime
*
*********************************************************************/
char *__ctime_a(const time_t *timer)
{
	char	*tmp_out;

	tmp_out = ctime(timer);             /* call ctime				*/
	__toascii_a(tmp_out,tmp_out);       /* convert output to ascii	*/
	return ((char *)tmp_out);
}
