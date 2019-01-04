/**************************************************************** 
 * iDiRT 1.x							*
 * Configuration File						*
 ****************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

#include "logconfig.h"

#define MASTERUSER      "YourName"	/* Name of the MASTER character */
#define UNVEIL_PASS     "Password"	/* The UNVEIL-password.		*/

#define PORT            6715		/* Port to use			*/

#define	DFL_PROMPT	"&+b:&+W> &*"	/* Default prompt for players	*/
#define	MUD_NAME	"iDiRT"		/* Name of the MUD		*/

/* The absolute maximum number of locations/objects/mobiles (permanent +
 * all in-game wizard-made) you want in the game at any one time:
 */
#define GLOBAL_MAX_MOBS 10000
#define GLOBAL_MAX_LOCS 10000
#define GLOBAL_MAX_OBJS 10000

/********************************
 * Some in game defines: 	*
 ********************************/

/* Define this if you want to use the sliding scale value for objects
 * that depends upon the number of users online. 
 */
#define USE_TSCALE

/* Define this if you want to be able to use the Zopen utilities for
 * the iDiRT world files.
 */
/* #define USE_ZOPEN */

/* Define this if you want quests required to become a Wizard.
 */
/* #define USE_QUESTS */

/* The following define is the text that is displayed by default when the 
 * MUD is shutdown (or crashed)
 */
#define SHUTDOWN_MSG "iDiRT is currently unavailable, please try again later."

/* Maximum armor level that player may have. */
#define MAXARMOR 50

/* Maximum amount of time (in seconds) that a mortal can be idle before the
 * MUD disconnects him automatically. Wizards will timeout if they are idle
 * IDLE_MAX + 1 hour. Set to 0 to disable auto-timeout.
 */
#define IDLE_MAX 3600		/* 1 hour */

/* The minimum amount of time (in seconds) that a person can be idle before 
 * the reset stone can be pressed without him being there.
 */
#define RESET_IDLE 900		/* 15 minutes */ 

/* The minimum amount of time that can pass between 'healall' commands. 
 * In the healallcom() code, if the user is above the Wizard level, he/she 
 * can healall regardless of the last time it was done.
 */
#define HEALALL_TIME 0		/* Disabled */

/* If you want users to be able to see the last time they logged in when 
 * they login, define SHOW_LAST_LOGIN.
 */
#define SHOW_LAST_LOGIN

/* Amount of time to pass between autosaves (in seconds). */
#define AUTOSAVE_TIME	600	/* 10 minutes */

/* Define to use the Ident protocol or not */
/*#define USE_IDENT*/

/**********************************
 * Some system dependent defines: *
 **********************************/

/* If you get a linker error complaining that bzero and bcopy don't exist
 * define this.
 */
/* #define NO_BCOPY */

/* If the two routines strcasecmp() and strncasecmp() is missing from your
 * system, define SYS_EQBUG.
 */
/* #define SYS_EQBUG */

/* If variable number of args is implemented, define VARGS.
 */
#define VARGS

/* If your system doesn't have the random() function, define SYS_NO_RANDOM
 */
/* #define SYS_NO_RANDOM */

/* If we are on a Hewlett Packard HP/UX machine, define SYS_HP_UX
 */
/* #define SYS_HP_UX */

/* If the prototypes for inet_ntoa in the systems include files
 * are erroneous, define SYS_INET_NTOA_BUG
 */
/* #define SYS_INET_NTOA_BUG */

/* If you have a Linux system, this defines a signal that is missing.
 * GCC should automatically define your operating system type, and
 * you shouldn't have to edit this define.
 */
#ifdef _LINUX_
#ifndef SIGSYS
#define SIGSYS SIGUSR2
#endif
#endif

#endif
