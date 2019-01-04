#ifndef _PARSE_H
#define _PARSE_H

int	gamecom(char *str,Boolean savecom);
char	*getreinput(char *b);
int	brkword(void);
int	my_brkword(void);
int	chkverb(void);
int	chklist(char *word, char *lista[], int listb[]);
int	Match(char *x, char *y);
void	doaction(int n);
void	quit_game(void);
void	erreval(void);
Boolean	parse_2(int vb);
int	findprep(char *t);
int	prmmod(int p);
int	do_tables(int a);

#endif
