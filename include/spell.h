#ifndef _SPELL_H
#define _SPELL_H

void	spellscom(void);
void	spellcom(int spell);
Boolean	cast_spell(int caster, int victim, int verb);
int	mob_cast_spell(int caster);
Boolean	check_object(int plr, int flag);
void	send_magic_msg(int dest, int caster, int victim, char *msg, int type);
char	*make_magic_msg(char *b, char *s, char *c, char *v);

void	wipe_duration(int plr);
void	duration_end(int plr, int spell, int tmp);
Boolean	add_duration(int plr, int spell, int duration, int tmp);
Boolean	check_duration(int plr, int spell);
void	handle_duration(int plr);
void	push_duration (int plr, int spell, int duration, int tmp);

#endif
