#ifndef _EXTERN_H
#define _EXTERN_H

extern char     *pwait;
extern char     *qwait;
extern jmp_buf  to_main_loop;

extern char    *Exits[];
extern char    *exits[];

extern Location	   *room_data;
extern Object	   *objects;
extern UBLOCK_REC  *ublock;

extern ZONE	*zoname;
extern char	**verbtxt;
extern char	*txt1;
extern char	*txt2;
extern char	item1[];
extern char	item2[];
extern char	*TF[];
extern char	*OO[];
extern int	levels[LVL_WIZARD + 1];
extern char	strbuf[MAX_COM_LEN];
extern char	wordbuf[MAX_COM_LEN];

extern char     		*progname;
extern char			*data_dir;
extern char			my_hostname[MAXHOSTNAMELEN];
extern struct hostent		*my_hostent;
extern struct sockaddr_in	s_in;
extern char			**envp;
extern int			pid;

extern PLAYER_REC       *players;

extern WORLD_REC        the_world_rec;
extern WORLD_REC	*the_world;
extern CLIMATE_REC	the_climate_rec;
extern CLIMATE_REC	*the_climate;
 
extern int	   mynum;         /* current player slot-number */
extern int         real_mynum;    /* real mynum, see global.h */
extern int         quit_list;     /* List of players to quit */
extern PLAYER_REC *cur_player;    /* Current player info. */
extern UBLOCK_REC *cur_ublock;    /* Current ublock info. */

extern int	max_players;
extern int      num_const_chars;
extern int	numchars;       /* Number of players + mobiles */
extern int      char_array_len;

extern int	numobs;		/* Number of objects in the game */
extern int	num_const_obs;	/* Number of constant (not created in-game) */
extern int      obj_array_len;

extern int      numzon;		/* Number of zones in the world		*/
extern int      num_const_zon;
extern int      zon_array_len;

extern int	numloc;		/* Number of locations */
extern int      num_const_locs;
extern int      loc_array_len;

extern long int id_counter;
extern int_table  id_table;

extern int	*verbnum;
extern int	ob1;
extern int	ob2;
extern int	pl1;
extern int	pl2;
extern int	pptr;		/* The parameter pointer		*/
extern int	prep;

extern int	stp;
extern int	verbcode;
extern time_t   next_event;     /* check mud.c */
extern time_t   last_reset;
extern time_t   global_clock;
extern time_t   last_startup;
extern time_t   last_healall;
extern time_t   last_autosave;
extern int      numresets;
extern int      numreboots;
extern int      numcrashes;
extern long	qdone;
extern Boolean	breset;
extern Boolean	norun;

extern char *Pflags[];
extern char *Sflags[];
extern char *Mflags[];
extern char *Nflags[];
extern char *Eflags[];
extern char *Iflags[];
extern char *MLevels[]; /* Male mortal level names */
extern char *FLevels[]; /* Female mortal level names */
extern char *Quests[];

extern int  DebugMode;
extern int  DebugPlr;

#endif
