/********************************************************************/
/*																	*/
/* Name		: 	scanf_a.c 											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997, 1998 .                */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ACSII-to-EBCDIC front end to the			*/
/*			  	following functions: 								*/
/*					- fscanf NOTE -- not exported as this only 		*/
/*								     accepts EBCDIC input.			*/ 
/*					- scanf 							 			*/
/*					- sscanf 										*/
/*																	*/
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
/*				2) The %n$ form of the conversion specifiers are 	*/
/*				not supported by any of the scanf variations.		*/
/*																	*/
/*				3) This code supports a maximum of 20 variable		*/
/*				arguments.											*/
/*																	*/ 
/********************************************************************/
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
/*#include <global.h> */
#ifdef GEN_IEEE_FP
#include <ieee_md.h>
#endif
#include "global_a.h"

#ifdef GEN_PRAGMA_EXPORT
/* #pragma export(__fscanf_a)										*/
#pragma export(__scanf_a)
#pragma export(__sscanf_a)
#endif

typedef	int		BOOL;
#define	FALSE	0
#define	TRUE	!FALSE

/*********************************************************************
*
*	Proto-type statements   
*
*********************************************************************/
int		GetArgs(char * format, va_list parg);
void	ConvertArgsToAscii(int);

/*********************************************************************
*
*	Local variables         
*
*********************************************************************/
typedef	struct {
	void	*argPtr;			/* Arg ptr							*/
	char	argPrefix;			/* Prefix, h,l, L					*/	
	char	argCS;				/* Conversion specifier				*/
	int		argWidth;			/* Width specified in string 		*/
} argInfo;
argInfo	ai[20];

/*%PAGE																*/
/*********************************************************************
*
*	Name   	 :	__fscanf_a  
*	Function :	ASCII front-end to fscanf 
*	Notes    :	Assumes - EBCDIC input file (NOTE this restriction)
*				        - ASCII format string  
*				ASCII output in any character or string variables.
*				Although implemented here, the functiopn only accepts
*						  EBCDIC input so it will not be exported.
*
*********************************************************************/
int __fscanf_a(FILE *stream, const char *format, ...)
{ 
	extern	argInfo ai[20]; 
	va_list	parg;
	int		result;
	int		argCount;
	char	eformat[200]; 
	char	*eformatp;
	size_t	lenformat;

	/* Copy format string to temp buffer and convert to EBCDIC		*/
	lenformat = strlen(format);
	if (lenformat < sizeof(eformat) )
		eformatp = &eformat[0];
	else
		eformatp = malloc(lenformat+1);
	__toebcdic_a(eformatp,format);

	/* Collect arg info, call real scan routine and convert any		*/
	/* data returned as character or string to ASCII				*/
	va_start(parg,format);
	argCount = GetArgs(eformatp,parg);
	result = fscanf(stream,eformatp,
			ai[0].argPtr,ai[1].argPtr,ai[2].argPtr,ai[3].argPtr,
			ai[4].argPtr,ai[5].argPtr,ai[6].argPtr,ai[7].argPtr,
			ai[8].argPtr,ai[9].argPtr,ai[10].argPtr,ai[11].argPtr,
			ai[12].argPtr,ai[13].argPtr,ai[14].argPtr,ai[15].argPtr,
			ai[16].argPtr,ai[17].argPtr,ai[19].argPtr,ai[19].argPtr);
	if (result != EOF && result > 0)
		ConvertArgsToAscii(argCount);
	va_end(parg);

	/* Free temp buffer if one was obtained							*/
	if (lenformat >= sizeof(eformat))
		free(eformatp);

	return(result);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name   	 :	__scanf_a  
*	Function :	ASCII front-end to scanf 
*	Notes    :	Assumes - ASCII  format string
*				        - EBCDIC input from stdin   
*				ASCII output in any character or string variables.
*
*********************************************************************/
int __scanf_a(const char *format, ...)
{
	extern	argInfo ai[20]; 
	va_list	parg;
	int		result;
	int		argCount;
	char	eformat[200]; 
	char	*eformatp;
	size_t	lenformat;

	/* Copy format string to temp buffer and convert to EBCDIC		*/
	lenformat = strlen(format);
	if (lenformat < sizeof(eformat) )
		eformatp = &eformat[0];
	else
		eformatp = malloc(lenformat+1);
	__toebcdic_a(eformatp,format);

	/* Collect arg info, call real scan routine and convert any		*/
	/* data returned as character or string to ASCII				*/
	va_start(parg,format);
	argCount = GetArgs(eformatp,parg);
	result = scanf(eformatp,
			ai[0].argPtr,ai[1].argPtr,ai[2].argPtr,ai[3].argPtr,
			ai[4].argPtr,ai[5].argPtr,ai[6].argPtr,ai[7].argPtr,
			ai[8].argPtr,ai[9].argPtr,ai[10].argPtr,ai[11].argPtr,
			ai[12].argPtr,ai[13].argPtr,ai[14].argPtr,ai[15].argPtr,
			ai[16].argPtr,ai[17].argPtr,ai[19].argPtr,ai[19].argPtr);
	if (result != EOF && result > 0)
		ConvertArgsToAscii(argCount);
	va_end(parg);

	/* Free temp buffer if one was obtained							*/
	if (lenformat >= sizeof(eformat))
		free(eformatp);

	return(result);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name   	 :	__sscanf_a  
*	Function :	ASCII front-end to sscanf 
*	Notes    :	Assumes - ASCII  format string
*				        - ASCII  buffer
*				ASCII output in any character or string variables.
*
*********************************************************************/
int __sscanf_a(const char * buffer, const char *format, ...)
{
	extern	argInfo ai[20]; 
	va_list	parg;
	int		result;
	int		argCount;
	char	eformat[200]; 
	char	ebuffer[200];
	char	*eformatp;
	char	*ebufferp;
	size_t	lenformat;
	size_t	lenbuffer;

	/* Copy buffer string to temp buffer and convert to EBCDIC		*/
	lenbuffer = strlen(buffer);
	if (lenbuffer < sizeof(ebuffer))
		ebufferp = &ebuffer[0];
	else	
		ebufferp = malloc(lenbuffer+1);
	__toebcdic_a(ebufferp,buffer);

	/* Copy format string to temp buffer and convert to EBCDIC		*/
	lenformat = strlen(format);
	if (lenformat < sizeof(eformat) )
		eformatp = &eformat[0];
	else
		eformatp = malloc(lenformat+1);
	__toebcdic_a(eformatp,format);

	/* Collect arg info, call real scan routine and convert any		*/
	/* data returned as character or string to ASCII				*/
	va_start(parg,format);
	argCount = GetArgs(eformatp,parg);
	result = sscanf(ebufferp,eformatp,
			ai[0].argPtr,ai[1].argPtr,ai[2].argPtr,ai[3].argPtr,
			ai[4].argPtr,ai[5].argPtr,ai[6].argPtr,ai[7].argPtr,
			ai[8].argPtr,ai[9].argPtr,ai[10].argPtr,ai[11].argPtr,
			ai[12].argPtr,ai[13].argPtr,ai[14].argPtr,ai[15].argPtr,
			ai[16].argPtr,ai[17].argPtr,ai[19].argPtr,ai[19].argPtr);
	if (result != EOF && result > 0)
		ConvertArgsToAscii(argCount);
	va_end(parg);

	/* Free temp buffers if obtained								*/
	if (lenbuffer >= sizeof(ebuffer))
		free(ebufferp);
	if (lenformat >= sizeof(eformat))
		free(eformatp);

	return(result);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name   	 :	GetArgs
*	Function :	Read args and save info in array
*	Inputs	 :	- Conversion specification buffer (EBCDIC)
*				- Argument list
*	Outputs	 :	ai array updated with arg info 
*	Returns  :	Returns number of arguments in list
*
*********************************************************************/
int GetArgs(char *eformatp,va_list parg)
{
	extern	argInfo ai[20]; 
 	int			i,argCount,width;
	char 		*ip;
	char *		result;
	const char	listofQual[] = "hlL"; 	
	const char	listofTypes[] = "sScCdioxXufeEgGpnD";
	const int	sizeQual  = sizeof(listofQual)-1;
	const int	sizeTypes = sizeof(listofTypes)-1;
	BOOL     	sw_error;
	BOOL      	sw_skipStore;

	/* Scan thru buffer till error, max-output or EOL				*/
	argCount=0;
	sw_error = FALSE;
	for (ip=&eformatp[0] ; !sw_error && *ip != '\0';) {
		sw_skipStore = FALSE;

		/* If input character not % then just skip it 			   	*/
		if (*ip != '%') {
			ip++;
			continue;
		}

		/* At this point we have % followed by something			*/
		i = 0;

		/* Scan for width value 									*/
	 	if (ip[i+1] == '*') { 
			sw_skipStore = TRUE;   
			width = 1;
			i++;
		}
		else  
			for (width=0;isdigit(ip[i+1]); ++i)
				width = width *10 + (ip[i+1] - '0');

		/* Scan for h,l or L										*/
		if ((result = (char *)memchr(listofQual,ip[i+1],sizeQual)) != NULL) { 
			ai[argCount].argPrefix = ip[i+1];
			i++;
			}

		/* Scan for "type" character								*/
		i++;
		if (i > 1 + sizeof(eformatp)) {
			sw_error = TRUE; 
			continue;
		}

		/* If next char is valid conversion specification character	*/
		/* store arg info in array									*/	
		if ((result = (char *)memchr(listofTypes,ip[i],sizeTypes)) != NULL 
			&& sw_skipStore == FALSE) { 
			ai[argCount].argCS = ip[i];
			ai[argCount].argWidth = width;
			ai[argCount++].argPtr = va_arg(parg, void *);
			}

		/* % character was followed by a character that is not	 	*/
		/* a formatting code, ignore the % 							*/
		else
			i = 0;

		/* Incr input ptr											*/
		ip += i+1;
	}

	return (argCount);
}

/*%PAGE																*/
/*********************************************************************
*
*	Name   	 :	ConvertArgsToAscii
*	Function :	Converts character and string arguments to ASCII
*	Inputs	 :	Count of args in ai array
*	Outputs	 :	Character and string args changed to ASCII	
*	Returns	 :	None
*
*********************************************************************/
void ConvertArgsToAscii(int argCount)
{
	extern	argInfo ai[20]; 
	int		i,j;
	char	*ptrchar;
	char	tmpchar[2];
#ifdef GEN_IEEE_FP
	double	tmpdbl;
	double	*ptrdbl;
	double_t tmpIEEEdbl;
	float	tmpfp;
	float_t	tmpIEEEfp;
#endif

	/* Loop thru ai array (saved arg info)							*/
	for (i=0;i<argCount;i++) {
		switch (ai[i].argCS) {
			/* If string, convert it to ASCII						*/ 
			case 's':
			case 'S':
				__toascii_a(ai[i].argPtr,ai[i].argPtr);
				break;
			/* If character, convert all characters (one at a time)	*/
			case 'c':
			case 'C':
				ptrchar = (char *) ai[i].argPtr;
				for (j=0;j<ai[i].argWidth;j++) {
					tmpchar[0] = *ptrchar;
					tmpchar[1] = '\0';
					__toascii_a(tmpchar,tmpchar);
					*ptrchar++ = tmpchar[0];
				}
				break;
#ifdef GEN_IEEE_FP
			/* If floating point, convert to ieee format			*/
			case 'e':
			case 'E':
			case 'f':
			case 'g':
			case 'G':
				ptrdbl = (double *) ai[i].argPtr;
				if (ai[i].argPrefix == 'L'
					| ai[i].argPrefix == 'l') {
					memcpy(&tmpdbl,ptrdbl,sizeof(double));
					ConvertDoubleToIEEE(&tmpdbl,&tmpIEEEdbl);
					memcpy(ptrdbl,&tmpIEEEdbl,sizeof(double));
					}
				else {
					memcpy(&tmpfp,ptrdbl,sizeof(float));
					ConvertFloatToIEEE(&tmpfp,&tmpIEEEfp);
					memcpy(ptrdbl,&tmpIEEEfp,sizeof(float));
					}
				break;
#endif
			/* For all other types, no conversion required			*/
			default	:
				break;
		}
	}

	return;
}
