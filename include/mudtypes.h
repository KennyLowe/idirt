#ifndef _MUDTYPES_H
#define _MUDTYPES_H

typedef	LongInt	 LFLAGS;
typedef	DLongInt OFLAGS;
typedef	DLongInt PFLAGS;
typedef	LongInt  SFLAGS;
typedef	LongInt  MFLAGS;
typedef	long int QFLAGS;
typedef long int NFLAGS;
typedef long int EFLAGS;

#define TITLE_LEN	300	/* Max. length of a players title */
#define PROMPT_LEN	30	/* Max. length of the prompt */
#define PNAME_LEN	12	/* Max. length of a player-name */
#define MNAME_LEN	16	/* Max length, mobile-name */
#define ONAME_LEN	32	/* Max length, object-name */
#define SETIN_MAX	80	/* Max length, travel-message */
#define TTY_LEN		64	/* Max length, name of players host */
#define MSG_LEN		240	/* Max length of a message */
#define PASSWD_LEN	16	/* Max chars of passwd + 1 */
#define MAX_COM_LEN	300	/* Max chars in a command line */
#define NUM_CONDS	6
#define NUM_ACTS	4

typedef struct _a_inp_h {
  struct _a_inp_h *next;
  void            (*inp_handler)(char *input_string);
} INP_HANDLER;

/* Setin struct
 */
typedef struct _SETIN_STRUCT {
  char	prompt[PROMPT_LEN+1];
  char	setin[SETIN_MAX+1];
  char	setout[SETIN_MAX+1];
  char	setmin[SETIN_MAX+1];
  char	setmout[SETIN_MAX+1];
  char	setvin[SETIN_MAX+1];
  char	setvout[SETIN_MAX+1];
  char	setqin[SETIN_MAX+1];
  char	setqout[SETIN_MAX+1];
  char	setsit[SETIN_MAX+1];
  char	setstand[SETIN_MAX+1];
  char	setsum[SETIN_MAX+1];
  char	setsumin[SETIN_MAX+1];
  char	setsumout[SETIN_MAX+1];
} SETIN_STRUCT;


/* A setin-file entry.
 */
typedef struct _SETIN_REC {
  char	name[PNAME_LEN+1];
  char	prompt[PROMPT_LEN+1];
  char	setin[SETIN_MAX+1];
  char	setout[SETIN_MAX+1];
  char	setmin[SETIN_MAX+1];
  char	setmout[SETIN_MAX+1];
  char	setvin[SETIN_MAX+1];
  char	setvout[SETIN_MAX+1];
  char	setqin[SETIN_MAX+1];
  char	setqout[SETIN_MAX+1];
  char	setsit[SETIN_MAX+1];
  char	setstand[SETIN_MAX+1];
  char	setsum[SETIN_MAX+1];
  char	setsumin[SETIN_MAX+1];
  char	setsumout[SETIN_MAX+1];
} SETIN_REC;

/* iDiRT Message System */
typedef struct _MSGIDX {
  long		offset;
  short		delete;
} MSGIDX;

typedef struct _MAILMSG {
  char		from[PNAME_LEN+1];
  char		subject[MAX_COM_LEN];
  long		date;
  char		*msgtxt;
} MAILMSG;

typedef struct _MAILER {
  void  	(*old_handler)(char *str);
  char  	old_prompt[PROMPT_LEN+40];

  FILE  	*mailbox;
  FILE  	*output;
  MSGIDX	*msgidx;  
  MAILMSG	message;

  char		sendto[PNAME_LEN+1];
  char		buffer[256];
  char		mailboxname[100];
  char		outputname[100];
  int		lastmsg;
} MAILER;

/* Pager Structure */
typedef struct _PAGER {
  FILE          *file;
  INP_HANDLER   *old_handler;
  char          prompt[PROMPT_LEN+40];
  int           len;
} PAGER;

/* Spell Duration Structure */
typedef struct _SPELL_DURATION {
  int		spell;
  int		duration;
  int		tmp;
  struct	_SPELL_DURATION	*next;
} SPELL_DURATION;

/* Trace structure */
typedef struct _TRACE {
  int		trace;
  Boolean	is_obj;
  int		loc;
  int		oloc;
  int		carry;
} TRACE;

/* A PERSONA as described in the uaf_rand file 
 */
typedef struct _PERSONA {
  char		p_name[PNAME_LEN+1];
  char		p_title[TITLE_LEN+1];
  char		p_passwd[PASSWD_LEN];
  long int	p_home;
  int		p_score;
  int		p_strength;
  int		p_damage;
  int		p_armor;
  SFLAGS	p_sflags;
  PFLAGS	p_pflags;
  PFLAGS	p_mask;
  QFLAGS	p_quests;
  NFLAGS	p_nflags;
  EFLAGS	p_eflags;
  int		p_lang;
  int		p_vlevel;
  int		p_level;
  time_t	p_last_on;
  int		p_wimpy;
  long int	p_id;
  char		p_last_host[MAXHOSTNAMELEN];
  int		p_magic;
  int		p_channel;
  int		p_killed;
  int		p_died;  
  int           p_pager;  
  char		p_usrname[MAXHOSTNAMELEN+20];
} PERSONA;

/* Zone table entry 
 */
typedef struct _ZONE {
  char		*z_name;	/* Zone name */

  int		maxlocs;
  int		maxmobs;
  int		maxobjs;

  int_set	locations;
  int_set	mobiles;
  int_set	objects;

  Boolean	temporary;	/* Real zone, or one to destruct on reset? */
} ZONE;

/* Location data in the rooms arrray as read from locations file 
 */
typedef struct {
  int		r_exit[NEXITS];
  LFLAGS	r_flags;
  char		*r_short;
  char		*r_long;
  long int	id;
  int_set	objects;	/* Set of objects in this loc. */
  int_set	mobiles;	/* Set of players/mobiles in this loc. */
  int_set	exits_to_me;	/* Set of locations that have exits to this */
  Boolean	temporary;	/* Can it be destructed on reset ? */
  Boolean	touched;	/* Has any exit been changed since last reset?*/
  int		zone;		/* To which zone do we belong ? */

/* Reset data */
  long int	r_exit_reset[NEXITS];
  LFLAGS	r_flags_reset;
} Location;

/* An object
 */
typedef struct {

/* Unalterable, no need to reset on a reset: */
  char		*oname;		/* objects name */
  char		*oaltname;	/* objects alternate name */
  char		*odesc[4];	/* descriptions for each state  */
  int		omaxstate;	/* max state a wiz can SET an obj to */
  long		oexamine;	/* ptr in file where examine text is found */
  char		*oexam_text;	/* ptr to exam-txt for in-game created objs */
  long int	id;		/* unique ID, needed for new saved objects */
  Boolean	temporary;	/* Part of a wizards permenent zone? */
  int		linked;		/* Which object, if any, is this linked to? */
  int		zone;		/* To which zone do we belong? */
  int		onum;		/* number for the code to test so that cloned
				   objects can behave like the originals. */
/* Alterable, needs saved reset values too: */
  int		ovalue;		/* base value */
  int		osize;
  int		oloc;
  int		ovis;		/* the objects visibility level */
  int		odamage;
  int		oarmor;
  int		ocarrf;
  int		ostate;
  OFLAGS	oflags;

  long int	oloc_reset;
  int		osize_reset;
  int		ovalue_reset;
  int		ovis_reset;
  int		odamage_reset;
  int		oarmor_reset;
  int		ocarrf_reset;
  int		ostate_reset;
  OFLAGS	oflags_reset;

  int_set	objects;	/* set of objects inside this object */
} Object;

/* A record describing a player or mobile in the world. 
 */
typedef struct {
  char		pname[MNAME_LEN+1];
  int		ploc;
  long int	phome;		/* players start-loc. and home */
  int		pdam;		/* Damage */
  int		parmor;
  int		pagg;		/* Agression */
  int		pspeed;		/* Speed */
  int		pcarry;		/* Carrying capacity */
  int		pstr;
  int		pvis;
  SFLAGS	psflags;
  PFLAGS	pflags;
  PFLAGS	pmask;
  MFLAGS	pmflags;
  QFLAGS	pquests;
  int		plev;
  int		pweapon;
  int		psitting;
  int		phelping;
  int		pfighting;
  unsigned int	pscore;
  char		*pftxt;		/* Mobile's one-line description */
  char		*p_exam;	/* exam-text for mobiles (may also be in DESC)*/
  int		pnum;		/* player/mobile number */
  int_set	objects;	/* set of objects carried by this character */
  long int	id;		/* Unique ID */
  Boolean	temporary;	/* Mobile part of a wizards permanent stuff? */
  int		zone;		/* To which zone do we belong ? */
  int		pwimpy;
  NFLAGS	pnflags;		 /* Language Flags */
  int		planguage;		 /* Language Selected */
  EFLAGS	peflags;		 /* Spell Flags */

/* Reset data for mobiles: */

  char		*pname_reset;
  long int	ploc_reset;
  int		pstr_reset;
  int		pvis_reset;
  SFLAGS	psflags_reset;
  PFLAGS	pflags_reset;
  MFLAGS	pmflags_reset;
  NFLAGS	pnflags_reset;
  EFLAGS	peflags_reset;
  int		plev_reset;
  int		pagg_reset;	/* Agression */
  int		pspeed_reset;	/* Speed */
  int		pdam_reset;
  int		parmor_reset;
  int		pwimpy_reset;
} UBLOCK_REC;

typedef UBLOCK_REC Mobile;
typedef UBLOCK_REC Player;

/* Player data that are not shared by mobiles.
 */
typedef struct _a_player {
  time_t	last_cmd;
  time_t	rlast_cmd;
  time_t	logged_on;
  int		fil_des;
  FILE		*stream;
  struct sockaddr_in	sin;
  int		sin_len;
  INP_HANDLER	*inp_handler;
  char		*inp_buf_p;
  char		*inp_buf_end;
  char		*sock_buf_p;
  char		*sock_buf_end;
  void		*writer;	/* Current writer */
  int		no_logins;	/* No of failed passwd */
  int		work;		/* General work area for use by code */
  char		work2[64];
  Boolean	no_echo;	/* True if telnet should turn off echo */
  Boolean	isawiz;		/* Is this player a system's wizard? */
  Boolean	ismonitored;	/* Is this player being monitored? */
  Boolean	iamon;		/* Am I on? Init to false. */
  Boolean	in_pbfr;	/* Are we busy inside pbfr()? */
  Boolean	aliased;	/* Are we aliased? */
  int		aliasto;	/* Who are we aliased to? */
  int		quit_next;	/* Next on quit_list, -2 if not in list */
  int		polymorphed;	/* Polymorphed for how many more turns? */
  int		pretend;	/* Who we pretend to be if aliased or poly */
  int		isforce;	/* Set to -1 if not force, else pl. indx. */
  int		i_follow;	/* Set to -1 if not following anyone. */
  int		me_ivct;	/* Invisible for how many cmds ? */

  int		snooped;	/* How many are snooping us? */
  int		snooptarget;	/* Who if any are we snooping? */
  int		asmortal;	/* What level are we pretending to have? */

  char		passwd[PASSWD_LEN];
  char		cprompt[PROMPT_LEN+30];
  char		prompt[PROMPT_LEN+1];
  char		setin[SETIN_MAX+1];
  char		setout[SETIN_MAX+1];
  char		setmin[SETIN_MAX+1];
  char		setmout[SETIN_MAX+1];
  char		setvin[SETIN_MAX+1];
  char		setvout[SETIN_MAX+1];
  char		setqin[SETIN_MAX+1];
  char		setqout[SETIN_MAX+1];
  char		setsit[SETIN_MAX+1];
  char		setstand[SETIN_MAX+1];
  char		setsum[SETIN_MAX+1];
  char		setsumin[SETIN_MAX+1];
  char		setsumout[SETIN_MAX+1];

  char		o_setin[SETIN_MAX+1];	/* Duplicate sets for aliased player */
  char		o_setout[SETIN_MAX+1];
  char		o_prompt[PROMPT_LEN+1];

  char		wd_her[MNAME_LEN+1];
  char		wd_him[MNAME_LEN+1];
  char		*wd_them;
  char		*wd_it;

  char		ptitle[TITLE_LEN+1];
  char		hostname[MAXHOSTNAMELEN];
  char		prev_com[MAX_COM_LEN];
  char		inp_buffer[MAX_COM_LEN];
  char		sock_buffer[MAX_COM_LEN]; 

  MAILER	Mailer;			/* Mail structure		*/
  Boolean	inmailer;	 	/* Is the user in the mailer?	*/

  int		pmagic;			/* Magic points			*/
  int		pchannel;		/* Chat channel			*/
  int		pkilled;		/* Number of mobiles killed	*/
  int		pdied;			/* Number of times died		*/
  int		forget[10];		/* Forget list			*/
  int		pconverse;		/* Player conversing with	*/
  SPELL_DURATION *duration;		/* Spell Duration (Current Ptr)	*/

  PAGER         pager;
  Boolean       inpager;

  FILE          *log;
  Boolean       logged;

  Boolean	newplr;
  char		usrname[MAXHOSTNAMELEN+20];
  Boolean	is_conn;		/* Player is connected		*/
  Boolean	host_ban;		/* Host is banned		*/
  Boolean	user_ban;		/* User/Who at login ban	*/
  int		overwrite;
  char		awaymsg[40];
  int		replyplr;
  TRACE		Trace;
} PLAYER_REC;

/* Weather and climate structure
 */
typedef struct _climate {
  int		weather;
  Boolean	daytime;
  int		time;
  int		day;
  int		month;
  int		season;
} CLIMATE_REC;
  
/* The world. Contains some global variables 
 */
typedef struct _a_world {
  int		w_msg_low;
  int		w_msg_high;
  int		w_lock;
  int		w_mob_stop;
  int		w_peace;
  int		w_max_users;
  int		w_tournament;
  PFLAGS	w_pflags[8];
  PFLAGS	w_mask[8];
} WORLD_REC;

#endif
