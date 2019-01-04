#ifndef _LFLAGS_H
#define _LFLAGS_H

/* iDiRT
 * Location Flag Definitions (lflags)
 */

#define LFL_LIGHT       0	/* No need for light src here (default)	*/
#define LFL_DARK	1	/* Always dark				*/
#define LFL_L_REAL	2	/* Dark during night, light during day	*/

#define LFL_T_ORDINARY	3	/* Normal (default)			*/
#define LFL_HOT		4	/* Need protection			*/
#define LFL_COLD	5	/* need protection			*/
#define LFL_T_REAL	6	/* Changing with day/night and season	*/

#define LFL_DEATH	7	/* Mortals are killed in this room	*/
#define LFL_CANT_SUMMON	8	/* Can't summon someone from this room	*/ 
#define LFL_NO_SUMMON	9	/* Can't do summon in this room		*/
#define LFL_NO_QUIT	10	/* Can't quit in this room		*/
#define LFL_NO_SNOOP	11	/* No snooping				*/
#define LFL_NO_MOBILES	12	/* No mobiles can enter this room	*/
#define LFL_NO_MAGIC	13	/* No magic can be cast in this room	*/
#define LFL_PEACEFUL	14	/* Can't fight in this room		*/
#define LFL_SOUNDPROOF	15	/* No sound can enter this room		*/
#define LFL_ONE_PERSON	16	/* One person in room at a time		*/
#define LFL_PARTY	17	/* Emote can be done freely (obsolete)	*/
#define LFL_PRIVATE	18	/* Private (2 people only)		*/
#define LFL_ON_WATER	19	/* Is on water (need boat)		*/
#define LFL_IN_WATER	20	/* Room is under water			*/
#define LFL_OUTDOORS	21	/* Room is outdoors (Weather messages)	*/
#define LFL_T_EXTREME	22	/* HOT in the day, COLD in the night	*/
#define LFL_NEGREGEN	23	/* Negative regeneration, eat a lot!	*/
#define LFL_NOREGEN	24	/* You do not regenerate health		*/
#define LFL_WIZARD	25	/* Wizard's+ can only enter this room	*/
#define LFL_AWIZ	26	/* ArchWiz's+ can only enter this room	*/
#define LFL_AVATAR	27	/* Avatar's+ can only enter this room	*/
#define LFL_GOD		28	/* Only God's can enter this room	*/
#define LFL_OWNER	29	/* Only the owner can enter this room	*/
#define LFL_NOINVIS	30	/* Cannot enter this room invis		*/
#define LFL_NOGOTO	31	/* Person cannot 'goto' this room	*/
#define LFL_MAZE	32	/* Wiz-ArchWizs won't see exit rooms	*/
#define LFL_PUBLIC	33	/* A public room, any Wizard+ can edit	*/
#define LFL_FASTHEAL	34	/* A person will heal faster here	*/
#define LFL_NOAT	35	/* 'At' cannot be used in this room	*/
#define LFL_FASTMANA	36	/* A person will heal mana faster here	*/
#define LFL_MOVE	37	/* People can move in a lflag'd room	*/

#endif
