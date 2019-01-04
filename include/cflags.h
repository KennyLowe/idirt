#ifndef _CFLAGS_H
#define _CFLAGS_H

#define	IN_ROOM 	0	/* In room (room) */
#define	IN_CONTAINER	1	/* In container (object) */
#define CARRIED_BY	2	/* Carried by player/mobile */
#define	WORN_BY		3	/* Worn by player/mobile */
#define	WIELDED_BY	4	/* Wielded by player/mobile */
#define	BOTH_BY		5	/* Both worn and wielded player/mobile */

#define iswielded(O)	(ocarrf(O) >= WIELDED_BY)
#define isworn(O)	(ocarrf(O) == WORN_BY || ocarrf(O) == BOTH_BY)

#endif
