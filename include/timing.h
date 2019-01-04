#ifndef _TIMING_H
#define _TIMING_H

Boolean	mud_open(time_t *next_ev, time_t *now);
char	*time2ascii(time_t t);
void	timecom(void);
void	eltime(void);
long	gametime(void);
void	uptimecom(void);
char	*sec_to_str(long int sec);
char	*sec_to_hhmmss(long int seconds);
time_t	round_to_min(time_t t);
char	*my_ctime(time_t *clock);
void	set_timer(void);
void	on_timer(void);
int	boot_hours(FILE *f,char *fname);
void	trace_handler (int plr);
void	check_idle (int plr);

#define	TIME_CURRENT           ((time_t)(0))
#define	TIME_NEVER             ((time_t)(-1))
#define	TIME_UNAVAIL           ((time_t)(-2))
#define	TIMER_INTERRUPT        2 /* How often do we do timer interrupts? */

#endif
