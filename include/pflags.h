#ifndef _PFLAGS_H
#define _PFLAGS_H

/* iDiRT
 * Privelige Flag Definitions (pflags)
 */

/* Protection bits */
#define	PFL_NOEXOR	0	/* Can not be exo'd			*/
#define	PFL_NOSNOOP	1	/* Can not be snooped			*/
#define	PFL_NOHASSLE	2	/* Can not be attacked			*/
#define	PFL_NOALIAS	3	/* Can not be aliased into		*/
#define PFL_NOZAP	4	/* Can not be zapped			*/
#define PFL_NOSUMMON	5	/* Can not be summoned			*/
#define PFL_NOTRACE	6	/* Can not be traced			*/
#define PFL_NOSTEAL	7	/* You cannot steal from him		*/
#define PFL_NOMAGIC	8	/* Cripple, blind, etc does not work	*/
#define PFL_NOFORCE	9	/* Can not be forced			*/
#define PFL_NOPUNT	10	/* Can not be punted			*/
#define PFL_NOFORGET	11	/* Can not be forgotten			*/
#define	PFL_NOBURN	12	/* Can not be burned			*/
#define PFL_NOSIC	13	/* Can not be sic'd			*/
#define	PFL_NOPROBATION	14	/* Can not be placed on probation	*/

/* Priveliges */
#define PFL_FROB	16	/* Can frob and 17-18 works on others	*/
#define PFL_CH_SCORE	17	/* May change score			*/
#define PFL_CH_LEVEL	18	/* May change level			*/
#define PFL_CH_MDATA	19	/* May change mobile data		*/
#define PFL_CLONE	20	/* May use the CLONE command		*/
#define PFL_LD_STORE	21	/* May use the LOAD and STORE commands	*/
#define	PFL_PFLAGS	22	/* May set privileges on others		*/
#define PFL_MASK	23	/* May set priveliges in MASK		*/
#define PFL_ROOM	24	/* May set location bits and exits	*/
#define PFL_MFLAGS	25	/* May set mobile bits			*/
#define PFL_OBJECT	26	/* May set object properties with SET	*/
#define PFL_TITLES	27	/* May set titles/setins permanently	*/
#define PFL_UAF		28	/* May get/set data for players not on	*/
#define PFL_EXOR	29	/* May exorcise players			*/
#define PFL_RESET	30	/* May reset the game			*/
#define PFL_SHUTDOWN	31	/* May use Opengame and Shutdown	*/
#define PFL_SNOOP	32	/* May snoop or trace other players	*/
#define PFL_HEAL	33	/* May use Heal				*/
#define PFL_ALIAS	34	/* May alias mobiles			*/
#define PFL_ALIASP	35	/* May alias other players		*/
#define PFL_RAW		36	/* May send Raw messages		*/
#define PFL_EMOTE	37	/* May use Emote			*/
#define PFL_ECHO	38	/* May use Echo and Echoall		*/
#define PFL_ZAP		39	/* May use Zap				*/
#define PFL_RES		40	/* May use Resurrect			*/
#define PFL_SHUSER	41	/* Can see other players hosts		*/
#define PFL_STATS	42	/* May use STAT and SHOW and PRIVS	*/
#define PFL_GOTO	43	/* May use GOTO and IN/AT		*/
#define PFL_SUMOBJ	44	/* May summon objects			*/
#define PFL_WEATHER	45	/* May control the weather		*/
#define PFL_LOCK	46	/* May lock the game			*/
#define PFL_WRECK	47	/* May wreck doors (don't need key)	*/
#define PFL_PEACE	48	/* May declare peace or war		*/
#define PFL_SYSLOG	49	/* May look at the system log-file	*/
#define PFL_STARTINVIS	50	/* May start as invisible		*/
#define PFL_TRACE	51	/* May trace players/objects		*/
#define PFL_PUNT	52	/* May punt players 			*/
#define PFL_EFLAGEDIT	53	/* Can use eflag editor			*/
#define PFL_SFLAGEDIT	54	/* Can use sflag editor			*/
#define PFL_IGNORENOAT	55	/* Can override 'NoAt' lflag		*/
#define PFL_TIMEOUT	56	/* Can timeout users			*/
#define PFL_REBOOT	57	/* Can reboot the MUD			*/
#define PFL_TEXTRAW	58	/* Can use TextRaw			*/
#define PFL_PZAP	59	/* Can PZap (pretend Zap)		*/
#define PFL_SIGNAL	60	/* Can call SIGNAL			*/
#define PFL_BURN	61	/* Can burn other players		*/
#define PFL_SEEUSER	62	/* Can see username from Ident		*/
#define PFL_CANLOG	63	/* Can write to the Mudlog		*/
#define PFL_SIC		64	/* Can sic mobiles on players		*/
#define PFL_FLIST	65	/* Can use the ForgetList command	*/
#define	PFL_NFLAGEDIT	66	/* Can edit Nflags			*/
#define	PFL_SETTIME	67	/* Can set the climate time		*/
#define PFL_CHATMOD	68	/* Chat line moderator			*/
#define PFL_SOCKET	69	/* Can view/edit socket information	*/
#define PFL_IDLE	70	/* Pretending to be idle		*/
#define PFL_SEEANON	71	/* Can see people on the anon channel	*/
#define PFL_NOTIMEOUT	72	/* Cannot be tout'd			*/
#define PFL_NOECHO	73	/* Will not see echo's, echoall's, etc	*/
#define PFL_SEEIDLE	74	/* Can see real idle times		*/
#define PFL_VIEWCOM	75	/* Can see player input handlers	*/
#define PFL_SEESOCKET	76	/* Can see socket activity		*/
#define PFL_BAN		77	/* Can use the BAN command		*/
#define	PFL_PUNTALL	78	/* Can use the PUNTALL command		*/
#define	PFL_PROBATION	79	/* Can use the probation command	*/
#define	PFL_IFLAGEDIT	80	/* Can edit IFLAGS			*/

#define PFL_MAX		96	/* Maximum Pflag			*/

#endif
