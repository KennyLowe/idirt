#ifndef _PARSE_H
#define _PARSE_H

int	gamecom(const char *str,Boolean savecom);
char	*getreinput(char *b);
int	brkword(void);
int	my_brkword(void);
int	chkverb(void);
int	chklist(const char *word, const char *lista[], int listb[]);
int	Match(const char *x, const char *y);
void	doaction(int n);
void	quit_game(void);
void	erreval(void);
Boolean	parse_2(int vb);
int	findprep(char *t);
int	prmmod(int p);
int	do_tables(int a);

#endif
