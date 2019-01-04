#ifndef _SFLAGS_H
#define _SFLAGS_H

/* iDiRT
 * Special Flag Definitions (sflags)
 */

#define	SFL_FEMALE	0     	/* If you don't know, you're too young	*/
#define SFL_FREAQ	1     	/* Player has been turned into a FreaQ	*/
#define SFL_COLOR	2     	/* ANSI color support			*/
#define SFL_NOSHOUT	3     	/* Don't receive shouts			*/
#define	SFL_BRIEF	4     	/* Brief mode, no long descriptions	*/
#define SFL_QUIET	5     	/* Doesn't receive []'d messages	*/
#define	SFL_OCCUPIED	6	/* This mobile is aliased		*/
#define SFL_ALOOF	7     	/* May not be bothered by mortals	*/
#define SFL_BLIND	8     	/* Player is blind			*/
#define SFL_DEAF	9     	/* Player is deaf			*/
#define SFL_DUMB	10    	/* Player can't speak			*/
#define SFL_CRIPPLED	11	/* Player can't move			*/
#define SFL_BUSY        12    	/* Don't get any messages		*/
#define SFL_NOWIZ       13    	/* Wizard can't hear 'wiz' messages.	*/

/* All iDiRT Defines */
#define SFL_NOGOSSIP    14	/* Do not listen to Gossip channel	*/
#define SFL_AUTOEXIT    15	/* Display exits automatically		*/
#define SFL_NOAWIZ      16	/* Cannot hear 'awiz' messages.		*/
#define SFL_NOGOD       17	/* Cannot hear 'god' messages.		*/
#define SFL_NOWISH      18	/* Wizards can't hear wishes		*/
#define SFL_NOINV       19	/* Do not display others inventory	*/
#define SFL_NOANON      20	/* Do not listen to Anon channel	*/
#define SFL_HEARBACK    21	/* Display what a user just said	*/
#define SFL_NOPROPHET	22	/* Cannot hear 'prophet' messages.	*/
#define SFL_NEWSTYLE	23	/* NewStyle output			*/
#define SFL_NOHEAL	24	/* Person cannot be healed		*/
#define SFL_DRUNK	25	/* Person is drunk			*/
#define SFL_SILENT	26	/* Person enters game silently		*/
#define SFL_AWAY	27	/* Person is away from the keyboard	*/
#define SFL_NOADV	28	/* Cannot hear 'advisor' messages.	*/
#define SFL_NOUPPER	29	/* Cannot hear 'upper god' messages.	*/
#define	SFL_NOSLAIN	30	/* Do not display slain people		*/
#define	SFL_NOFIGHT	31	/* Do not view other fights		*/
#define SFL_LIT		32	/* Player is lit			*/
#define SFL_HEALFIGHT	33	/* Can be healed in a fight		*/
#define SFL_AUTOHEAL	34	/* Heal player after fight		*/
#define SFL_NOFINGER	35	/* Player cannot be finger'd		*/
#define SFL_NOBEEP	36	/* Player does not hear beeps 		*/
#define SFL_SEEEXT	37	/* Player can see extended messages	*/
#define SFL_NOBLINK	38	/* Ignore blinking characters		*/
#define SFL_CODING	39	/* Busy Coding				*/
#define SFL_NOWET	40	/* Ignore weather messages		*/
#define SFL_NOFLAG	41	/* Ignore No* flags in who/whon		*/
#define SFL_NOORACLE	42	/* Turn off The Oracle			*/
#define SFL_NOPUFF	43	/* Turn off Puff			*/
#define	SFL_ONPROBATION	44	/* Player is on probation		*/
#define	SFL_INTERVIS	45	/* Player is invis on InterMUD		*/
#define	SFL_NO_IGOSSIP	46	/* Player has InterMUD Gossip off	*/

#endif
