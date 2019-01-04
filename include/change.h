#ifndef _CHANGE_H
#define _CHANGE_H

void	changecom(void);
void	change_title(void);
void	change_desc(void);

void    ask_old_passwd(char *pass);
void    ask_new_passwd(char *pass);
void    ask_confirm_passwd(char *pass);

void    room_desc_handler(void *w, void *ad, int adlen);
void    mob_desc_handler(void *w, void *ad, int adlen);
void    obj_desc_handler(void *w, void *ad, int adlen);
void    player_desc_handler(void *w, void *ad, int adlen);

#endif
