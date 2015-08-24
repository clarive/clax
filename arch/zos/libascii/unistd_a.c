/********************************************************************/
/*																	*/
/* Name		: 	unistd_a.c 											*/	
/*                                                                  */
/* Copyright:   Licensed Materials - Property of IBM.               */
/*              (C) Copyright IBM Corp. 1997.                       */
/*              All rights reserved.                                */
/* 																	*/
/* Function :	Contains ASCII-to-EBCDIC front end to the			*/
/*			  	unistd functions.    								*/
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
#include <pwd.h>
#include "global_a.h"
 
#ifdef GEN_PRAGMA_EXPORT
#pragma export(__access_a)
#pragma export(__chdir_a)
#pragma export(__chown_a)
#pragma export(__execv_a)
#pragma export(__execve_a)
#pragma export(__execvp_a)
#pragma export(__getcwd_a)
#pragma export(__gethostname_a)
#pragma export(__getlogin_a)
#pragma export(__getpass_a)
#pragma export(__getwd_a)
#pragma export(__link_a)
#pragma export(__rmdir_a)
#pragma export(__unlink_a)
#endif
 
/*%PAGE																*/
/********************************************************************/
/*																	*/
/* ASCII front-end routines for UNISTD functions					*/
/*																	*/
/********************************************************************/
 
int __access_a(const char *path, int how)
{
	return access((const char *) __getEstring1_a(path), how);
}
 
int __chdir_a(const char *path)
{
	return chdir((const char *) __getEstring1_a(path));
}
 
int __chown_a(const char *path, uid_t owner, gid_t group)
{
	return chown((const char *) __getEstring1_a(path), owner, group);
}

int __execv_a(const char *path, char *const argv[])
{
	return execv((const char *) __getEstring1_a(path), argv);
}

int __execve_a(const char *path, char *const argv[], char *const envp[])
{
	return execve((const char *) __getEstring1_a(path), argv, envp);
}

int __execvp_a(const char *file, char *const argv[])
{
	return execvp((const char *) __getEstring1_a(file), argv);
}
 
char *__getcwd_a(char *buffer, size_t size)
{
	if (getcwd(buffer, size) != 0) {
		__toascii_a(buffer, buffer);
		return(buffer);
	}
	else
		return(NULL);
}
 
int __gethostname_a(char *name, size_t namelen)
{
	int g_hostint;
	if ((g_hostint=gethostname(name,namelen)) != -1) {
		__toascii_a(name,name);
	}
	return g_hostint;
}
 
char *__getlogin_a(void)
{
	char * user;
	user = getlogin();
	if ((user)!= NULL) {
		__toascii_a(user,user);
		return(user);
	}
	else
		return(NULL);
}

char * __getpass_a(const char *prompt)
{ 
	char	* p;

	p = getpass((const char *) __getEstring1_a(prompt));
	if ((p) != NULL) {
		__toascii_a(p,p);
		return(p);
		}
	else
		return(NULL);
}

char *__getwd_a(char *path_name)
{
	char	*p;

	p = getwd(path_name);
	__toascii_a(p,p);
	return(p);
}
 
int __link_a(const char *oldfile, const char *newname)
{
	return link((const char *) __getEstring1_a(oldfile),
				(const char *) __getEstring2_a(newname));
}
 
int __rmdir_a(const char *path)
{
	return rmdir((const char *) __getEstring1_a(path));
}
 
int __unlink_a(const char *path)
{
	return unlink((const char *) __getEstring1_a(path));
}
