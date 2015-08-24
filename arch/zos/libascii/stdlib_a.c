/********************************************************************/
/*																	*/
/* Name		: 	stdlib_a.c											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the 			*/
/*			  	stdlib functions.    								*/
/* 																	*/
/* Compile	:	GEN_PRAGMA_EXPORT - generate PRAGMA statements to	*/
/* Options						export these entry points from the	*/
/*								DLL									*/
/* 				GEN_IEEE_FP   - compiles assuming all floating		*/
/* 								point input/output is in IEEE 		*/
/*								format; otherwise in standard OS390	*/
/*								floating point format.				*/
/*																	*/
/* Notes	:	1) All the procedures are name "__xxxxxxxx_a" where	*/
/*				xxxxxxxx is the name of the standard C run-time		*/
/*				function name. Unless otherwise noted, all functions*/
/* 				take the same argument,produce the same output and	*/
/*				return the same values as the standard functions.	*/
/*																	*/ 
/********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>
#include <grp.h>
#include <pwd.h>
#ifdef GEN_IEEE_FP
#include <ieee_md.h>
#endif
#include "global_a.h"

#ifdef GEN_PRAGMA_EXPORT
 #pragma export(__atof_a)
 #pragma export(__atoi_a)
 #pragma export(__atol_a)
 #pragma export(__ecvt_a)
 #pragma export(__fcvt_a)
 #pragma export(__gcvt_a)
 #pragma export(__getenv_a)
 #pragma export(__mbtowc_a)
 #pragma export(__mbstowcs_a)
 #pragma export(__mktemp_a)
 #pragma export(__ptsname_a)
 #pragma export(__putenv_a)
 #pragma export(__la_setenv_a)
 #pragma export(__setkey_a)
 #pragma export(__strtod_a)
 #pragma export(__strtol_a)
 #pragma export(__strtoul_a)
 #pragma export(__system_a)
#endif

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__atof_a 
*	Function :	ASCII front-end for atof
*
*********************************************************************/

double __atof_a(const char *nptr)
{
#ifdef GEN_IEEE_FP
	double_t	tmpIEEEdbl;

	tmpIEEEdbl = str2dbl((char *) __getEstring1_a(nptr), NULL);
	return (double) *((double*)&tmpIEEEdbl);
#else
	return atof((const char *) __getEstring1_a(nptr));
#endif
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__atoi 
*	Function :	ASCII front-end for atoi
*
*********************************************************************/
int __atoi_a(const char *nptr)
{
	return atoi((const char *) __getEstring1_a(nptr));
}

/*%PAGE																*/
/*********************************************************************
*
*  	Name     :	__atol_a
*	Function :	ASCII front-end for atol
*
*********************************************************************/
long int __atol_a(const char *nptr)
{
	return atol((const char *) __getEstring1_a(nptr));
}

/*%PAGE																*/
/*********************************************************************
*
*  	Name     :	__ecvt_a 
*	Function :	ASCII front-end for ecvt
*
*********************************************************************/
char *__ecvt_a(double x, int ndigit, int *decpt, int *sign)
{
	char *tmp_out;
#ifdef GEN_IEEE_FP
	double_t tmpIEEEdbl;
	memcpy(&tmpIEEEdbl,&x,sizeof(double));
	x = dbl2nat(tmpIEEEdbl);
#endif /* GEN_IEEE_FP */

	tmp_out = ecvt(x,ndigit, decpt,sign); /* call ecvt    */
	__toascii_a(tmp_out,tmp_out);        /* convert output to ascii */
	return ((char *)tmp_out);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__fcvt_a 
*	Function :	ASCII front-end for fcvt  
*
*********************************************************************/

char *__fcvt_a(double x, int ndigit, int *decpt, int *sign)
{
	char *tmp_out;
#ifdef GEN_IEEE_FP
	double_t tmpIEEEdbl;
	memcpy(&tmpIEEEdbl,&x,sizeof(double));
	x = dbl2nat(tmpIEEEdbl);
#endif /* GEN_IEEE_FP */

	tmp_out =  fcvt(x, ndigit, decpt, sign);
	__toascii_a(tmp_out,tmp_out);        /* convert output to ascii */
	return tmp_out;
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__gcvt_a 
*	Function :	ASCII front-end for gcvt  
*
*********************************************************************/

char *__gcvt_a(double x, int ndigit, char * buf)
{
	char *tmp_out;
#ifdef GEN_IEEE_FP
	double_t tmpIEEEdbl;
	memcpy(&tmpIEEEdbl,&x,sizeof(double));
	x = dbl2nat(tmpIEEEdbl);
#endif /* GEN_IEEE_FP */

	tmp_out =  gcvt(x, ndigit, buf);
	__toascii_a(tmp_out,tmp_out);        /* convert output to ascii */
	return tmp_out;
}

/*%PAGE																*/
/*********************************************************************
*
*	Name   	 :	__getenv_a   
*	Function :	ASCII front-end to getenv
*
*********************************************************************/
char *__getenv_a(const char *varname)
{
	ATHD_t *myathdp;
	char tmpvarname[80];
	char *tmpvarnamep;
	int varlen=80;
	char *tmpenvp;
	char *getreturnp;
	if ((varlen = 1+strlen(varname)) > 80)
		tmpvarnamep = malloc(varlen);
	else
		tmpvarnamep = &tmpvarname[0];
	__toebcdic_a(tmpvarnamep,varname); /* convert ascii to ebcdic */
	tmpenvp = getenv(tmpvarnamep);
	if (varlen > 80)
		free(tmpvarnamep);
	if (tmpenvp == NULL)
		return(tmpenvp);
	myathdp = athdp();  /* get pointer to athd thread structure */
	if (myathdp->getenvlen < (varlen = 1 + strlen(tmpenvp))) {
		if (myathdp->getenvp != NULL)
			free(myathdp->getenvp);
		myathdp->getenvp = malloc(varlen);
		myathdp->getenvlen = varlen;
	}
	__toascii_a(myathdp->getenvp,tmpenvp);
	return(myathdp->getenvp); /* Return address of return buffer  */
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__mbtowc_a    
*	Function :	ASCII front-end for mbtowc   
*
*********************************************************************/
int __mbtowc_a(wchar_t *pwc, const char *string, size_t n)
{
	mbtowc(pwc,(const char *) __getEstring1_a(string),n);    
}                                                       

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__mbstowcs_a  
*	Function :	ASCII front-end for mbstowcs
*
*********************************************************************/

/* mbstowcs          */                                          
int __mbstowcs_a(wchar_t *pwc, const char *string, size_t n)       
{                                                                
	mbstowcs(pwc,(const char *) __getEstring1_a(string),n);    
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__mktemp_a
*	Function :	ASCII front-end for mktemp
*
*********************************************************************/
char * __mktemp_a(char *template)
{
char *tmp_out;
	__toebcdic_a(template,template);  /* convert template to ebcdic */
	tmp_out = mktemp(template);       /* call mktemp                */
	__toascii_a(tmp_out,tmp_out);     /* convert output to ascii    */
	__toascii_a(template,template);   /* convert template back to ascii */
	return ((char *)tmp_out);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__ptsname_a
*	Function :	ASCII front-end for ptsname
*
*********************************************************************/
char  *__ptsname_a(int fildes)
{
	char	*p;

	p = ptsname (fildes);
	__toascii_a(p,p);
	return(p);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__putenv_a 
*	Function :	ASCII front-end for putenv
*
*********************************************************************/
int __putenv_a(const char *envvar)
{
	return putenv((const char *) __getEstring1_a(envvar));
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__la_setenv_a    
*	Function :	ASCII front-end for setenv
*
*********************************************************************/
int __la_setenv_a(char *var_name, char *new_value, int change_flag)
{
	return setenv((const char *) __getEstring1_a(var_name),
				 (const char *) __getEstring2_a(new_value),change_flag);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__setkey_a    
*	Function :	ASCII front-end for setkey
*
*********************************************************************/
void __setkey_a(const char *key)                             
{
	setkey((const char *) __getEstring1_a(key));
}                                                              

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__strtod_a 
*	Function :	ASCII front-end for strtod      
*
*********************************************************************/
double __strtod_a( const char *nptr, char **endptr)
{
	char		*tmp;
	char		*e;
	double		tmpdbl;
#ifdef GEN_IEEE_FP
	double_t	tmpIEEEdbl;

	tmp = __getEstring1_a(nptr);
	tmpIEEEdbl = str2dbl(tmp, &e);
	memcpy(&tmpdbl,&tmpIEEEdbl,sizeof(double));
#else
	tmp = __getEstring1_a(nptr);
	tmpdbl = strtod(tmp, &e);
#endif
	if (endptr != NULL) {
		if (e)
			*endptr = (char*) nptr + (e - tmp);
		else
			*endptr = (char*) nptr;
	}
	return tmpdbl;
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__strtol_a 
*	Function :	ASCII front-end for strtol
*
*********************************************************************/
long int __strtol_a( const char *nptr, char **endptr, int base )
{
	char	*tmp;
	long	l;
	char	*e;

	tmp = __getEstring1_a(nptr);
	l = strtol(tmp, &e, base);
	if ( endptr )
		*endptr = (char*) nptr + (e - tmp);
	return l;
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__strtoul_a 
*	Function :	ASCII front-end for strtoul
*
*********************************************************************/
unsigned long int __strtoul_a( const char *s, char **endptr, int base )
{
	char		*tmp;
	unsigned	long ul;
	char		*e;

	tmp = __getEstring1_a(s);
	ul = strtoul(tmp, &e, base);
	if ( endptr )
		*endptr = (char*) s + (e - tmp);
	return ul;
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__system_a 
*	Function :	ASCII front-end for system
*
*********************************************************************/
int __system_a( const char *s )
{
	if (s)
		return system(__getEstring1_a(s));
	else
		return system(NULL);
}

/*%PAGE																*/
/*********************************************************************
*
*	Start of routines that are not exported
*
*********************************************************************/



/*********************************************************************
*
*	Name	 :	term_getenv
*	Function :	Thread termination routine for getenv() ascii.
*
*********************************************************************/
void term_getenv(ATHD_t *athdptr)
{
	if (athdptr->getenvp != NULL)
		free(athdptr->getenvp);
	return;
}
