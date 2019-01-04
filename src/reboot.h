#ifndef _REBOOT_H
#define _REBOOT_H

/* The reboot record is read in first, and then the variable nwiz_zones
 * is used to find out how many times a read (char[50]) must occur for
 * the Wizard Zone names. After that is completed, RPLR_REC is read in
 * the number of times specified in nplayers.
 */

typedef struct _reboot_player_record {
  int		plx;
  int		fil_des;
  char		pname[PNAME_LEN+1];
  time_t	logged_on;
  time_t	last_cmd;
  time_t	rlast_cmd;
  int		ploc;
  int		pvis;
  int		pconv;
  char		hostname[MAXHOSTNAMELEN];
  char		usrname[MAXHOSTNAMELEN+20];
  int		forget[10];
  char		awaymsg[40];
  TRACE		tracedata;
} RPLR_REC;

typedef struct _reboot_record {
  char		version[20]; 
  int		main_socket;
  time_t	last_startup;
  int		numresets;
  int		numreboots;
  int		numcrashes;
  time_t	last_reset;
  int		climate_time;
  int		climate_day;
  int		climate_month;
  int		nwiz_zones;
  int		nplayers;
} REBOOT_REC;

/* Prototypes */
void	run_reboot (Boolean crash, Boolean will_update);
void	rebootcom (void);
void	updatecom (void);

#endif

