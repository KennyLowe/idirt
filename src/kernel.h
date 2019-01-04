#ifndef _KERNEL_H
#define _KERNEL_H

#include <sys/types.h>
#include <sys/param.h>
#include <setjmp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/* Include user-specified configuration constants and defines. */
#include "VERSION.H"
#include "config.h"
#include "levels.h"
#include "exits.h"
#include "types.h"
#include "utils.h"
#include "mudtypes.h"
#include "mudmacros.h"
#include "extern.h"
#include "files.h"

#define NEW(t, c)		((t *)xmalloc((c), sizeof (t)))
#define FREE(x)			xfree ((void *) x)
#define BCOPY(s,l)              memcpy(xmalloc(1, l),s,l)
#define COPY(s)			strcpy(NEW(char, strlen(s) + 1), s)
#define EMPTY(p)		(*(p) == '\0')
#define EQ(a, b)		(strcasecmp((a), (b)) == 0)

#define OPERATOR(n)	(EQ(n,MASTERUSER))

#define WIZZONE_EXIST_H  72L      /* How many hours will a wizard's zone be
                                     kept in the game without him being on ? */

#endif
