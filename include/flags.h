#ifndef IDIRT_FLAGS_H
#define IDIRT_FLAGS_H

void	show_bits(int *bits, int n, const char **table);
Boolean	dump_pflags(void);
void	xshow_lflags(int loc);
void	pflagscom(void);
void	maskcom(void);
void	lflagscom(void);
void	mflagscom(void);
void	sflagscom(void);
void	eflagscom(void);
void	nflagscom(void);
void	set_xpflags(int y,PFLAGS *p, PFLAGS *m);

#endif
