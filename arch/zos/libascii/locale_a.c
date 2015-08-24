/********************************************************************/
/*																	*/
/* Name		: 	locale_a.c											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the 			*/
/*			  	locale functions.    								*/
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
#include <locale.h>
#include "global_a.h"

#ifdef GEN_PRAGMA_EXPORT
 #pragma export(__localeconv_a)
 #pragma export(__setlocale_a)
#endif

/********************************************************************/
/*                                                                  */
/* Area to hold copy of lconv - used by localeconv                  */
/*                                                                  */
/********************************************************************/
struct elconv {

    /*                                                              */
    /* Warning field elconv_str must be first in this structure     */
    /*                                                              */
    struct lconv elconv_str; /* Local copy of lconv CB              */

    /* local copies of character strings returned by localeconv     */
    char elconv_decimal_point[8];
    char elconv_thousands_sep[8];
    char elconv_grouping[8];
    char elconv_int_curr_symbol[8];
    char elconv_currency_symbol[8];
    char elconv_mon_decimal_point[8];
    char elconv_mon_thousands_sep[8];
    char elconv_mon_grouping[8];
    char elconv_positive_sign[8];                                    
    char elconv_negative_sign[8];
    char elconv_left_parenthesis[8];                                 
    char elconv_right_parenthesis[8];
    char elconv_debit_sign[8];
    char elconv_credit_sign[8];

    } ;
 typedef struct elconv elconv_t ;

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__localeconv_a 
*	Function :	ASCII front-end for localeconv
*
*********************************************************************/
struct lconv *__localeconv_a(void)
{
	struct	lconv* 	syslconv;
	ATHD_t	*		myathdp;
	elconv_t *      myelconv;
	int				elconv_sz;
	myathdp = athdp();			/* get ptr to athd 					*/

	/* If 1st localeconv call for this thread						*/
	/* -- Get space for local copy of lconv area					*/
	/* -- Set ptrs in elconv_str to fields in elconv           		*/
	if (myathdp->elconv_a == 0) {
		elconv_sz = sizeof(elconv_t);
		myelconv = (elconv_t *) calloc(1,elconv_sz);
		myathdp->elconv_a =&(myelconv->elconv_str);
		myathdp->elconv_a->decimal_point = 
				(char *)&(myelconv->elconv_decimal_point);
		myathdp->elconv_a->thousands_sep =
				(char *)&(myelconv->elconv_thousands_sep);
		myathdp->elconv_a->grouping =
				(char *)&(myelconv->elconv_grouping);
		myathdp->elconv_a->int_curr_symbol =
				(char *)&(myelconv->elconv_int_curr_symbol);
		myathdp->elconv_a->currency_symbol =
				(char *)&(myelconv->elconv_currency_symbol);
		myathdp->elconv_a->mon_decimal_point	=
				(char *)&(myelconv->elconv_mon_decimal_point);
		myathdp->elconv_a->mon_thousands_sep	=
				(char *)&(myelconv->elconv_mon_thousands_sep);
		myathdp->elconv_a->mon_grouping =
				(char *)&(myelconv->elconv_mon_grouping); 
		myathdp->elconv_a->positive_sign =
				(char *)&(myelconv->elconv_positive_sign);
		myathdp->elconv_a->negative_sign =
				(char *)&(myelconv->elconv_negative_sign);
		myathdp->elconv_a->left_parenthesis =
				(char *)&(myelconv->elconv_left_parenthesis);
		myathdp->elconv_a->right_parenthesis =
				(char *)&(myelconv->elconv_right_parenthesis);
		myathdp->elconv_a->debit_sign =
				(char *)&(myelconv->elconv_debit_sign);
		myathdp->elconv_a->credit_sign =
				(char *)&(myelconv->elconv_credit_sign);
	}

	/* Invoke system localeconv and then copy lconv area returned	*/
	/* (which is in read-only memory), to local area so that the	*/
	/* translation from ebcidic to ascii can be done.				*/
	syslconv = localeconv();

	myathdp->elconv_a->int_frac_digits = syslconv->int_frac_digits;
	myathdp->elconv_a->frac_digits = syslconv->frac_digits;
	myathdp->elconv_a->p_cs_precedes = syslconv->p_cs_precedes;
	myathdp->elconv_a->p_sep_by_space = syslconv->p_sep_by_space;
	myathdp->elconv_a->n_cs_precedes = syslconv->n_cs_precedes;
	myathdp->elconv_a->n_sep_by_space = syslconv->n_sep_by_space;
	myathdp->elconv_a->p_sign_posn = syslconv->p_sign_posn;
	myathdp->elconv_a->n_sign_posn = syslconv->n_sign_posn;

	/* convert lconv ebcidic info to ascii and store in the local  */
	/* copy of the lconv in myathdp                                */
	__toascii_a((char *)myathdp->elconv_a->decimal_point,
				(char *)syslconv->decimal_point);
	__toascii_a((char *)myathdp->elconv_a->thousands_sep,
				(char *)syslconv->thousands_sep);
	__toascii_a((char *)myathdp->elconv_a->grouping,
				(char *)syslconv->grouping);
	__toascii_a((char *)myathdp->elconv_a->int_curr_symbol,
				(char *)syslconv->int_curr_symbol);
	__toascii_a((char *)myathdp->elconv_a->currency_symbol,
				(char *)syslconv->currency_symbol);
	__toascii_a((char *)myathdp->elconv_a->mon_decimal_point,
				(char *)syslconv->mon_decimal_point);
	__toascii_a((char *)myathdp->elconv_a->mon_thousands_sep,
				(char *)syslconv->mon_thousands_sep);
	__toascii_a((char *)myathdp->elconv_a->mon_grouping, 
				(char *)syslconv->mon_grouping);  
	__toascii_a((char *)myathdp->elconv_a->positive_sign,
				(char *)syslconv->positive_sign);
	__toascii_a((char *)myathdp->elconv_a->negative_sign,
				(char *)syslconv->negative_sign);
	__toascii_a((char *)myathdp->elconv_a->left_parenthesis,
				(char *)syslconv->left_parenthesis);
	__toascii_a((char *)myathdp->elconv_a->right_parenthesis,
				(char *)syslconv->right_parenthesis);
	__toascii_a((char *)myathdp->elconv_a->debit_sign,
				(char *)syslconv->debit_sign);    
	__toascii_a((char *)myathdp->elconv_a->credit_sign, 
				(char *)syslconv->credit_sign);   

	return myathdp->elconv_a;
}

/*%PAGE																*/
/*********************************************************************
*
*	Name     :	__setlocale_a 
*	Function :	ASCII front-end for setlocale
*
*********************************************************************/
char *__setlocale_a(int category, const char *locale)
{
	char	*locale_e;
	char	*c;
	ATHD_t	*myathdp;

	myathdp = athdp();
	if (locale) {
		locale_e = __getEstring1_a(locale);
		c = setlocale(category, locale_e);
	} 
	else
		c = setlocale(category, NULL);
	if (c)
		__toascii_a(myathdp->estring1_a,c);
	return myathdp->estring1_a;
}
/*%PAGE                                                             */
/*********************************************************************
*
*   Name     :  term_locale
*   Function :  Termination routine for the locale
*               library routines. This routine runs when the thread
*               is terminated to clean up resources obtained by the
*               terminating thread.
*
*********************************************************************/
void term_locale(ATHD_t *athdptr)
{
	struct lconv * loc_elconv_a;
	if ((loc_elconv_a = athdptr->elconv_a) != 0) {
		athdptr->elconv_a = NULL;
		free(loc_elconv_a);
	}
	return;
}
	
