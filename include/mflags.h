#ifndef _MFLAGS_H
#define _MFLAGS_H

/*
**  Mflag defines
*/

#define	MFL_CFIREBALL	0	/* Can fireball */
#define MFL_CMISSILE	1
#define MFL_CFROST	2
#define MFL_CSHOCK	3
#define MFL_NFIREBALL	4	/* Negate effect of fireball */
#define MFL_NMISSILE	5
#define MFL_NFROST	6
#define MFL_NSHOCK	7
#define MFL_NOHEAT	8	/* Mobile doesn't like things lit */
#define MFL_THIEF	9	/* Mobile steals */
#define MFL_SWPN	10	/* Mobile steals your weapon */
#define MFL_SWORN	11	/* Mobile may steal what you wear */
#define MFL_PITIT	12	/* Mobile pits, what it steals */
#define MFL_DLEVEL	13	/* Mobile drains level */
#define MFL_DSCORE	14	/* Mobile drains your score */
#define MFL_TALK	15	/* Mobile can talk */
#define MFL_BAR_N	16	/* Mobile may bar your way in <dir> */
#define MFL_BAR_E	17
#define MFL_BAR_S	18
#define MFL_BAR_W	19
#define MFL_BAR_U	20
#define MFL_BAR_D	21
#define MFL_QFOOD	22	/* Mobile quits when given food */
#define MFL_BLIND	23	/* Mobile scratches your eyes and blinds you */
#define MFL_DEAF	24	/* Mobile screams and deafens you */
#define MFL_DUMB	25	/* Mobile dumbs you */
#define MFL_NOGRAB	26	/* Cannot take items from the mobiles room */
#define MFL_GRABH	27	/* NOGRAB, only mobile attacks */
#define MFL_PICK	28	/* Mobile do picks up things it finds */
#define MFL_NOSTEAL	29	/* Stealing from mobile and be attacked */
#define MFL_BLOCK	30	/* BAR, only mobile attacks */
#define MFL_CROSS	31	/* Mobile doesn't attack players with cross */

#endif
