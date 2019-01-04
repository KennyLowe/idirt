#ifndef _OFLAGS_H
#define _OFLAGS_H

/* iDiRT
 * Object Flag Definitions (oflags)
 */

#define OFL_DESTROYED	0	/* Destroyed				*/
#define OFL_NOGET	1	/* Flannel				*/
#define OFL_OPENABLE	2	/* Can be opened (1=closed, 0=open)	*/
#define OFL_LOCKABLE	3	/* Can be locked (state 2 = locked)	*/
#define OFL_PUSHABLE	4	/* Set state to 0 when pushed		*/
#define OFL_PUSHTOGGLE	5	/* Toggle state when pushed		*/
#define OFL_FOOD	6	/* Can be eaten				*/
#define OFL_ARMOR	7	/* Can be worn as armor			*/
#define OFL_WEARABLE	8	/* Can be worn				*/
#define OFL_LIGHTABLE	9	/* Can light				*/
#define OFL_EXTINGUISH	10	/* Can extinguish			*/
#define OFL_KEY		11	/* Is a key				*/
#define OFL_GETFLIPS	12	/* Change to state 0 when taken		*/
#define OFL_LIT		13	/* Is lit				*/
#define OFL_CONTAINER	14	/* Is a container			*/
#define OFL_WEAPON	15	/* Is a weapon				*/
#define	OFL_REGENHEALTH	16	/* Regenerates Health Faster		*/
#define	OFL_REGENMANA	17	/* Regenerates Mana Faster		*/

#define OMFL_START	40	/* First Magical Object Flag		*/
#define OMFL_FIREBALL	40	/* Helps with the fireball spell        */
#define OMFL_MISSILE	41	/* Helps with the missile spell         */
#define OMFL_FROST	42	/* Helps with the frost spell           */
#define OMFL_SHOCK	43	/* Helps with the shock spell           */
#define OMFL_AID	44	/* Helps with the aid spell             */
#define OMFL_VTOUCH	45	/* Helps with the vtouch spell          */
#define OMFL_LIGHT	46	/* Helps with the light spell           */
#define OMFL_DAMAGE	47	/* Helps with the damage spell          */
#define OMFL_ARMOR	48	/* Helps with the armor spell           */
#define OMFL_BHANDS	49	/* Helps with the bhands spell          */
#define OMFL_BLUR	50	/* Helps with the blur spell            */
#define OMFL_ICESTORM	51	/* Helps with the icestorm spell        */
#define	OMFL_END	51	/* Last Magical Object Flag		*/

#define OFL_MAX		96	/* Maximum OFlag			*/

#endif
