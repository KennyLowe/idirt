#ifndef _VERSION_H
#define _VERSION_H

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

#include "MACHINE.H"				/* Get Machine Defines	*/

#define _HEADER_	_ARCH_" ("_OS_")"	/* Machine/OS Header	*/
#define	_VERSION_	"1.82.00"		/* Version ID		*/

#define	VERSION		"iDiRT "_VERSION_	/* Version for Display	*/

#ifdef	IS_ALPHA
#undef	VERSION
#define	VERSION		"iDiRT "_VERSION_" (Alpha)"
#endif

#ifdef	IS_BETA
#undef	VERSION
#define	VERSION		"iDiRT "_VERSION_" (Beta)"
#endif

#endif
