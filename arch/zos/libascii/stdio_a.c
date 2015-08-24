/********************************************************************/
/*																	*/
/* Name		: 	stdio_a.c  											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997, 1998 .                */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the			*/
/*			  	stdio functions.     								*/
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
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <netdb.h>
#include <regex.h>
#include <sys/time.h>
#include "global_a.h"
 
#ifdef GEN_PRAGMA_EXPORT
 #pragma export(__fdopen_a)
 #pragma export(__fgets_a)
 #pragma export(__fopen_a)
 #pragma export(__fputc_a)
 #pragma export(__fputs_a)
 #pragma export(__fread_a)
 #pragma export(__freopen_a)
 #pragma export(__fwrite_a)
 #pragma export(__fwrite_allascii_a)
 #pragma export(__getc_a)
 #pragma export(__getc_ascii_a)
 #pragma export(__getopt_a)
 #pragma export(__gets_a)
 #pragma export(__perror_a)
 #pragma export(__popen_a)
 #pragma export(__putc_a)
 #pragma export(__puts_a)
 #pragma export(__remove_a)
 #pragma export(__rename_a)
 #pragma export(__setvbuf_a)
 #pragma export(__tempnam_a)
 #pragma export(__tmpnam_a)
#endif
 
#define WRKBUFSIZ _POSIX_PATH_MAX
 
/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII front-end routines for STDIO functions						*/
/*																	*/
/********************************************************************/

FILE *__fdopen_a(int fildes, const char *options)
{
	return fdopen(fildes, (const char *) __getEstring2_a(options));
}
 
char *__fgets_a(char *string, int n, FILE *stream)
/* assume the input is in ascii format if file, in ebcdic if stdin */
{
	if (stream==stdin) {  /* assume the input is ebcdic */
		if (fgets(string, n, stream) != NULL) {
			__toascii_a(string, string);
			return(string);
		}
		else
			return(NULL);
	}
	else {
		int i, len;
		fpos_t pos;
		fgetpos(stream, &pos);
		if (fgets(string, n, stream) != NULL) {
			len=strlen(string);
			for (i=0; string[i]!=0x0A && i<len; i++);
			if (i==len || string[i+1]=='\0')
				return(string);
			else {
				fsetpos(stream, &pos); /* reset the stream position */
				fgets(string, i+2, stream); /* gets again but only for i+1 bytes */
				return(string);
			}
		}
		return(NULL);
	}
}
 
FILE *__fopen_a(const char *path, const char *mode)
{
	return fopen((const char *) __getEstring1_a(path),
				 (const char *) __getEstring2_a(mode));
}
 
int __fputc_a(int c, FILE *stream)
{
	char input_char[]=" ";      /* 2 bytes work area : ' '+'\0' */
	input_char[0]=c;
	if ((stream==stdout) || (stream == stderr))
		__toebcdic_a(input_char, input_char);
	return (fputc(input_char[0], stream));
}
 
int __fputs_a(char *fstring, FILE *stream)
{
	int tmpint;
	/* if stdout or stderr, then convert string to ebcdic, else
       preserve in ascii 											*/
	if ((stream == stderr) || (stream == stdout)) {
		__toebcdic_a(fstring,fstring);
		tmpint = fputs(fstring,stream);
		__toascii_a(fstring,fstring);      /* convert back to ascii */
		return(tmpint);
	}
	else
	 	return(fputs(fstring,stream));
}
 
int __fread_a(void *buffer,size_t size,size_t count, FILE *stream)
{
	size_t tmpint;
	/* if stdin , then convert string to ascii for return          */
	if (stream == stdin) {
		tmpint = fread(buffer,size,count,stream);
		__toascii_a(buffer,buffer);       /* convert back to ascii */
		return(tmpint);
	}
	else
		return(fread(buffer,size,count,stream));
}
 
FILE *__freopen_a( const char *path, const char *mode, FILE *stream)
{
	return freopen((char const *) __getEstring1_a(path),
				   (char const *) __getEstring2_a(mode),
				   stream);
}
 
size_t __fwrite_a(const void *buffer, size_t size, size_t count, FILE *stream)
{
	size_t	bytes;
	if (stream==stderr || stream==stdout)
		__toebcdic_a((char *) buffer, (char *) buffer);
	bytes = fwrite(buffer, size, count, stream);
	if (stream==stderr || stream==stdout)
		__toascii_a((char *) buffer, (char *) buffer);
	return bytes;
}
 
size_t __fwrite_allascii_a(const void *buffer, size_t size, size_t count, FILE *stream)
{
	size_t bytes;
	bytes = fwrite(buffer, size, count, stream);
	return bytes;
}
 
int __getc_a(FILE *stream)
{
	char input_char[]=" ";      /* 2 bytes work area : ' '+'\0' */
 
	input_char[0]=getc(stream);
	if (input_char[0] == 0xff)
		return(-1);
	if (stream==stdin)
		__toascii_a(input_char, input_char);
	return (input_char[0]);
}
 
int __getc_ascii_a(FILE *stream)
{
	char input_char[]=" ";      /* 2 bytes work area : ' '+'\0' */
	input_char[0]=getc(stream);
	if (input_char[0] == 0xff)
		return(-1);
	return (input_char[0]);
}

/* The definiton of this function has been changed to remove the 	*/
/* const from the 2nd argument. This was done to eliminate a warning*/
/* message on the calls to __argvtoebcdic_a and __argvtoascii_a.	*/
/* THis code actually violates the design of the real getopt in that*/
/* the 2nd argument valuesare actually changed for a short period of*/
/* time between the two __argvtoxxx routines.						*/
/* int __getopt_a(int argc, char *const argv[], const char *varname)*/
int __getopt_a(int argc, char *argv[], const char *varname)
{
	ATHD_t *myathdp;
	char tmpvarname[80];
	char *tmpvarnamep;
	int varlen=80;
	int tmpoptp;
	char optarg_ascii[80];
	char tmpoptp_ascii[2];
	char optopt_ascii[2];
	char tmpasciibuff[2];
	char *tmpoptp_ascii_p;
	char *optopt_ascii_p;
	char *getreturnp;

	__argvtoebcdic_a(argc,argv);

	if ((varlen = 1+strlen(varname)) > 80)
		tmpvarnamep = malloc(varlen);
	else
		tmpvarnamep = &tmpvarname[0];
	__toebcdic_a(tmpvarnamep,varname); 

	tmpoptp = getopt(argc,argv,tmpvarnamep);
	__toascii_a(optarg_ascii,optarg); 

	tmpoptp_ascii_p = &tmpoptp_ascii[0];
	optopt_ascii_p = &optopt_ascii[0];

	if (varlen > 80)
		free(tmpvarnamep);
	__argvtoascii_a(argc,argv);
	sprintf(optarg,"%s",optarg_ascii);
	if (tmpoptp != -1) {
		sprintf(tmpasciibuff,"%c",tmpoptp);
		__toascii_a(tmpoptp_ascii_p,tmpasciibuff);
		if (optopt) {
			sprintf(tmpasciibuff,"%c",optopt);
			__toascii_a(optopt_ascii_p,tmpasciibuff);
			optopt = (int)optopt_ascii[0];
		}
		return((int)tmpoptp_ascii[0]);
	}
	else {
		return(tmpoptp);
	}
}
/**/
 
char *__gets_a(char *buffer)
{
	if (gets(buffer) != NULL) {
		__toascii_a(buffer, buffer);
		return(buffer);
	}
	else
		return(NULL);
	}
 
void __perror_a( const char *s )
{
	perror((const char *) __getEstring1_a(s));
}
 
FILE *__popen_a(const char *command, const char *mode)
{
	return popen((const char *) __getEstring1_a(command),
				 (const char *) __getEstring2_a(mode));
}
 
int __putc_a(int c, FILE *stream)
{
	char input_char[]=" ";      /* 2 bytes work area : ' '+'\0' */
 
	input_char[0]=c;
	if (stream==stdout || stream==stderr)
		__toebcdic_a(input_char, input_char);
	return (putc(input_char[0], stream));
}
 
/* puts goes to stdout by definition */
int  __puts_a(char *buffer)
{
	return(puts((const char *) __getEstring1_a(buffer)));
}
 
int __remove_a(const char *path)
{
	return remove((const char *) __getEstring1_a(path));
}
 
int __rename_a( const char *old, const char *new )
{
	return rename((const char *)__getEstring1_a(old),
			   	  (const char *)__getEstring2_a(new));
}
 
int __setvbuf_a(FILE *stream,char *buf,int type, size_t size)
{
	return setvbuf(stream,(char *) __getEstring1_a(buf), type, size);
}

char *__tempnam_a(const char *dir, const char *pfx)
{
	return tempnam((const char *) __getEstring1_a(dir),
				   (const char *) __getEstring2_a(pfx));
}

char *__tmpnam_a(char *string)
{
	char *	tmp_string;
	tmp_string = tmpnam(string);
	if (string == NULL)
		return __getAstring1_a(tmp_string);
	else
		{
			__toascii_a(string,string);
			return string;
		}
}
