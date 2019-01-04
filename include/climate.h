#ifndef _NEWCLIMATE_H
#define _NEWCLIMATE_H

typedef enum { 
clear, showers, rain, stormy, hailing, drizzle, snow, blizzard
} weather_type;

typedef	enum { 
winter, spring, summer, fall 
} season_type;

void	weathercom(void);
int	modify_weather(int loc);
char	*char_func(int plr, int arg, char *msg);
void	adjust_weather(weather_type new);
void	set_weather(weather_type new);
void	show_weather(void);
void	change_weather(void);
char	*weather_desc(weather_type type);
char	*climate_desc(weather_type type);

void	move_time(void);
void	which_season(void);
void	_set_time(int month, int day, int hr, int min);
void	settimecom(void);
void	climate_split_time(int time, int *min, int *hrs);
Boolean	climate_day(void);
char	*climate_time (void);
char	*season_name(season_type type);
void	climatetime(void);

#endif
