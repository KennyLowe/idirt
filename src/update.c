
#include <unistd.h>
#include <stdlib.h>
#include "kernel.h"
#include "objsys.h"
#include "mobile.h"
#include "update.h"
#include "log.h"

void
run_update (void)
{
  UPDATE_IDX_REC update_idx;
  char fn[25];
  FILE *fp;

  sprintf (fn, "update.data.%d", getpid ());
  if ((fp = fopen (fn, "wb")) == NULL) {
    return;
  }
  update_idx.qdone = qdone;
  update_idx.mudzones = num_const_zon;
  update_idx.mudlocs = num_const_locs;
  update_idx.mudobjs = num_const_obs;
  update_idx.mudchars = num_const_chars - max_players;
  update_idx.mobnum = save_mobiles ();
  update_idx.objnum = save_objects ();

  fwrite (&update_idx, sizeof (update_idx), 1, fp);
  fclose (fp);
}

void
update_world (int num)
{
  UPDATE_IDX_REC update_idx;
  char fn[25];
  FILE *fp;

  sprintf (fn, "update.data.%d", num);
  if ((fp = fopen (fn, "rb")) == NULL) {
    mudlog ("UPDATE: Error: Cannot read file %s", fn);
    return;
  }
  fread (&update_idx, sizeof (update_idx), 1, fp);
  fclose (fp);

  check_update (num);
  qdone = update_idx.qdone;
  read_mobiles (update_idx.mobnum, num);
  read_objects (update_idx.objnum, num);
  unlink (fn);
}

/* Checks to see if the world has changed any. It checks for new zones,
 * new locations, new mobiles, and new objects. It then logs a warning
 * that data has been changed. This is to let people know that something
 * could go wrong with the update since data has been changed.
 */
int
check_update (int num)
{
  UPDATE_IDX_REC data;
  Boolean error = False;
  char fn[25];
  FILE *fp;

  sprintf (fn, "update.data.%d", num);
  if ((fp = fopen (fn, "rb")) == NULL) {
    mudlog ("UPDATE: Error: Cannot read file %s", fn);
    return -1;
  }
  fread (&data, sizeof (data), 1, fp);
  fclose (fp);

  if (num_const_zon != data.mudzones) {
    mudlog ("UPDATE: Warning: Number of zones has been changed");
    error = True;
  }
  if (num_const_locs != data.mudlocs) {
    mudlog ("UPDATE: Warning: Number of locations has been changed");
    error = True;
  }
  if (num_const_chars - max_players != data.mudchars) {
    mudlog ("UPDATE: Warning: Number of characters has been changed");
    error = True;
  }
  if (num_const_obs != data.mudobjs) {
    mudlog ("UPDATE: Warning: Number of objects has been changed");
    error = True;
  }
  if (error)
    mudlog ("UPDATE: This could cause problems with updated data");
  return error;
}

/* Save the mobiles and return the number saved.
 */
int
save_mobiles (void)
{
  UPDATE_MOBILE_REC data;
  int mob, idx = 0;
  char fn[25];
  FILE *fp;

  sprintf (fn, "mobiles.update.%d", getpid ());
  if ((fp = fopen (fn, "wb")) == NULL) {
    mudlog ("UPDATE: Error: Cannot create file %s", fn);
    return -1;
  }
  for (mob = max_players; mob < numchars; mob++) {
    if (ploc (mob) != ploc_reset (mob) || pstr (mob) != pstr_reset (mob)) {
      ++idx;
      data.num = mob;
      data.str = pstr (mob);
      data.loc = ploc (mob);
      data.score = pscore (mob);
      fwrite (&data, sizeof (data), 1, fp);
    }
  }
  fclose (fp);
  return idx;
}

/* Save the objects and return the number saved.
 */
int
save_objects (void)
{
  UPDATE_OBJECT_REC data;
  int obj, idx = 0;
  char fn[25];
  FILE *fp;

  sprintf (fn, "objects.update.%d", getpid ());
  if ((fp = fopen (fn, "wb")) == NULL) {
    mudlog ("UPDATE: Error: Cannot create file %s", fn);
    return -1;
  }
  for (obj = 0; obj < numobs; obj++) {
    if (oloc (obj) != oloc_reset (obj) || ocarrf (obj) != ocarrf_reset (obj) ||
	obits (obj).u != obits_reset (obj).u ||
	obits (obj).h != obits_reset (obj).h ||
	obits (obj).l != obits_reset (obj).l ||
	state (obj) != state_reset (obj)) {
      ++idx;
      data.num = onum (obj);
      data.loc = oloc (obj);
      data.state = state (obj);
      data.carrflg = ocarrf (obj);
      data.flags_u = obits (obj).u;
      data.flags_h = obits (obj).h;
      data.flags_l = obits (obj).l;
      fwrite (&data, sizeof (data), 1, fp);
    }
  }
  fclose (fp);
  return idx;
}

/* Reads in the mobiles data file.
 */
void
read_mobiles (int mob, int fnum)
{
  UPDATE_MOBILE_REC data;
  int idx;
  char fn[25];
  FILE *fp;

  sprintf (fn, "mobiles.update.%d", fnum);
  if ((fp = fopen (fn, "rb")) == NULL) {
    mudlog ("UPDATE: Error: Cannot read file %s", fn);
    return;
  }
  for (idx = 0; idx < mob; idx++) {
    fread (&data, sizeof (data), 1, fp);
    setploc (data.num, data.loc);
    setpstr (data.num, data.str);
    setpscore (data.num, data.score);
  }
  fclose (fp);
  unlink (fn);
}

void
read_objects (int obj, int fnum)
{
  UPDATE_OBJECT_REC data;
  int idx;
  char fn[25];
  FILE *fp;

  sprintf (fn, "objects.update.%d", fnum);
  if ((fp = fopen (fn, "rb")) == NULL) {
    mudlog ("UPDATE: Error: Cannot read file %s", fn);
    return;
  }
  for (idx = 0; idx < obj; idx++) {
    fread (&data, sizeof (data), 1, fp);
    setoloc (data.num, data.loc, data.carrflg);
    setobjstate (data.num, data.state);
    obits (data.num).u = data.flags_u;
    obits (data.num).h = data.flags_h;
    obits (data.num).l = data.flags_l;
  }
  fclose (fp);
  unlink (fn);
}
