#ifndef VERSION_H
#define VERSION_H

/************************************************************************
 * iDiRT 1.x								*
 * 1994-1996 by Illusion						*
 ************************************************************************/

/************************************************************************
 * VERSION.H : Version identification file				*
 ************************************************************************/

/************************************************************************
 * This file contains version information for iDiRT and also contains	*
 * information about the machine that the MUD is running on.		*
 ************************************************************************/

#ifdef __linux__
#undef OS
#define OS "linux"
#ifdef __amd64__
#define ARCH "x64"
#else
#ifdef __linux__
#define ARCH "x86"
#endif
#endif
#endif


#define HEADER		ARCH  "(" OS ")"	/* Machine/OS Header	*/
#define	DIRT_VERSION	"1.82.00"		/* Version ID		*/

#define	VERSION		"iDiRT " DIRT_VERSION	/* Version for Display	*/

#ifdef	IS_ALPHA
#undef	VERSION
#define	VERSION		"iDiRT " DIRT_VERSION " (Alpha)"
#endif

#ifdef	IS_BETA
#undef	VERSION
#define	VERSION		"iDiRT " DIRT_VERSION_" (Beta)"
#endif

#endif
