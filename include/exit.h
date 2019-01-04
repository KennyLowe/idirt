/* iDiRT Exit Handler */

#ifndef	_EXIT_H
#define _EXIT_H

void	__exit(int status);
void	sig_exit(char *sig, int signal);
void	autosave(void);
void	debug(void);
void	signalcom(void);
#endif
