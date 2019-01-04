#ifndef _MOBILE_H
#define _MOBILE_H

char	*make_prompt(char *b, char *s, char *h, char *c, 
		     char *l, char *n, char *m);

int	player_damage(int player);
int	player_armor(int player);
Boolean	check_armor(int plx, int obj);

void	list_people(void);
void	move_mobiles(void);
void	onlook(void);
void	chkfight(int mon);
void	consid_move(int mon);
void	do_follow (void);
void	check_follow (void);
void	dorune(int plx);
Boolean	dragget(void);
void	helpchkr(int plx);
void	movemob(int x);
void	stopcom(void);
void	startcom(void);
Boolean	stealstuff(int m);
Boolean	shiftstuff(int m);
char	*xname(char *n);
void	setname(int plx);
Boolean	see_player(int pla, int plb);
Boolean	seeplayer(int plx);
char	*see_name(int pla, int plb);
int	fpbn(char *name);
int	find_player_by_name(char *name);
int	check_if_player_exists(char *name);
int	find_player(char *name,PERSONA *p,Boolean *f);
int	find_mobile_by_id(long int id);
int	alive(int i);
int	wlevel(int lev);
Boolean	do_okay_l( int p, int v, Boolean c );
Boolean	do_okay( int p, int v, int prot_flag );
void	setpsex(int chr, Boolean v);
void	setploc(int plr, int room);
int	ptothlp(int pl);
int	maxstrength(int p);
int	maxmagic(int p);
void	destroy_mobile(int mob);
char	*make_title(char *title, char *name);
char	*make_rank(int player);
char	*std_title(int level, Boolean sex);
Boolean	reset_mobile(int mobile);
void	p_crapup(int player, char *str, int flags);
void	crapup(char *str,int flags);
void	xcrapup(char *str,Boolean saveflag);
void	death_msg(int plr);
void	loseme(Boolean saveflag);
char	*lev2s(char *b,int lvl);
int	tscale(void);
Boolean	chkdumb(void);
Boolean	chkcrip(void);
Boolean	chksitting(void);
void	calib_player(int pl);
void	calibme(void);
int	levelof(int score,int lev);
Boolean	check_setin(char *s, Boolean d, Boolean v);
Boolean	check_nooracle(int plx);
Boolean	check_busy(int plx);
Boolean	check_coding(int plx);
Boolean	check_away(int plx);
Boolean	check_forget(int p1, int p2);
void	wipe_forget(int plr);
char	*build_prompt(int plx);
int	vicf2(int fl,int chance);
int	vichere(void);
int	vicbase(void);
void	jumpcom(void);
void	special_events(int player);
void	regenerate(int plr);
void	set_quest(int plx, int quest);
void	dopouncie(int plx);
void	set_msg (char *b, Boolean dir_ok, Boolean sum);
char	*str_color (int plr);
char	*mag_color (int plr);

/* Values for crapup */
#define CRAP_SAVE    0x01
#define CRAP_RETURN  0x02
#define CRAP_UNALIAS 0x04

#define SAVE_ME   CRAP_SAVE
#define NO_SAVE   0

/* Values for vicf2 */
#define     SPELL_PEACEFUL 0
#define     SPELL_VIOLENT  1
#define     SPELL_REFLECTS 2

/* Values for special_events() */
#define     SP_ALL (-1)

#define	fpbns(N)  find_player_by_name(N)

#endif
