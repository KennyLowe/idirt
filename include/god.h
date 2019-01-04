#ifndef _GOD_H
#define _GOD_H

void	deletecom (void);
void	opengamecom (void);
void	shutdowncom (Boolean crash);
void	bresetcom (void);
void	idlecom (void);
void	seeidlecom (void);
void	socketcom (void);
void	togglesocket (void);
void	togglecoding (void);
void	wloadcom (void);
void	silentcom (void);
void	ploccom (void);
void	writelog (void);
void	bancom (char filename[100], int type);
void	probationcom (void);
void	levforcecom (void);
void	reboot_actions (void);
void	toggleseesocket (void);
void	forgetlist (void);
void	noruncom (void);

#endif
