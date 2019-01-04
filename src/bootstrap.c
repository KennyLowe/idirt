

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "kernel.h"
#include "bootstrap.h"
#include "wizlist.h"
#include "actions.h"
#include "timing.h"
#include "oflags.h"
#include "nflags.h"
#include "eflags.h"
#include "log.h"
#include <errno.h>

#define get_newline(f) while (getc(f) != '\n')

//extern int errno;

extern char *Pflags[];
extern char *WizLevels[];

static void boot_world (void);
static int boot_players (void);
static int boot_mobiles (FILE * f, char *fname);
static int boot_levels (FILE * f, char *fname);
static int boot_locations (FILE * f, char *fname);
static int boot_objects (FILE * f, char *fname);
static int boot_pflags (FILE * f, char *fname);
static int boot_verbs (FILE * f, char *fname);
static int boot_zones (FILE * f, char *fname);
static int boot_id_counter (void);

static Boolean read_pflags (FILE * f, PFLAGS p[]);
static char *get_string (FILE * f);

#define ID_CNT_START 200000L

char *PFT[] =
{"PflagApprentice", "MaskApprentice",
 "PflagWizard", "MaskWizard",
 "PflagProphet", "MaskProphet",
 "PflagArchwizard", "MaskArchwizard",
 "PflagAdvisor", "MaskAdvisor",
 "PflagAvatar", "MaskAvatar",
 "PflagGod", "MaskGod",
 "PflagMaster", "MaskMaster",
 "Comment",
 TABLE_END};

int
bootstrap (void)
{
  FILE *bootf, *f;
  int mem_used = 0;
  int tot_used = 0;
  char *y, x[128];
  Boolean not_loaded = False;

  boot_world ();

  printf ("\niDiRT Bootstrap Loader\n");

  mem_used = boot_id_counter ();

  printf ("  players\t: Allocated %d Bytes.\n",
	  tot_used = boot_players ());

  if ((bootf = fopen (BOOTSTRAP, "r")) == NULL) {
    printf ("fopen: Bootstrap failed for \"" BOOTSTRAP "\".\n");
    printf ("fopen: %s.\n", sys_errlist[errno]);
    return -1;
  }
  while (fgets (x, sizeof x, bootf)) {

    if ((y = strchr (x, '\n')))
      *y = '\0';

    printf ("  %s", x);

    if ((y = strchr (x, ':')) == NULL) {
      printf ("\nInvalid bootstrap declaration\n");
      return -1;
    }
#ifdef USE_ZOPEN
    if ((f = zopen (++y)) == NULL) {
      not_loaded = True;
    }
#else
    if ((f = fopen (++y, "r")) == NULL) {
      not_loaded = True;
    }
#endif

    switch (x[0]) {
    case 'A':
      mem_used = boot_extern (f, y);
      break;
    case 'C':
      mem_used = boot_mobiles (f, y);
      break;
    case 'E':
      mem_used = boot_levels (f, y);
      break;
    case 'H':
      mem_used = boot_hours (f, y);
      break;
    case 'L':
      mem_used = boot_locations (f, y);
      break;
    case 'O':
      mem_used = boot_objects (f, y);
      break;
    case 'P':
      mem_used = boot_pflags (f, y);
      break;
    case 'V':
      mem_used = boot_verbs (f, y);
      break;
    case 'W':
      set_wizfile (y);
      mem_used = boot_wizlist (f, y);
      break;
    case 'Z':
      mem_used = boot_zones (f, y);
      break;
    }

    if (f != NULL)
      fclose (f);

    if (mem_used < 0) {
      fclose (bootf);
      return mem_used;
    }
    printf ("\t: Allocated %d Bytes.", mem_used);
    tot_used += mem_used;

    if (not_loaded)
      printf (" (Can't open %c file %s)\n", x[0], y);
    else
      printf ("\n");
    not_loaded = False;
  }

  fclose (bootf);

  printf ("\n  Total Memory Allocated : %d Bytes.\n", tot_used);
  return 0;
}


static void
boot_world (void)
{
  WORLD_REC *w = &the_world_rec;
  CLIMATE_REC *c = &the_climate_rec;

  c->weather = 0;
  w->w_lock = 0;
  w->w_mob_stop = 0;
  w->w_peace = 0;
  w->w_max_users = max_players;
  w->w_tournament = 0;
}



static int
boot_players (void)
{
  int i;

  PLAYER_REC *p = players = NEW (PLAYER_REC, max_players);

  for (i = 0; i < max_players; i++, p++) {
    p->inp_handler = NULL;
    p->iamon = False;
  }

  return sizeof (PLAYER_REC) * max_players;
}



static int
boot_mobiles (FILE * f, char *fname)
{
  int mem_used = 0;
  int new_mem;
  int i;
  int mobs_loaded;

  if (f == NULL)
    return -1;

  numchars = max_players;

  ublock = NEW (UBLOCK_REC, char_array_len = numchars + 150);
  mem_used += sizeof (UBLOCK_REC) * char_array_len;

  for (i = 0; i < max_players; i++) {
    setpname (i, "");
    pnum (i) = -1;
  }

  if ((new_mem = load_mobiles (-1, f, &mobs_loaded, NULL)) < 0)
    return -1;

  num_const_chars = mobs_loaded + max_players;

  return mem_used + new_mem;
}


/* Read mobile spesifications from a file and load them into the game in the
 * zone specified. If it is -1, then use the zone specifications given in
 * the file for each mobile sepeartely. Return the number of mobiles actually
 * read, the total number of mobiles in the file,
 * and the amount of any extra memory allocated as the function value.
 * (-1 on error).
 */
int
load_mobiles (int zone, FILE * f, int *loaded, int *infile)
{
  int ct, c, i;
  int mem_used = 0;
  int num_ld, num_infile;
  char x[256];
  char *p;

  fgets (x, sizeof x, f);

  if ((num_ld = num_infile = atoi (x)) <= 0)
    return -1;

  if (numchars + num_infile > char_array_len) {

    int oldlen = char_array_len;

    if (numchars + num_infile > GLOBAL_MAX_MOBS)
      return -1;

    char_array_len = numchars + 75 + num_infile;

    ublock = resize_array (ublock, sizeof (Mobile),
			   oldlen, char_array_len);

    mem_used += sizeof (Mobile) * (char_array_len - oldlen);
  }
  for (ct = numchars; ct < numchars + num_ld; ct++) {

    ptemporary (ct) = False;

    for (p = pname (ct), i = 0;
	 (c = getc (f)) != '^' && i < MNAME_LEN; *p++ = c, i++) ;

    *p = '\0';
    get_newline (f);

    if (fscanf (f, "%ld %d %d %ld %d %d %d %d %d %d %d",
		&mob_id (ct), &pnum (ct),
		&pzone (ct),
		&ploc_reset (ct), &pstr_reset (ct),
		&pdam_reset (ct), &pagg_reset (ct), &parmor_reset (ct),
		&pspeed_reset (ct), &pvis_reset (ct), &pwimpy_reset (ct))
	!= 11)
      return -1;

    plev_reset (ct) = -1;	/* Negative level for all mobiles. */
    get_newline (f);

    /* If the mobile is allready in the game, skip that record
     */
    if (mob_id (ct) >= ID_CNT_START &&
	!insert_entry (mob_id (ct), ct, &id_table)) {

      get_newline (f);
      get_newline (f);

      for (i = 1; i <= 2; i++) {
	while (getc (f) != '^') ;
      }
      get_newline (f);
      get_newline (f);

      --ct;
      --num_ld;
      continue;
    }
    fscanf (f, "0x%8lx:0x%8lx 0x%8lx:0x%8lx:0x%8lx",
	    &sflags_reset (ct).h, &sflags_reset (ct).l,
	  &pflags_reset (ct).u, &pflags_reset (ct).h, &pflags_reset (ct).l);

    get_newline (f);

    fscanf (f, "0x%8lx:0x%8lx 0x%8lx 0x%8lx",
	    &mflags_reset (ct).h, &mflags_reset (ct).l,
	    &nflags_reset (ct), &eflags_reset (ct));

    /* Quick Language Patch */
    setplang (ct, NFL_ENGLISH);

    get_newline (f);

    pname_reset (ct) = COPY (pname (ct));

    pftxt (ct) = get_string (f);

    mem_used += strlen (pftxt (ct)) + strlen (pname_reset (ct)) + 2;

    if (EMPTY (pexam (ct) = get_string (f))) {
      FREE (pexam (ct));
      pexam (ct) = NULL;
    } else {
      mem_used += strlen (pexam (ct));
    }

    get_newline (f);

    init_intset (pinv (ct), 4);

    mem_used += get_set_mem_usage (pinv (ct));


    if (zone > -1)
      pzone (ct) = zone;
    zadd_mob (ct, pzone (ct));
  }

  numchars += num_ld;

  if (loaded != NULL)
    *loaded = num_ld;
  if (infile != NULL)
    *infile = num_infile;

  return mem_used;
}



static int
boot_levels (FILE * f, char *fname)
{
  int i, v, ct;

  if (f == NULL)
    return -1;
  fscanf (f, "%d", &ct);
  get_newline (f);
  if (ct != LVL_WIZARD) {
    printf ("Number of levels in levels file (%d) doesn't match LVL_WIZARD"
	    "(%d).\n", ct, LVL_WIZARD);
    return -1;
  }
  for (i = 1; i <= ct; i++) {
    fscanf (f, "%d", &v);
    get_newline (f);
    levels[i] = v;
  }
  return 0;
}




static int
boot_locations (FILE * f, char *fname)
{
  int mem_used = 0;
  int new_mem;

  if (f == NULL)
    return -1;

  numloc = 0;
  room_data = NEW (Location, loc_array_len = 800);

  mem_used += sizeof (Location) * loc_array_len;

  if ((new_mem = load_locations (-1, f, &num_const_locs, NULL)) < 0)
    return -1;

  return mem_used + new_mem;
}


/* Load locations from file into the game. Return the number of rooms the
 * file contained, and the number actually loaded. Return mem-used for value.
 */
int
load_locations (int zone, FILE * f, int *loaded, int *infile)
{
  int ct, mem_used = 0;
  int num_ld, num_infile;
  Location *r;
  char x[128];
  int i;

  fgets (x, sizeof x, f);

  if ((num_ld = num_infile = atoi (x)) <= 0)
    return -1;

  if (numloc + num_infile > loc_array_len) {

    int oldlen = loc_array_len;

    if (numloc + num_infile > GLOBAL_MAX_LOCS)
      return -1;

    loc_array_len = 100 + numloc + num_infile;

    room_data = resize_array (room_data, sizeof (Location),
			      oldlen, loc_array_len);

    mem_used += sizeof (Location) * (loc_array_len - oldlen);
  }
  for (r = room_data + numloc, ct = numloc;
       ct < numloc + num_ld; ct++, r++) {

    if (fscanf (f, "%ld %d %ld %ld %ld %ld %ld %ld",
		&r->id,
		&r->zone,
		&r->r_exit_reset[0], &r->r_exit_reset[1],
		&r->r_exit_reset[2], &r->r_exit_reset[3],
		&r->r_exit_reset[4], &r->r_exit_reset[5])
	!= 8)
      return -1;

    get_newline (f);

    fscanf (f, "0x%8lx:0x%8lx", &r->r_flags_reset.h, &r->r_flags_reset.l);
    get_newline (f);

    if (r->id >= ID_CNT_START &&
	!insert_entry (r->id, convroom (ct), &id_table)) {

      for (i = 1; i <= 2; i++) {
	while (getc (f) != '^') ;
      }
      get_newline (f);

      --r;
      --ct;
      --num_ld;
      continue;
    }
    r->temporary = False;
    r->touched = True;
    r->r_short = get_string (f);
    r->r_long = get_string (f);
    mem_used += strlen (r->r_short) + strlen (r->r_long) + 2;

    init_intset (&r->objects, 5);
    init_intset (&r->mobiles, 3);
    init_intset (&r->exits_to_me, 3);

    if (zone > -1)
      r->zone = zone;
    zadd_loc (convroom (ct), r->zone);

    mem_used += get_set_mem_usage (&r->objects)
      + get_set_mem_usage (&r->mobiles)
      + get_set_mem_usage (&r->exits_to_me);
  }

  numloc += num_ld;

  if (loaded != NULL)
    *loaded = num_ld;
  if (infile != NULL)
    *infile = num_infile;

  return mem_used;
}

static int
boot_objects (FILE * f, char *fname)
{
  int mem_used = 0;
  int new_mem;

  if (f == NULL)
    return -1;

  numobs = 0;
  objects = NEW (Object, obj_array_len = 500);

  mem_used += sizeof (Object) * 500;

  if ((new_mem = load_objects (-1, f, &num_const_obs, NULL)) < 0)
    return -1;

  return mem_used + new_mem;
}


/* Read object spesifications from a file and load them into the game in the
 * zone specified. If it is -1, then use the zone specifications given in
 * the file for each object sepeartely. Return the number of objects actually
 * read, the total number of objects in the file,
 * and the amount of any extra memory allocated as the function value.
 * (-1 on error).
 */
int
load_objects (int zone, FILE * f, int *loaded, int *infile)
{
  int j, ct, mem_used = 0;
  int num_ld, num_infile;
  char name[64], altname[64];
  long int link;

  j = fscanf (f, "%d", &num_infile);

  if (j != 1 || (num_ld = num_infile) <= 0) {
    return -1;
  }
  get_newline (f);

  if (numobs + num_infile > obj_array_len) {

    int oldlen = obj_array_len;

    if (numobs + num_infile > GLOBAL_MAX_OBJS) {
      return -1;
    }
    obj_array_len = 75 + numobs + num_infile;

    objects = resize_array (objects, sizeof (Object),
			    oldlen, obj_array_len);

    mem_used += sizeof (Object) * (obj_array_len - oldlen);
  }
  for (ct = numobs; ct < numobs + num_ld; ct++) {

    if (fscanf (f, "%s %s %d %ld %d "
		"%ld %d %d %ld "
		"%d %d %d "
		"0x%8lx:0x%8lx:0x%8lx"
		"%d %d %d %*d",
		name, altname, &ozone (ct), &obj_id (ct), &onum (ct),
	      &link, &ovis_reset (ct), &ocarrf_reset (ct), &oloc_reset (ct),
		&state_reset (ct), &odamage_reset (ct), &oarmor_reset (ct),
	      &obits_reset (ct).u, &obits_reset (ct).h, &obits_reset (ct).l,
		&omaxstate (ct), &ovalue_reset (ct), &osize_reset (ct))
	!= 18) {
      mudlog ("BOOTSTRAP: load_objects; fscanf() failed");
      return -1;
    }
    get_newline (f);

    /* If it already existed, skip it.
     */
    if (obj_id (ct) >= ID_CNT_START &&
	!insert_entry (obj_id (ct), ct, &id_table)) {

      for (j = 1; j <= 5; j++) {
	while (getc (f) != '^') ;
      }
      get_newline (f);
      get_newline (f);

      --ct, --num_ld;
      continue;
    }
    oname (ct) = COPY (name);
    oaltname (ct) = COPY (altname);

    if (link <= -1)
      olinked (ct) = -1;
    else if (link < numobs + num_infile)
      olinked (ct) = link;
    else if ((link = lookup_entry (link, &id_table)) != NOT_IN_TABLE) {
      olinked (ct) = link;
      olinked (link) = ct;
    } else
      olinked (ct) = -1;


    mem_used += strlen (name) + strlen (altname) + 2;

    for (j = 0; j < 4; j++) {
      olongt (ct, j) = get_string (f);
      mem_used += strlen (olongt (ct, j)) + 1;
    }

    if (zone == -1) {
      oexam_text (ct) = NULL;
      oexamine (ct) = ftell (f);

      for (j = 1; fgetc (f) != '^'; j++) ;
      if (j == 1)
	oexamine (ct) = 0;
    } else {
      oexam_text (ct) = get_string (f);
      oexamine (ct) = 0;
    }

    otemporary (ct) = False;

    init_intset (oinv (ct),
		 dtst_bit (&obits_reset (ct), OFL_CONTAINER) ? 10 : 0);

    if (zone > -1)
      ozone (ct) = zone;
    zadd_obj (ct, ozone (ct));

    mem_used += get_set_mem_usage (oinv (ct));
  }

  numobs += num_ld;

  if (loaded != NULL)
    *loaded = num_ld;
  if (infile != NULL)
    *infile = num_infile;

  return mem_used;
}


static int
boot_pflags (FILE * f, char *fname)
{
  WORLD_REC *w = &the_world_rec;
  PFLAGS p[16];
  int x, y;

  if (f == NULL)
    return -1;
  if (!read_pflags (f, p)) {
    printf ("\nIllegal syntax in %s.\n", fname);
    return -1;
  }
  for (x = y = 0; x < 8; ++x) {
    w->w_pflags[x] = p[y++];
    w->w_mask[x] = p[y++];
  }
  return 0;
}

static int
boot_verbs (FILE * f, char *fname)
{
  int v, ct, mem_used;
  char x[64];

  if (f == NULL)
    return -1;
  fscanf (f, "%d", &v);
  mem_used = sizeof (char *) * (v + 1);
  mem_used = sizeof (int) * (v + 1);

  verbtxt = NEW (char *, v + 1);
  verbnum = NEW (int, v + 1);

  for (ct = 0; ct < v; ct++) {
    fscanf (f, "%s %d", x, &verbnum[ct]);
    verbtxt[ct] = COPY (x);
    mem_used += strlen (x) + 1;
  }
  verbtxt[ct] = NULL;
  return mem_used;
}


static int
boot_zones (FILE * f, char *fname)
{
  int ct, mem_used;
  char x[64];

  if (f == NULL)
    return -1;

  fgets (x, sizeof x, f);

  numzon = num_const_zon = atoi (x);
  zon_array_len = num_const_zon + 20;

  zoname = NEW (ZONE, zon_array_len);

  mem_used = sizeof (ZONE) * zon_array_len;

  for (ct = 0; ct < num_const_zon; ct++) {

    fscanf (f, "%s %*d", x);
    zname (ct) = COPY (x);

    init_intset (zlocs (ct), 15);
    init_intset (zmobs (ct), 5);
    init_intset (zobjs (ct), 10);

    ztemporary (ct) = False;

    mem_used += strlen (x) + 1;
    /* add the space for the intsets later */
  }

  return mem_used;
}


static char *
id_counter_file ()
{
  return ID_COUNTER;
}


/* Boot ID counter and initialize ID table.
 */
static int
boot_id_counter (void)
{
  int mem_used = 0;

  FILE *f = fopen (id_counter_file (), "r");

  printf ("  ID-table & ID-counter ");

  if (f == NULL) {
    printf ("(Creating %s)", id_counter_file ());

    if ((f = fopen (id_counter_file (), "w")) == NULL) {
      printf ("fopen: Bootstrap failed for \"%s\".\n",
	      id_counter_file ());

      perror ("fopen");
      exit (1);
    }
    fprintf (f, "%ld", id_counter = ID_CNT_START);
  } else {
    int status = fscanf (f, "%ld", &id_counter);

    if (status != 1 || id_counter < ID_CNT_START) {
      printf ("Erroneous contents in %s\n", id_counter_file ());
      exit (1);
    }
  }

  fclose (f);

  init_inttable (&id_table, 1024);

  printf ("used %d bytes.\n", mem_used = get_table_mem_usage (&id_table));
  return mem_used;
}


Boolean
save_id_counter (void)
{
  FILE *f;
  int status;

  if ((f = fopen (id_counter_file (), "w")) == NULL) {
    progerror (id_counter_file ());
    return False;
  }
  status = fprintf (f, "%ld\n", id_counter);

  fclose (f);

  if (status == EOF) {
    progerror (id_counter_file ());
    return False;
  }
  return True;
}


/* Is this a character legal in a Pflag ?
 */
static Boolean
islegal (int c)
{
  return isalpha (c) || c == '/';
}

/*
 * **  Read pflags.
 */
static Boolean
read_pflags (FILE * F, PFLAGS p[])
{
  int x;			/* 0 = PX, MX, PW, MW, PA, MA, PD, MD, PG, MG */
  int y;
  char *s;
  PFLAGS f;
  int k, v;
  int c;
  int w;
  int i;
  char b[160];

  f.u = f.l = f.h = 0;
  w = -1;
  for (i = 0; i < 16; i++) {
    p[i].u = p[i].h = p[i].l = 0;
  }
  for (k = getc (F); k != EOF; k = getc (F)) {
    if (!islegal (k))
      continue;

    for (s = b; islegal (k); k = getc (F))
      *s++ = k;

    *s = 0;

    if (k != ':' || (x = tlookup (b, PFT)) == -1)
      return False;

    if (x >= 16) {
      while ((k = getc (F)) != EOF && k != '\n') ;
      if (k == EOF)
	break;
      continue;
    }
    if (x < w)
      return False;

    y = 0;
    w = x;

    while ((c = getc (F)) != ';') {
      if (c == EOF)
	return False;
      if (c == '+') {
	y = 1;
	continue;
      } else if (c == '-') {
	y = -1;
	continue;
      } else if (!islegal (c)) {
	y = 0;
	continue;
      }
      for (s = b; islegal (c); c = getc (F)) {
	*s++ = c;
      }
      ungetc (c, F);
      *s = 0;
      if ((v = tlookup (b, Pflags)) == -1) {
	printf ("\nUnknown Pflag: %s.", b);
	return False;
      }
      if (v >= 64) {
	v -= 64;
	k = 1 << v;
	if (y > 0) {
	  p[w].u |= k;
	} else if (y < 0) {
	  p[w].u &= ~k;
	} else if ((w & 1) == 0) {
	  for (i = w; i < 16; i++) {
	    p[i].u |= k;
	  }
	} else {
	  for (i = w; i < 16; i += 2) {
	    p[i].u |= k;
	  }
	}
      }
      if (v >= 32) {
	v -= 32;
	k = 1 << v;
	if (y > 0) {
	  p[w].h |= k;
	} else if (y < 0) {
	  p[w].h &= ~k;
	} else if ((w & 1) == 0) {
	  for (i = w; i < 16; i++) {
	    p[i].h |= k;
	  }
	} else {
	  for (i = w; i < 16; i += 2) {
	    p[i].h |= k;
	  }
	}
      } else {
	k = 1 << v;
	if (y > 0) {
	  p[w].l |= k;
	} else if (y < 0) {
	  p[w].l &= ~k;
	} else if ((w & 1) == 0) {
	  for (i = w; i < 16; i++) {
	    p[i].l |= k;
	  }
	} else {
	  for (i = w; i < 16; i += 2) {
	    p[i].l |= k;
	  }
	}
      }
      y = 0;
    }
  }

  return (w >= 0);
}

static char *
get_string (FILE * f)
{
  char *y, x[4096];

  for (y = x; (*y = fgetc (f)) != '^'; y++) ;
  *y = 0;
  get_newline (f);
  return COPY (x);
}
