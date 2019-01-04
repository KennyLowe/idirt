#ifndef _ZONES_H
#define _ZONES_H

int	get_zone_by_name(char *zname);
int	get_wizzone_by_name(char *name);
int	loc2zone(int loc);
int	findzone(int loc, char *str);
int	getlocid(int z,int off);
int	getlocnum(char *zname,int off);

void	reset_zone(int z, time_t *now, int *d_locs, int *d_mobs, int *d_objs,
                                       int *r_locs, int *r_mobs, int *r_objs);

char	*wiz_loc_filename(char *buff, char *name);
char	*wiz_mob_filename(char *buff, char *name);
char	*wiz_obj_filename(char *buff, char *name);

int	load_zone(char *name, int *nlocs, int *nlocs_f, int *nmobs, int *nmobs_f,
	          int *nobjs, int *nobjs_f);

void	zonescom(void);
void	locationscom(void);

#endif
