#ifndef _BPRINTF_H
#define _BPRINTF_H

void	pbfr(void);
void	bflush(void);
void	snoopcom(void);
void	snoop_off(int plr);
void	print_buf(char *b,Boolean notself);

void	fix_color(char *dests, const char *srcs);
void	strip_color(char *dests, const char *srcs);

void	special_codes(char *dests, char *srcs);

void	bprintf(const char *format, ...);

void	file_pager(const char *filename);
void	quit_pager(void);
void	pager(char *c);

#endif


