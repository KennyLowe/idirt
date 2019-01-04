#ifndef _BOOTSTRAP_H
#define _BOOTSTRAP_H

int	bootstrap(void);
int	load_objects(int zone, FILE *f, int *num_ld, int *num_infile);
int	load_locations(int zone, FILE *f, int *num_ld, int *num_infile);
int	load_mobiles(int zone, FILE *f, int *num_ld, int *num_infile);
Boolean	save_id_counter(void);

#endif
