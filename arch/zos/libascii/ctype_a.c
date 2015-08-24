/********************************************************************/
/*																	*/
/* Name		: 	ctype_a.c 											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII isxxxx functions for use by OE/MVS	*/
/*				compiled with the ASCII flag.						*/
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

#ifdef GEN_PRAGMA_EXPORT
#pragma export(__isalnum_a)
#pragma export(__isalpha_a)
#pragma export(__iscntrl_a)
#pragma export(__isdigit_a)
#pragma export(__isgraph_a)
#pragma export(__islower_a)
#pragma export(__isprint_a)
#pragma export(__isspace_a)
#pragma export(__isupper_a)
#pragma export(__isxdigit_a)
#pragma export(__tolower_a)
#pragma export(__toupper_a)
#pragma export(__ispunct_a)
#endif

/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII front-end routines for CTYPE functions						*/
/*																	*/
/********************************************************************/
 
int __isalnum_a( int c )
{
	return( ( c >= 'A' && c <= 'Z' ) || 
			( c >= 'a' && c <= 'z' ) || 
			( c >= '0' && c <= '9' ) );
}

int __isalpha_a( int c )
{
	return( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) );
}

int __iscntrl_a( int c ) 
{
	return( (c >= '\x00' && c <= '\x1F') || (c == 'x\7F') );   
} 

int __isdigit_a( int c )
{
	return( c >= '0' && c <= '9' );
}

int __isgraph_a( int c)
{
	return( (c > '\x20' && c < '\x7F'));  
}

int __islower_a( int c )
{
	return( c >= 'a' && c <= 'z' );
}

int __isprint_a( int c)
{
	return( (c >= '\x20' && c < '\x7F'));  
}

int __ispunct_a( int c)
{
	return(( (c >= '\x21' && c <= '\x2f') || \
			 (c >= '\x3a' && c <= '\x40') || \
			 (c >= '\x5b' && c <= '\x60') || \
			 (c >= '\x7b' && c <= '\x7e') )   ); 
}

int __isspace_a( int c )
{
	return( c == ' ' || c == '\n' || c == '\t' || 
			c == '\v' || c == '\r' || c == '\f' );
}

int __isupper_a( int c )
{
	return( c >= 'A' && c <= 'Z' );
}

int __isxdigit_a( int c )
{
	return( ( c >= '0' && c <= '9' ) || 
			( c >= 'a' && c <= 'f' ) || 
			( c >= 'A' && c <= 'F' ) );
}

int __tolower_a( int c )
{
	return( (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c );
}

int __toupper_a( int c )
{
	return( (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c );
}
