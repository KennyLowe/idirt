#ifndef _FIGHT_H
#define _FIGHT_H

Boolean	wound_player (int attacker, int victim, int damage, int hit_type);
int	dambyitem(int pl, int it);
void	wieldcom(void);
void	hit_player(int attacker,int victim,int weapon);
void	killcom(void);
void	bloodrcv(int *array, int isme);
void	breakitem(int x);
Boolean	testpeace(int plr);
void	woundmn(int mon, int am);
void	fleecom(void);
void	mhitplayer(int mon);
void	combatmessage(int victim, int attacker, int wpn, int ddn);
void	setpfighting(int x, int y);

#endif
