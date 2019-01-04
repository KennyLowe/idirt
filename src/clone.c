
#include <unistd.h>
#include <stdlib.h>
#include "kernel.h"
#include "bootstrap.h"
#include "mobile.h"
#include "pflags.h"
#include "cflags.h"
#include "sendsys.h"
#include "objsys.h"
#include "oflags.h"
#include "zones.h"
#include "parse.h"
#include "fight.h"
#include "exits.h"
#include "clone.h"
#include "eflags.h"
#include "commands.h"
#include "rooms.h"
#include "bprintf.h"
#include "uaf.h"
#include "log.h"

/**************************************************************************
 * clone.c
 *
 * New file, added April 1993
 * Nicknack
 */




/* Create a new object by copying an existing one. A new name and
 * zone for it to belong to (both optional) may be given.
 *
 * Return the new object's index number, or -1 on error.
 */
int
clone_object (int obj, int new_zone, char *new_name)
{
  int i;

  if (numobs >= GLOBAL_MAX_OBJS)
    return -1;

  if (numobs == obj_array_len) {

    obj_array_len = min (obj_array_len + 75, GLOBAL_MAX_OBJS);

    objects = resize_array (objects, sizeof (Object),
			    numobs, obj_array_len);
  }
  objects[numobs] = objects[obj];

  oname (numobs) = new_name != NULL ? COPY (new_name)
    : oname (obj) != NULL ? COPY (oname (obj)) : NULL;

  if (oaltname (obj) != NULL)
    oaltname (numobs) = COPY (oaltname (obj));

  for (i = 0; i < 4; i++)
    if (olongt (obj, i) != NULL)
      olongt (numobs, i) = COPY (olongt (obj, i));

  if (oexam_text (obj) != NULL)
    oexam_text (numobs) = COPY (oexam_text (obj));

  otemporary (numobs) = True;
  create (numobs);

  init_intset (oinv (numobs), otstbit (numobs, OFL_CONTAINER) ? 15 : 0);

  setoloc (numobs, oloc (numobs), ocarrf (numobs));

  insert_entry ((obj_id (numobs) = id_counter++), numobs, &id_table);

  if (new_zone > -1) {
    zadd_obj (numobs, ozone (numobs) = new_zone);
  } else {
    zadd_obj (numobs, ozone (obj));
  }

  return numobs++;
}



/* Destruct an object created by clone_object().
 *
 * If destructed, and its index was assigned to another object from the
 * same zone, place True in *index_reused.
 */
Boolean
destruct_object (int obj, Boolean * index_reused)
{
  int i;
  Boolean reused = False;

  if (opermanent (obj) || obj >= numobs)
    return False;

  /* If container, empty it's contents
   */
  if (!is_empty (oinv (obj))) {
    int new_loc = oloc (obj);
    int new_carrf = ocarrf (obj);

    switch (ocarrf (obj)) {
    case IN_CONTAINER:
    case IN_ROOM:
      break;
    case CARRIED_BY:
    case WORN_BY:
    case WIELDED_BY:
    case BOTH_BY:
      new_loc = ploc (oloc (obj));
      new_carrf = IN_ROOM;
      break;
    }
    for (i = ofirst_obj (obj); i != SET_END; i = onext_obj (obj)) {
      setoloc (i, new_loc, new_carrf);
    }
  }
  /* Remove the object from the world by setting it to to an illegal loc.
   */
  setoloc (obj, 0, IN_ROOM);

  /* Remove its ID from the id-table.
   */
  remove_entry (obj_id (obj), &id_table);

  /* Free the allocated memory.
   */
  free_intset (oinv (obj));

  if (oname (obj) != NULL)
    FREE (oname (obj));
  if (oaltname (obj) != NULL)
    FREE (oaltname (obj));
  if (oexam_text (obj) != NULL)
    FREE (oexam_text (obj));

  for (i = 0; i < 4; i++)
    if (olongt (obj, i) != NULL)
      FREE (olongt (obj, i));

  /* If this was one side of a door, shut the other side.
   */
  if (olinked (obj) > -1) {
    olinked (olinked (obj)) = -1;
    state (olinked (obj)) = EX_CLOSED;
  }
  /* Move the last object in the objects array down to fill up the gap.
   */
  zremove_obj (numobs - 1, ozone (numobs - 1));

  if (obj != numobs - 1) {

    if (!(reused = ozone (numobs - 1) == ozone (obj))) {

      zremove_obj (obj, ozone (obj));
      zadd_obj (obj, ozone (numobs - 1));
    }
    objects[obj] = objects[numobs - 1];
    setoloc (numobs - 1, 0, IN_ROOM);
    setoloc (obj, oloc (obj), ocarrf (obj));

    if (olinked (obj) != -1)
      olinked (olinked (obj)) = obj;

    /* Change the value of the moved object's index in the id-table
     */
    change_entry (obj_id (obj), obj, &id_table);

    /* Change all references in the inventory of 'numobs-1' to obj
     */
    for (i = ofirst_obj (obj); i != SET_END; i = onext_obj (obj)) {
      setoloc (i, obj, ocarrf (i));
    }
  }
  if (--numobs < obj_array_len - 140) {

    obj_array_len -= 75;

    objects = resize_array (objects, sizeof (Object),
			    numobs, obj_array_len);
  }
  if (index_reused != NULL)
    *index_reused = reused;
  return True;
}




/* Create a new mobile by copying an existing one.
 * Return the new mobile number, or -1 on error.
 */
int
clone_mobile (int mob, int new_zone, char *new_name)
{
  if (numchars >= GLOBAL_MAX_MOBS + max_players)
    return -1;

  if (numchars == char_array_len) {

    char_array_len =
      min (char_array_len + 75, GLOBAL_MAX_MOBS + max_players);

    ublock = resize_array (ublock, sizeof (Mobile),
			   numchars, char_array_len);
  }
  ublock[numchars] = ublock[mob];

  if (new_name != NULL) {
    setpname (numchars, new_name);
    pname_reset (numchars) = COPY (new_name);
  } else {
    pname_reset (numchars) = COPY (pname_reset (mob));
  }

  if (pexam (mob) != NULL)
    pexam (numchars) = COPY (pexam (mob));
  if (pftxt (mob) != NULL) {
    char b[128];

    pftxt (numchars) = COPY (new_name == NULL ? pftxt (mob)
			     : (sprintf (b, "%s is here.", new_name), b));
  }
  init_intset (pinv (numchars), 4);

  ptemporary (numchars) = True;

  setploc (numchars, ploc (numchars));

  insert_entry ((mob_id (numchars) = id_counter++), numchars, &id_table);

  if (new_zone > -1) {
    zadd_mob (numchars, pzone (numchars) = new_zone);
  } else {
    zadd_mob (numchars, pzone (mob));
  }

  return numchars++;
}



/* Destruct a mobile created with clone_mobile().
 *
 * If destructed, and its index was assigned to another mobile from the
 * same zone, place True in index_reused.
 */
Boolean
destruct_mobile (int mob, Boolean * index_reused)
{
  int i, j;
  Boolean reused = False;

  if (ppermanent (mob) || mob >= numchars)
    return False;

  setpfighting (mob, -1);

  /* If someone is aliasing it, lose them.
   */
  if ((j = find_pretender (mob)) > -1) {
    unalias (j);
    unpolymorph (j);
  }
  /* If it carries something, drop it
   */
  for (i = pfirst_obj (mob); i != SET_END; i = pnext_obj (mob)) {

    setoloc (i, ploc (mob), IN_ROOM);
  }

  /* Remove mobile from the world by setting it to to an illegal room.
   */
  setploc (mob, 0);

  /* Remove its ID from the id-table.
   */
  remove_entry (mob_id (mob), &id_table);

  /* Free the allocated memory.
   */
  free_intset (pinv (mob));
  FREE (pname_reset (mob));
  if (pexam (mob) != NULL)
    FREE (pexam (mob));
  if (pftxt (mob) != NULL)
    FREE (pftxt (mob));

  /* Move the last mobile in the mobile array down to fill up the gap.
   */
  zremove_mob (numchars - 1, pzone (numchars - 1));

  if (mob != numchars - 1) {

    if (!(reused = pzone (mob) == pzone (numchars - 1))) {

      zremove_mob (mob, pzone (mob));
      zadd_mob (mob, pzone (numchars - 1));
    }
    ublock[mob] = ublock[numchars - 1];
    setploc (numchars - 1, 0);
    setploc (mob, ploc (mob));

    /* If someone was aliasing the last mobile in the array,
     * * change his alias-entry to this mobile's new index:
     */
    if ((j = find_pretender (numchars - 1)) > -1) {
      players[j].pretend = mob;
    }
    /* Update the id-table with its new index.
     */
    change_entry (mob_id (mob), mob, &id_table);

    /* Change the references of its inventory to its new number
     */
    for (i = pfirst_obj (mob); i != SET_END; i = pnext_obj (mob)) {

      setoloc (i, mob, ocarrf (i));
    }

    /* Change the references to it for its fight opponents &helpers
     */
    for (i = 0; i < lnumchars (ploc (mob)); i++) {

      if (phelping (lmob_nr (i, ploc (mob))) == numchars - 1)
	setphelping (lmob_nr (i, ploc (mob)), mob);

      if (pfighting (lmob_nr (i, ploc (mob))) == numchars - 1)
	setpfighting (lmob_nr (i, ploc (mob)), mob);
    }
  }
  if (--numchars < char_array_len - 140) {

    char_array_len -= 75;

    ublock = resize_array (ublock, sizeof (Mobile),
			   numchars, char_array_len);
  }
  if (index_reused != NULL)
    *index_reused = reused;
  return True;
}




/* Create a new location by copying an existing one.
 * Return the new location number, or 0 on error.
 */
int
clone_location (int l, int new_zone, char *new_name)
{
  int i;
  int loc_array_index = convroom (l);
  int c_numloc = convroom (numloc);

  if (numloc >= GLOBAL_MAX_LOCS)
    return 0;

  if (numloc == loc_array_len) {

    loc_array_len = min (loc_array_len + 100, GLOBAL_MAX_LOCS);

    room_data = resize_array (room_data, sizeof (Location),
			      numloc, loc_array_len);
  }
  room_data[numloc] = room_data[loc_array_index];
  room_data[numloc].temporary = True;

  for (i = 0; i < NEXITS; i++)
    setexit (c_numloc, i, lexit (c_numloc, i));

  lshort (c_numloc) = new_name != NULL ? COPY (new_name)
    : lshort (l) != NULL ? COPY (lshort (l)) : NULL;

  if (llong (l) != NULL)
    llong (c_numloc) = COPY (llong (l));

  init_intset (linv (c_numloc), 7);
  init_intset (lmobs (c_numloc), 5);
  init_intset (lexits_to_me (c_numloc), 4);

  insert_entry (loc_id (c_numloc) = id_counter++, c_numloc, &id_table);

  if (new_zone > -1) {
    zadd_loc (c_numloc, lzone (c_numloc) = new_zone);
  } else {
    zadd_loc (c_numloc, lzone (l));
  }

  ++numloc;

  return c_numloc;
}



/* Destruct a location created by clone_location().
 *
 * If destructed, and its index was assigned to another location from the
 * same zone, place True in index_reused.
 */
Boolean
destruct_location (int l, Boolean * index_reused)
{
  int i, j;
  Boolean reused = False;

  int loc_array_index = convroom (l);

  if (lpermanent (l) || loc_array_index >= numloc)
    return False;

  /* If it contains players, don't destruct.
   */
  for (i = 0; i < lnumchars (l); i++) {

    if (lmob_nr (i, l) < max_players)
      return False;
  }

  /* If it contains mobiles, move them away.
   */
  for (i = lfirst_mob (l); i != SET_END; i = lnext_mob (l)) {

    destroy_mobile (i);
  }

  /* If it contains objects, destroy them.
   */
  for (i = lfirst_obj (l); i != SET_END; i = lnext_obj (l)) {

    destroy (i);
  }

  /* Clear exits
   */
  for (i = 0; i < NEXITS; i++)
    setexit (l, i, 0);

  for (i = first_int (lexits_to_me (l)); i != SET_END;
       i = next_int (lexits_to_me (l))) {

    for (j = 0; j < NEXITS; j++)
      if (lexit (i, j) == l)
	setexit (i, j, 0);
  }

  /* Free the allocated memory.
   */
  free_intset (linv (l));
  free_intset (lmobs (l));
  free_intset (lexits_to_me (l));

  if (lshort (l) != NULL)
    FREE (lshort (l));
  if (llong (l) != NULL)
    FREE (llong (l));

  /* Remove its ID entry in the ID table.
   */
  remove_entry (loc_id (l), &id_table);

  /* Move the last room in the locations array down to fill up the gap.
   */
  zremove_loc (convroom (numloc - 1), lzone (convroom (numloc - 1)));

  if (loc_array_index != numloc - 1) {

    if (!(reused = lzone (l) == lzone (convroom (numloc - 1)))) {

      zremove_loc (l, lzone (l));
      zadd_loc (l, lzone (convroom (numloc - 1)));
    }
    room_data[loc_array_index] = room_data[numloc - 1];

    change_entry (loc_id (l), l, &id_table);

    /* Change the references of its inventory to its new number
     */
    for (i = lfirst_obj (l); i != SET_END; i = lnext_obj (l)) {

      setoloc (i, l, ocarrf (i));
    }

    for (i = lfirst_mob (l); i != SET_END; i = lnext_mob (l)) {

      setploc (i, l);
    }
  }
  if (--numloc < loc_array_len - 140) {

    loc_array_len -= 75;

    room_data = resize_array (room_data, sizeof (Location),
			      numloc, loc_array_len);
  }
  if (index_reused != NULL)
    *index_reused = reused;
  return True;
}


/* Illegal characters in all names, descs etc.
 */
static int
illegal_char (int c)
{
  return c == '^';
}


/* The CLONE command.
 */
void
clonecom (Boolean do_brkword)
{
  int id, a;
  int zone;
  char name[MAX_COM_LEN], new_name[MAX_COM_LEN];
  char *p, *q;
  int blanks = 0, digits = 0, graphic = 0, others = 0, illegal = 0;
  PERSONA P, P2;

  if (!ptstflg (mynum, PFL_CLONE)) {
    erreval ();
    return;
  }
  if (do_brkword && brkword () == -1) {
    bprintf ("Clone what ?\n");
    return;
  }
  strcpy (name, wordbuf);

  if (EMPTY (p = getreinput (new_name))) {
    p = NULL;
  } else {
    for (q = p; *q != '\0'; q++) {
      if (isdigit (*q))
	digits++;
      else if (illegal_char (*q))
	illegal++;
      else if (*q == ' ')
	blanks++;
      else if (!isalpha (*q) && isgraph (*q))
	graphic++;
      else if (!isalpha (*q))
	others++;
    }

    /* Remove trailing spaces: */
    while (--q > p && *q == ' ') ;

    *++q = '\0';
  }

  if (illegal || others) {
    bprintf ("Illegal character(s) in name.\n");
    return;
  }
  zone = get_wizzone_by_name (pname (mynum));

  if ((a = find_loc_by_name (name)) < 0) {

    if ((id = clone_location (a, zone, p)) == 0) {
      bprintf ("The max # of rooms (%d) has been reached.\n",
	       GLOBAL_MAX_LOCS);
      return;
    }
    bprintf ("[%s]\n", showname (id));

#ifdef LOG_CLONE_ROOM
    mudlog ("CLONE: Location: %s cloned %s to %s", pname (mynum), p, sdesc (id));
#endif

  } else if ((a = fpbn (name)) != -1) {

    if (digits || graphic) {
      bprintf ("New mobile name: blanks and letters only.\n");
      return;
    }
    if (p != NULL && strlen (p) > MNAME_LEN) {
      bprintf ("Mobile name %s too long (max = %d chars.)\n",
	       p, MNAME_LEN);
      return;
    }
    if (getuaf (pname (a), &P) || getuaf (p, &P2)) {
      bprintf ("There is a player with that name!\n");
      return;
    }
    if (strstr (pname (a), "Puff") != NULL) {
      bprintf ("Sorry, we don't need any more fractal dragons running around!\n");
      return;
    }
    if ((id = clone_mobile (a, zone, p)) < 0) {
      bprintf ("The max # of mobs (%d) has been reached.\n",
	       GLOBAL_MAX_MOBS);
      return;
    }
    reset_mobile (id);
    setploc (id, ploc (mynum));

    bprintf ("%s (%d) appears before you!\n", pname (id), GLOBAL_MAX_OBJS + id);

    send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	      "%s appears before you!\n", pname (id));

#ifdef LOG_CLONE_MOBILE
    mudlog ("CLONE: Mobile: %s has cloned %s to %s", pname (mynum), pname (a), pname (id));
#endif

  } else if ((a = fobn (name)) != -1) {

    if (digits || graphic || blanks) {
      bprintf ("New object name: letters only.\n");
      return;
    }
    if (is_classname (p)) {
      bprintf ("Can't be same name as a class of objects.\n");
      return;
    }
    if (p != NULL && strlen (p) > ONAME_LEN) {
      bprintf ("Object name %s too long (max = %d chars.)\n",
	       p, ONAME_LEN);
      return;
    }
    if (olinked (a) > -1) {
      bprintf ("That's a door-type object and can't be "
	       "cloned (yet)\n");
      return;
    }
    if ((id = clone_object (a, zone, p)) < 0) {
      bprintf ("The max # of objs (%d) has been reached.\n",
	       GLOBAL_MAX_OBJS);
      return;
    }
    setoloc (id, ploc (mynum), IN_ROOM);

    bprintf ("The %s (%d) is created before you!\n", oname (id), id);

    send_msg (ploc (mynum), 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	      "The %s is created before you!\n", oname (id));

#ifdef LOG_CLONE_OBJECT
    mudlog ("CLONE: Object: %s cloned %s to %s", pname (mynum), oname (a), oname (id));
#endif
  } else if (a == 1) {

    strcat (wordbuf, "1");
    clonecom (False);
  } else {
    bprintf ("I don't know any %s\n", name);
  }
}


/* The DESTRUCT command.
 */
void
destructcom (char *args)
{
  int a;

  if (!ptstflg (mynum, PFL_CLONE)) {
    erreval ();
    return;
  }
  if (args == NULL && (args = wordbuf, brkword () == -1)) {
    bprintf ("Destruct what ?\n");
    return;
  }
  if ((a = find_loc_by_name (args)) < 0) {

    if (lpermanent (a)) {
      bprintf ("That's a permanent location.\n");
    } else if (!destruct_location (a, NULL)) {
      bprintf ("There are players in that location.\n");
    } else {
      bprintf ("Ok.\n");

#ifdef LOG_DEST_ROOM
      mudlog ("DESTRUCT: Location: %s destructed %s", pname (mynum), showname (a));
#endif

    }
  } else if ((a = fpbn (args)) != -1 && a >= max_players) {

    if (ppermanent (a)) {
      bprintf ("That's a permanent mobile.\n");
      return;
    }
    send_msg (ploc (a), 0, pvis (a), LVL_MAX, NOBODY, NOBODY,
	      "%s crumbles to dust.\n", pname (a));

    if (!is_empty (pinv (a)))
      send_msg (ploc (a), 0, pvis (a), LVL_MAX, NOBODY, NOBODY,
		"Its belongings drop to the ground.\n");

#ifdef LOG_DEST_MOBILE
    mudlog ("DESTRUCT: Mobile: %s destructed %s", pname (mynum), pname (a));
#endif

    destruct_mobile (a, NULL);
  } else if ((a = fobn (args)) != -1) {

    if (opermanent (a)) {
      bprintf ("That's a permanent object.\n");
      return;
    }
    send_msg (obj_loc (a), 0, ovis (a), LVL_MAX, NOBODY, NOBODY,
	      "The %s crumbles to dust.\n", oname (a));

    if (!is_empty (oinv (a)) && ocarrf (a) >= CARRIED_BY)
      send_msg (obj_loc (a), 0, ovis (a), LVL_MAX, NOBODY, NOBODY,
		"Its contents drops to the ground.\n");

#ifdef LOG_DEST_OBJECT
    mudlog ("DESTRUCT: Object: %s destructed %s", pname (mynum), oname (a));
#endif

    destruct_object (a, NULL);
  } else if (a == 1) {
    destructcom (strcat (args, "1"));
  } else {
    bprintf ("I don't know any %s\n", args);
  }
}


/* The STORE command.
 */
void
storecom ()
{
  char filename[128];
  char *p;
  int z, x, i, j, zlev;
  PERSONA P;
  FILE *f;
  int n_locs = 0, n_mobs = 0, n_objs = 0;

  char *err_unsucc = "Store was unsuccessful.\n";

  if (!ptstflg (mynum, PFL_LD_STORE)) {
    erreval ();
    return;
  }
  p = (brkword () == -1) ? pname (mynum) : wordbuf;

  if ((z = get_zone_by_name (p)) < num_const_zon) {
    bprintf ("%s: Nothing to store.\n", p);
    return;
  }
  zlev = (x = fpbns (p)) >= 0 ? plev (x)
    : getuaf (p, &P) ? P.p_level : LVL_MIN;

  if (!EQ (pname (mynum), zname (z))
      && !do_okay_l (plev (mynum), zlev, False)) {

    bprintf ("You're not powerful enough.\n");
    return;
  }
  /* Mark zone not temporary
   */
  ztemporary (z) = False;

  /* Update the id_counter file so that when the server exits or crashes
   * * the saved id-counter is allways greater then any saved item's ID.
   */
  if (!save_id_counter ()) {
    bprintf ("%s", err_unsucc);
    return;
  }
  wiz_mob_filename (filename, zname (z));

  if (!is_empty (zmobs (z))) {

    if ((f = fopen (filename, "w")) == NULL) {
      progerror (filename);
      bprintf ("%s", err_unsucc);
      return;
    }
    fprintf (f, "%d\n", n_mobs = znumchars (z));

    for (i = zfirst_mob (z); i != SET_END; i = znext_mob (z)) {

      /* Mark him not to be destroyed on reset:
       */
      ptemporary (i) = False;

      /* Set his reset values to his current state:
       */
      ploc_reset (i) = loc_id (ploc (i));
      pstr_reset (i) = pstr (i);
      pvis_reset (i) = pvis (i);
      sflags_reset (i) = sflags (i);
      pflags_reset (i) = pflags (i);
      mflags_reset (i) = mflags (i);
      nflags_reset (i) = nflags (i);
      eflags_reset (i) = eflags (i);
      plev_reset (i) = plev (i);
      pagg_reset (i) = pagg (i);
      pspeed_reset (i) = pspeed (i);
      pdam_reset (i) = pdam (i);
      parmor_reset (i) = parmor (i);
      pwimpy_reset (i) = pwimpy (i);

      /* Store it on file
       */
      fprintf (f, "%s^\n",
	       EMPTY (pname (i)) ? pname_reset (i) : pname (i));

      fprintf (f, "%ld %d %d %ld %d %d %d %d %d %d %d\n",
	       mob_id (i), pnum (i), z, ploc_reset (i),
	       pstr (i), pdam (i), pagg (i), parmor (i),
	       pspeed (i), pvis (i), pwimpy (i));

      fprintf (f, "0x%08lx:0x%08lx 0x%08lx:0x%08lx:0x%08lx\n",
	       sflags (i).h, sflags (i).l,
	       pflags (i).u, pflags (i).h, pflags (i).l);

      fprintf (f, "0x%08lx:0x%08lx 0x%08lx 0x%08lx\n",
	       mflags (i).h, mflags (i).l, nflags (i), eflags (i));

      fprintf (f, "%s^\n", pftxt (i) == NULL ? "" : pftxt (i));
      fprintf (f, "%s^\n\n", pexam (i) == NULL ? "" : pexam (i));
    }

    fclose (f);
  } else {
    unlink (filename);
  }

  wiz_obj_filename (filename, zname (z));

  if (!is_empty (zobjs (z))) {

    FILE *obj_file;

    if ((obj_file = fopen (OBJECTS, "r")) == NULL) {
      progerror (OBJECTS);
      bprintf ("%s", err_unsucc);
      return;
    }
    if ((f = fopen (filename, "w")) == NULL) {
      progerror (filename);
      bprintf ("%s", err_unsucc);
      fclose (obj_file);
      return;
    }
    fprintf (f, "%d\n", n_objs = znumobs (z));

    for (i = zfirst_obj (z); i != SET_END; i = znext_obj (z)) {

      /* Mark it not to be destroyed on reset:
       */
      otemporary (i) = False;

      /* Set its reset values to its current state:
       */
      oloc_reset (i) = ocarrf (i) == IN_ROOM ? loc_id (oloc (i))
	: ocarrf (i) == IN_CONTAINER ? obj_id (oloc (i))
	: mob_id (oloc (i));
      osize_reset (i) = osize (i);
      ovalue_reset (i) = obaseval (i);
      ovis_reset (i) = ovis (i);
      odamage_reset (i) = odamage (i);
      oarmor_reset (i) = oarmor (i);
      ocarrf_reset (i) = ocarrf (i);
      state_reset (i) = state (i);
      obits_reset (i) = obits (i);

      fprintf (f, "%s %s %d %ld %d %ld "
	       "%d %d %ld %d %d %d 0x%08lx:0x%08lx:0x%08lx %d %d %d %d\n",

	       oname (i),
	       oaltname (i) != NULL ? oaltname (i) : "<null>",
	       z, obj_id (i), onum (i),
	       olinked (i) == -1 ? -1L : obj_id (olinked (i)),

	       ovis (i), ocarrf (i), oloc_reset (i),
	       state (i), odamage (i), oarmor (i),
	       obits (i).u, obits (i).h, obits (i).l,
	       omaxstate (i), obaseval (i), osize (i), 0 /*weight */ );

      for (j = 0; j < 4; j++) {
	if (olongt (i, j) != NULL) {
	  fprintf (f, "%s", olongt (i, j));
	}
	fprintf (f, "^\n");
      }

      if (oexam_text (i) != NULL) {
	fprintf (f, "%s", oexam_text (i));
      } else if (oexamine (i) > 0) {
	int c;

	fseek (obj_file, oexamine (i), 0);

	while ((c = getc (obj_file)) != '^' && c != EOF)
	  putc (c, f);
      }
      fprintf (f, "^\n\n");
    }

    fclose (obj_file);
    fclose (f);
  } else {
    unlink (filename);
  }

  wiz_loc_filename (filename, zname (z));

  if (!is_empty (zlocs (z))) {

    if ((f = fopen (filename, "w")) == NULL) {
      progerror (filename);
      bprintf ("%s", err_unsucc);
      return;
    }
    fprintf (f, "%d\n", n_locs = znumloc (z));

    for (i = zfirst_loc (z); i != SET_END; i = znext_loc (z)) {

      /* Mark it not to be destroyed on resets:
       */
      ltemporary (i) = False;

      /* Set its reset values to its current state:
       */
      ltouched (i) = False;
      for (j = 0; j < NEXITS; j++) {
	lexit_reset (i, j) = lexit (i, j);
	if (exists (lexit (i, j)))
	  lexit_reset (i, j) = loc_id (lexit (i, j));
      }

      xlflags_reset (i) = xlflags (i);

      /* Store it on file
       */
      fprintf (f, "%ld %d", loc_id (i), z);

      for (j = 0; j < NEXITS; j++) {
	fprintf (f, " %ld", lexit_reset (i, j));
      }

      fprintf (f, "\n0x%08lx:0x%08lx\n%s^\n",
	       xlflags (i).h, xlflags (i).l, lshort (i));

      if (llong (i) != NULL) {
	char *q = llong (i);

	while (*q != '\0')
	  putc (*q++, f);
      }
      fprintf (f, "^\n");
    }

    fclose (f);
  } else {
    unlink (filename);
  }

  bprintf ("Zone %s: Stored %d room(s), %d mobile(s) and %d object(s).\n",
	   zname (z), n_locs, n_mobs, n_objs);
}


/* The LOAD command.
 */
void
loadcom (void)
{
  char *p;
  int zlev;
  PERSONA P;
  int plr;
  int nlocs, nlocs_f, nmobs, nmobs_f, nobjs, nobjs_f;
  int mem_used;

  if (!ptstflg (mynum, PFL_LD_STORE)) {
    erreval ();
    return;
  }
  if (brkword () == -1) {
    zlev = plev (mynum);
    p = pname (mynum);
  } else if ((plr = fpbns (wordbuf)) >= 0) {
    zlev = plev (plr);
    p = pname (plr);
  } else if (getuaf (wordbuf, &P)) {
    zlev = P.p_level;
    p = P.p_name;
  } else {
    bprintf ("No such player: %s\n", wordbuf);
    return;
  }

  if (!EQ (pname (mynum), p) && !do_okay_l (plev (mynum), zlev, True)) {
    bprintf ("You're not powerful enough.\n");
    return;
  }
  mem_used = load_zone (p, &nlocs, &nlocs_f, &nmobs, &nmobs_f,
			&nobjs, &nobjs_f);

  if (mem_used < 0) {
    bprintf ("Load failed for zone %s.\n", p);
  } else {
    if (nlocs_f == 0 && nmobs_f == 0 && nobjs_f == 0) {
      bprintf ("Can't find anything stored for %s.\n", p);
      return;
    }
    bprintf ("Zone %s:\n\n", p);
    if (nlocs_f > 0) {
      bprintf ("Loaded %d location(s) from %d",
	       nlocs, nlocs_f);
      if (nlocs != nlocs_f)
	bprintf (" (%d already existed)",
		 nlocs_f - nlocs);
      bprintf ("\n");
    }
    if (nmobs_f > 0) {
      bprintf ("Loaded %d mobile(s) from %d",
	       nmobs, nmobs_f);
      if (nmobs != nmobs_f)
	bprintf (" (%d already existed)",
		 nmobs_f - nmobs);
      bprintf ("\n");
    }
    if (nobjs_f > 0) {
      bprintf ("Loaded %d object(s) from %d",
	       nobjs, nobjs_f);
      if (nobjs != nobjs_f)
	bprintf (" (%d already existed)",
		 nobjs_f - nobjs);
      bprintf ("\n");
    }
  }
}

/********************************************************
 * Extended Object Editing Routines			*
 * 1995, Illusion					*
 ********************************************************/

/* linkcom:
 * Links two objects together.
 * 1995, Illusion
 */
void
linkcom (void)
{
  int obj1, obj2;

  if (!ptstflg (mynum, PFL_CLONE)) {
    erreval ();
    return;
  }
  if ((obj1 = ob1) == -1) {
    bprintf ("Object 1 does not exist.\n");
    return;
  }
  if ((obj2 = ob2) == -1) {
    bprintf ("Object 2 does not exist.\n");
    return;
  }
  olinked (obj1) = obj2;
  olinked (obj2) = obj1;

  mudlog ("LINK: %s linked %s (%d) to %s (%d)", pname (mynum), oname (obj1),
	  onum (obj1), oname (obj2), onum (obj2));

  bprintf ("Linked %s to %s.\n", oname (obj1), oname (obj2));
}

/* maxstatecom:
 * Set the maxstate of an object.
 * 1995, Illusion
 */
void
maxstatecom (void)
{
  int obj, st;

  if (!ptstflg (mynum, PFL_CLONE)) {
    erreval ();
    return;
  }
  if ((obj = ob1) == -1) {
    bprintf ("Object does not exist.\n");
    return;
  }
  if (EMPTY (txt2)) {
    bprintf ("A new MaxState must be provided.\n");
    return;
  }
  st = max (0, atoi (txt2));

  if (st > 3) {
    bprintf ("MaxState too large: %d.\n", st);
    return;
  } else {
    bprintf ("Setting MaxState to %d.\n", st);
  }

  mudlog ("MAXSTATE: %s set the MaxState of %s (%d) to %d", pname (mynum),
	  oname (obj), onum (obj), st);

  omaxstate (obj) = st;
}
