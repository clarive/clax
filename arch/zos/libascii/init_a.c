/********************************************************************/
/*																	*/
/* Name		: 	init_a.c 											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997, 1998.                 */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Miscallaneous routines required by ASCII/EBCDIC		*/
/*				interface code. All these routines are internal		*/
/*				use only and thus the functions are not exported.	*/	
/*																	*/
/* Compile	:	None												*/
/* Options															*/
/*																	*/
/* Notes  	:	None												*/
/*																	*/ 
/********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include "global_a.h"

char version[20]="libascii V1.1.9";
pthread_key_t *keyptr = (pthread_key_t *) NULL;
pthread_key_t key;
char athdid[5] = "ATHD";

ATHD_t *  __initASCIIlib_a();
void __termASCIIlib_a(void *);

/*%PAGE																*/
/*********************************************************************
*
*	Name   	 :	getathdp
*	Function :	Returns pointer to current thread's ATHD thread
*				structure.
*
*********************************************************************/
struct ATHD *getathdp()
{
	int status;
	ATHD_t *athdptr;
	/*
	 * Call pthread_getspecific() to get the address of the current thread's
	 * ATHD structure.  If the current thread doesn't havee a ATHD structure
	 * then call __initASCIIlib_a() to build one.
	 */
	if (((status = pthread_getspecific(key, (void **) &athdptr)) == -1)  ||
		(athdptr == NULL) ){
		athdptr = __initASCIIlib_a();
	}
	return(athdptr);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name   	 :	__initASCIIlib_a
*	Function :	Main initialization for all ASCII library routines,
*
*********************************************************************/
ATHD_t * __initASCIIlib_a()
{
	ATHD_t *athdptr;
	int athdsz;
	/* Perform key create for process if necessary */
	if (keyptr == (pthread_key_t *) NULL) {
		keyptr = &key;
		pthread_key_create(keyptr,__termASCIIlib_a);
	}

	/* Assume the current thread doesn't have a valid athd data area. */
	athdsz = sizeof(ATHD_t);
	athdptr = (ATHD_t *) calloc(1,athdsz); 
	if ((pthread_setspecific(key, (void *) athdptr) == -1) &&
	    ( errno == EINVAL) ) {
		/*
		 * Pthread_setspecific failed because parm key is invalid.
		 * At this point I am not sure if this code will ever be
		 * needed.
		 */
		keyptr = &key;
		pthread_key_create(keyptr, __termASCIIlib_a);
		pthread_setspecific(key, (void *) athdptr);
	}		

	/* Initialize athd structure. */

	memcpy(athdptr->cthdeye,athdid,4); 
	athdptr->pid = getpid();
	athdptr->threadid = pthread_self();

	/* Initialize ASCII translation routines. */
	init_trans_a();	

	/* Initialize ebcdic path name used my many routines. */
	athdptr->epathname = malloc((size_t) _POSIX_PATH_MAX);

	/* Set flag indiating athd initialization completed.  */
	athdptr->initdone = 1;

	return(athdptr);
}
/*********************************************************************
*
*	Name   	 :	__termASCIIlib_a
*	Function :	Thread termination routine for ASCII library.
*
*********************************************************************/
void __termASCIIlib_a(void *inparm)
{
	ATHD_t *athdptr;
	/*
	 * If athd data area exists and initializatione completed then
	 * perform termination.
	 */
	athdptr = (ATHD_t *) inparm;
	if (athdptr != NULL) {
		if ( athdptr->initdone == 1) {
			athdptr->initdone = 0; /* just to be sure no recursive calls. */
			term_getenv(athdptr); /* call getenv thread termination.      */
			term_trans(athdptr);  /* call translation thread termination. */
			term_locale(athdptr); /* call locale thread termination       */
			free(athdptr->epathname);
		}
		free(athdptr);     /* free athd data area for current thread */
	}
    return;  /* for now just return */
}
 
/*%PAGE																*/
/*********************************************************************
*
*	Name   	 :	__panic_a
*	Function :	Routine called when unusual condition encountered  
*				for which there is no recovery.
*
*********************************************************************/
void __panic_a(char *reason)
{
	int	S_errno = errno;
	int	S_errno2 = __errno2();

	__cdump(reason);
	perror(reason);
}
