#ifndef _GLOBAL_H
#define _GLOBAL_H

jmp_buf to_main_loop;

Location   *room_data;
Object     *objects;
UBLOCK_REC *ublock;

ZONE	*zoname;
char	**verbtxt;
char	*txt1;
char	*txt2;
int	levels[LVL_WIZARD + 1];
char	strbuf[MAX_COM_LEN];
char	wordbuf[MAX_COM_LEN];
char	item1[MAX_COM_LEN];
char	item2[MAX_COM_LEN];

char			*progname;
char			*data_dir = DATA_DIR;
char			my_hostname[MAXHOSTNAMELEN];
struct hostent		*my_hostent;
struct sockaddr_in      s_in;
char			**envp;
int			pid;

PLAYER_REC      *players;
WORLD_REC       the_world_rec;
WORLD_REC	*the_world = &the_world_rec;
CLIMATE_REC	the_climate_rec;
CLIMATE_REC	*the_climate = &the_climate_rec;

int	    mynum;          /* current player slot-number */
int         real_mynum;     /* real mynum if mynum is fake due to aliasing */
int         quit_list;      /* real mynum of player to quit */
PLAYER_REC *cur_player;     /* Current player info. */
UBLOCK_REC *cur_ublock;     /* Current ublock info. */


int	max_players = 32;
int     num_const_chars;
int	numchars;       /* Number of players + mobiles */
int     char_array_len;

int	num_const_obs;  /* Number of constant (not created in-game) objects */
int	numobs;		/* Number of objects in the game */
int     obj_array_len;

int     numzon;		/* Number of zones in the world		*/
int     num_const_zon;
int     zon_array_len;

int     num_const_locs;
int	numloc;		/* Number of locations */
int     loc_array_len;

long int   id_counter;  /* Next ID number to be given to a wiz-creation */
int_table  id_table;    /* Lookup table for [ID numbers -> game indexes] */

int	*verbnum;
int	ob1;
int	ob2;
int	pl1;
int	pl2;
int	pptr;		/* The parameter pointer		*/
int	prep;

int	stp;
int	verbcode;
time_t	next_event;     /* check mud.c */
time_t	last_reset;     /* Last reset time */
time_t	global_clock;
time_t  last_startup;   /* Last startup time */
time_t  last_healall;   /* Last healall */
time_t  last_autosave;  /* Autosave counter */
int     numresets;      /* Number of resets */
int     numreboots;     /* Number of reboots */
int     numcrashes;     /* Number of crashes */
long    tourn_prize;    /* Tournament Prize */
Boolean	breset;
long	qdone;
Boolean	norun;

int	DebugMode;
int	DebugPlr;

#endif
