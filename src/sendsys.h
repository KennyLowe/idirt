#ifndef _SENDSYS_H
#define _SENDSYS_H

/* Special destinations for send message */
#define DEST_ALL  (-32767)
#define NOBODY    (-32766)    /* Value of x1 and x2 below if ignored */

/* Mode word for send_msg */
/*
 *  bits 00..05 contain pflag bit number.
 *  bits 06..07 contain pflag mode
 *                 00 - no pflag testing.
 *                 01 = send message if specified pflag is set.
 *                 10 = send message if specified pflag is not set.
 *                 11 = no pflag testing.
 *  bits 08..13 contain sflag bit number
 *  bits 14..15 contain sflag mode - as 6..7 but for sflags instead of pflags.
 *  bit  16        do not send message if quiet bit is set.
 *  bit  17        do not send message if recipient is crippled.
 *  bit  18        do not send message if recipient is dumb.
 *  bit  19        do not send message if recipient is deaf.
 *  bit  20        send message only if recipient is outdoors.
 *  bit  21	   do not send message if recipient is blind.
 *  bit  31        negate the whole test.
 */

/* Mode bits for send_msg */
#define MODE_NEG	0x80000000
#define MODE_NOBLIND      0x200000
#define MODE_OUTDOORS     0x100000
#define MODE_NODEAF        0x80000
#define MODE_NODUMB        0x40000
#define MODE_NOCRIP        0x20000
#define MODE_QUIET         0x10000
#define MODE_NSFLAG         0x8000
#define MODE_SFLAG          0x4000
#define MODE_NPFLAG           0x80
#define MODE_PFLAG            0x40
#define MODE_FLAGS            0x3f

#define MODE_P 0	/* Pflag bit number in bits 0..5  (6 bits) */
#define MODE_S 8	/* Sflag bit number in bits 8..13 (6 bits) */

#define MS(x) ((x)<<MODE_S)
#define MP(x) ((x)<<MODE_P)

/* Bit manipulation for Languages */
#define MODE_NLANG      0x80
#define MODE_LANG       0x40
#define MODE_L          0
#define ML(x)           ((x)<<MODE_L)

void broad(char *mesg);
void sillycom(char *txt);
void sillytp(int per, char *msg);

void send_g_msg(int destination,
		char *func(int plx, int arg, char *t),
		int  arg,
		char *text);

#ifdef VARGS
void sendf(int destination,char *format,...);

void gsendf(int destination,
	    char *func(int plx, int arg, char *text),
	    int arg,
	    char *format,...);

void send_msg(int destination,
	      int mode,
	      int min,
	      int max,
	      int x1,
	      int x2,
	      char *format,...);
#else
void sendf();  /* sendf(destination,format,arg1,arg2,...); */
void gsendf(); /* gsendf(destination,func,args,format,arg1,arg2,...); */
void send_msg(); /* send_msg(destintaion,mode,min,max,x1,x2,format,arg1...); */
#endif

void lsend_msg(int destination,
	       int lang,
	       int mode,
	       int min,
	       int max,
	       int x1,
	       int x2,
	       char *format,...);

#endif
