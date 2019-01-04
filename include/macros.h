#ifndef _MACROS_H
#define _MACROS_H

#define min(x,y)                ((x)>(y)?(y):(x))
#define max(x,y)                ((x)<(y)?(y):(x))
#define arraysize(a)            (sizeof(a)/sizeof(a[0]))
#define EVEN(N)                 (((N)&1) == 0)
#define ODD(N)                  (((N)&1) != 0)

#define setbits(W,F)		((W) |= (F))
#define clrbits(W,F)		((W) &= ~(F))

/* Extracts the bits in mask M from W */
#define bits(W,M)		((W) & (M))

#define xsetbit(W,V)		setbits((W),(1 << V))
#define xclrbit(W,V)		clrbits((W),(1 << V))
#define xtstbit(W,V)	       	bits((W),(1 << V))

#ifndef SYS_NO_RANDOM
#define my_random()             ((unsigned int)random())
#else
#define my_random()             ((unsigned int)(rand() >> 8))
#endif

#endif
