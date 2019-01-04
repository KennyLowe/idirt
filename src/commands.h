#ifndef _COMMANDS_H
#define _COMMANDS_H

/* commands.c */
void	oraclecom(char com[10]);
void	lookcom(void);
void	wherecom(void);
void	examcom(void);
void	sys_reset(void);
void	resetcom(int flags);
void	unveilcom(char *unv_pass);
void	posecom(void);
void	reviewcom(void);
void	scorecom(void);
void	sitcom(void);
void	standcom(int player);
void	pncom(void);
void	bugcom(void);
void	typocom(void);
void	helpcom(void);
void	help2(char *cont);
void	help3(char *cont);
void	help4(char *cont);
void	help5(char *cont);
void	help6(char *cont);
void	help7(char *cont);
int	showhelp(char *verb);
void	questcom(void);
void	infocom(void);
void	policycom(void);
void	globalcom(void);
int	find_pretender(int plx);
void	aliascom(void);
void	polymorph(int plx, int turns);
void	unalias(int pl);
void	unpolymorph(int pl);
void	becom(char *passwd);
void	dircom(void);
void	treasurecom(void);
void	levelscom(void);
void	promptcom(void);
void	followcom(void);
void	losecom(void);
void	togglecom(int flg, char on[80], char off[80]);
void	qdonecom(void);
void	beepcom(void);
void	togglefinger(void);
void	fingercom(void);
void	setpager(void);
void	optionscom(void);
void	klockcom(char *passwd);
void	comparecom(void);
void	mbullcom(void);
void	wbullcom(void);
void	abullcom(void);
void	ubullcom(void);
void	judgecom(void);
void	incom (Boolean inv);

#define RES_TEST  0x01  /* Perform check for pflag */

#endif
