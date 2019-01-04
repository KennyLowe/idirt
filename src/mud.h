#ifndef _MUD_H
#define _MUD_H

void	socketmsg(Boolean user, Boolean chkvis, char *format,...);
void	push_input_handler(void (*h)(char *str));
void	pop_input_handler(void);
void	replace_input_handler(void (*h)(char *str));
Boolean	ask_yesno (char *prompt);
void	ask_yesno_handler (char *answer);
int	find_free_player_slot(void);
int	find_pl_index(int fd);
void	xsetup_globals(int plx);
void	setup_globals(int plx);
Boolean	is_host_banned(char *hostname);
Boolean	is_login_banned(char *hostname);
Boolean	is_host_user_banned(char *hostname);
Boolean	is_host_silent(char *hostname);
Boolean	is_player_banned(char *name);
void	new_player(char usrname[20]);
void	get_command(char *cmd);
void	quit_player(void);
void	enter_vis(char *v);
void	do_motd(char *cont);
void	do_issue(char *cont);
Boolean	privileged_user (char *name);
Boolean	is_monitored(char *name);

void    get_pname1(char *name);
void    get_pname2(char *reply);
void    get_new_pass1(char *pass);
void    get_new_pass2(char *pass);
void    get_passwd1(char *pass);
void    kick_out_yn(char *answer);

void	check_files (void);
void	display_title (void);

#endif
