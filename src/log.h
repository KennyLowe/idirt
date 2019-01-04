#ifndef _LOG_H
#define _LOG_H

#include <stdarg.h>

int	open_logfile(char *logfile,Boolean clear_flag);
void	close_logfile(void);
void	progerror(char *name);

#ifdef VARGS
void	vmudlog( char *format, va_list pvar);
void	mudlog( char *format, ...);
#else
void mudlog();
#endif

void    open_plr_log(void);
void    close_plr_log(void);

#ifdef VARGS
void    vwrite_plr_log(char *format, va_list pvar);
void    write_plr_log(char *format, ...);
#else
void    write_plr_log();
#endif

#endif
