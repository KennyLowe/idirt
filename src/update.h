#ifndef _UPDATE_H
#define _UPDATE_H

typedef struct _update_index {
  long  qdone;
  int   mobnum;
  int   objnum;
  int	mudzones;
  int	mudlocs;	
  int	mudobjs;
  int	mudchars;
} UPDATE_IDX_REC;

typedef struct _update_mobiles {
  int   num;
  int   loc;
  int   str;
  int	score;
} UPDATE_MOBILE_REC;

typedef struct _update_objects {
  int   num;
  int   loc;
  int   state;
  int   carrflg;
  int   flags_u;
  int   flags_h;
  int   flags_l;
} UPDATE_OBJECT_REC;

void	run_update(void);
void	update_world(int num);
int	check_update(int num);
int	save_mobiles(void);
int	save_objects(void);
void	read_mobiles(int mob, int fnum);
void	read_objects(int obj, int fnum);

#endif
