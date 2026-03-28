#ifndef _COMM_H
#define _COMM_H

char	*make_com_text(char *b, const char *s, char *t, int send_plr, int recv_plr);
void	com_handler(const char *format, const char *linename, int lvl, int flg);
void	nolinecom(int lvl, int flg, const char *txt);
void	shoutcom(void);
void	saycom(void);
void	saytocom(void);
void	lsaycom(void);
void	tellcom(void);
void	gossipcom(void);
void	anoncom(void);
void	wishcom(void);
void	chatcom(void);
void	channelcom(void);
void	forgetcom(void);
void	langcom(void);
void	conversecom(void);
void	replycom(void);

#endif
