/****************************************************************
 * MUDLOG Configuration File					*
 * This include file tells MUDLOG what extra things to log and	*
 * what extra things not to log.				*
 ****************************************************************/

#ifndef _LOGCONFIG_H
#define _LOGCONFIG_H

/* COMPACT_LOG: Define this to make the syslog more compact.
 */
#define COMPACT_LOG

/* LOG_GETHOST: Log error in gethostbyaddr() if a hostname could not be
 * resolved
 */
/* #define LOG_GETHOST */

/* LOG_FDPEND: Log exception pendings in file descriptors. 
 */
/* #define LOG_FDPEND */

/* LOG_MFLAG: Log changes in MFLAGS.
 */
#define LOG_MFLAG

/* LOG_LFLAG: Log changes in LFLAGS.
 */
/* #define LOG_LFLAG */

/* LOG_CLONE_ROOM: Log location cloning.
 */
/* #define LOG_CLONE_ROOM */

/* LOG_CLONE_MOBILE: Log mobile cloning.
 */
#define LOG_CLONE_MOBILE

/* LOG_CLONE_OBJECT: Log object cloning.
 */
#define LOG_CLONE_OBJECT

/* LOG_DEST_ROOM: Log location destructing.
 */
#define LOG_DEST_ROOM

/* LOG_DEST_MOBILE: Log mobile destructing.
 */
#define LOG_DEST_MOBILE

/* LOG_DEST_OBJECT: Log object destructing.
 */
#define LOG_DEST_OBJECT

/* LOG_SNOOP: Log if snoop is used by less then LVL_DEFINE.
 */
/* #define LOG_SNOOP */

#endif


