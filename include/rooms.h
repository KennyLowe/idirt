#ifndef _ROOMS_H
#define _ROOMS_H

/* Flags for lookin(); */
#define SHOW_LONG      0x01

char	*sdesc(int room);
char	*ldesc(int room);
int	getexit(int room, int ex);
void	setexit (int room, int dir, int dest);
int	get_rand_exit_dir(int room);
int	get_rand_exit(int room);
int	count_players(int loc,int min_lvl,int max_lvl,int flags);
void	gotocom(Boolean tiptoe);
int	exists(int room);
void	exitcom(void);
Boolean	roomdark(int room);
Boolean	r_isdark(int room, int plr);
Boolean	isdark(void);
void	teletrap(int newch);
Boolean	trapch(int loc);
void	lookin(int loc,int showfl);
char	*showname(int loc);
char	*xshowname(char *b, int loc);
char	*buildname(char *b, int loc);
int	find_loc_by_name(char *name);
int	findroomnum(char *w);
int	getroomnum(void);
int	find_loc_by_id(long int id);
Boolean	reset_location(int loc);
Boolean	chkroom(int loc, int plr);
int	check_light(int loc);
int	check_temp(int loc);

#define COUNT_PLAYERS 0x01
#define COUNT_MOBILES 0x02
#define INVERT_LEVELS 0x04

#endif
