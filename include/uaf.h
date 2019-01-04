#ifndef _UAF_H
#define _UAF_H

void	get_gender(char *gen);
void	pers2player(PERSONA *d,int plr);
void	player2pers(PERSONA *d,time_t *last_on,int plr);
void	get_gender(char *gen);
void	saveother(void);
void	saveme(void);
void	saveallcom(void);
Boolean	finduaf(char *name,PERSONA *d,int fd);
Boolean	getuaf(char *name,PERSONA *d);
void	putuaf(PERSONA *d);
void	deluaf(char *name);
Boolean	getuafinfo(char *name);
Boolean	findsetins(char *name, SETIN_REC *s, int fd);
Boolean	getsetins(char *name, SETIN_REC *s );
void	putsetins(char *name, SETIN_REC *s);
void	fetchprmpt(int plr);
char	*build_setin(char *b, char *s, char *n, char *d, char *v);
char	*ipname(int plr);
int	make_kd_ratio(int kill, int death);

#endif
