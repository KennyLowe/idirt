#ifndef _LOG_H
#define _LOG_H

#include <stdarg.h>

int	open_logfile(const char *logfile, Boolean clear_flag);
void	close_logfile(void);
void	progerror(const char *name);

void	vmudlog(const char *format, va_list pvar);
void	mudlog(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

void    open_plr_log(void);
void    close_plr_log(void);

void    vwrite_plr_log(const char *format, va_list pvar);
void    write_plr_log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#endif
