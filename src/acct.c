
/* Accounting Routines by Thrace
 * (countrooms(), countmobiles(), countobjects(), acctcom(), dopc())
 */

/* Spiffed up and more information added by Illusion
 * Acct <zone> added by Illusion
 */

/* objectscom()
 * 1995 by Illusion
 */

/* mstatcom()
 * 1995 by Illusion
 */

#include <stdlib.h>
#include <strings.h>
#include "kernel.h"
#include "mudmacros.h"
#include "locations.h"
#include "objects.h"
#include "mobiles.h"
#include "pflags.h"
#include "oflags.h"
#include "lflags.h"
#include "cflags.h"
#include "sflags.h"
#include "bprintf.h"
#include "parse.h"
#include "acct.h"
#include "zones.h"
#include "mobile.h"
#include "objsys.h"
#include "rooms.h"
#include "timing.h"
#include "uaf.h"
#include "flags.h"
#include "rooms.h"
#include "objsys.h"
#include "zones.h"

#define Line1 "============================================================="
#define Line2 "-------------------------------------------------------------"

extern char *WizLevels[];

void
acctcom (void)
{
  char text[80];
  int zone = -1;
  int i;

  if (plev (mynum) < LVL_WIZARD) {
    bprintf ("You can't do that now.\n");
    return;
  }
  if (brkword () != -1) {
    if ((zone = get_zone_by_name (wordbuf)) == -1) {
      bprintf ("No Such Zone: %s\n", wordbuf);
      return;
    }
  }
  if (zone == -1)
    sprintf (text, "Current System Accounting");
  else
    sprintf (text, "Current System Accounting (Zone: %s)", zname (zone));

  for (i = 0; i < (60 - strlen (text)) / 2; ++i)
    bprintf (" ");

  bprintf ("&+W%s\n", text);
  bprintf ("&+C%s\n", Line1);

  countrooms (zone);
  bprintf ("\n");
  countmobiles (zone);
  bprintf ("\n");
  countobjects (zone);
}

/* Show percentage: max is maximum, exist is how many of max are left */
int
dopc (int max, int exist)
{
  int proc;
  int t;

  if (exist > max) {		/* Someone passed them backwards */
    t = max;			/* so we fix that here. */
    max = exist;
    exist = t;
  }
  if (!(max))
    proc = 0;
  else
    proc = (exist * 100) / max;

  return (proc);
}

void
countmobiles (int zone)
{
  int bas, j = 0, i;
  int a;
  int NoHassle, Possessed, DeadMobs;
  int proc;

  NoHassle = Possessed = DeadMobs = 0;
  a = max_players;

  if (zone == -1) {
    bas = numchars;
  } else {
    a = 0;
    bas = znumchars (zone);
  }

  for (; a < bas; a++) {
    i = (zone == -1) ? a : zmob_nr (a, zone);

    if (EMPTY (pname (i)))
      continue;

    if (ptstflg (i, PFL_NOHASSLE))
      NoHassle++;

    if (ptstflg (i, SFL_OCCUPIED))
      Possessed++;

    if (alive (i) != -1)
      j++;

    if (zone == -1)
      DeadMobs = ((numchars - max_players) - j);
    else
      DeadMobs = (znumchars (zone) - j);
  }
  if (zone == -1)
    proc = dopc (numchars - max_players, j);
  else
    proc = dopc (znumchars (zone), j);

  if (zone == -1)
    bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Total Mobiles", numchars - max_players, "Living Mobiles", j);
  else
    bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Total Mobiles", znumchars (zone), "Living Mobiles", j);

  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n",
	   "Dead Mobiles", DeadMobs, "Percent Living", proc);
  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n",
	   "Possessed", Possessed, "NoHassle", NoHassle);
}

void
countobjects (int zone)
{
  int proc;
  long totalval = 0;
  int aa, oc = 0;
  int destroyed, weapons, armor, food;
  int lightable, container, key, noget;
  int destweap, destarm, destfood, destlight, destcont, destkey;
  int objnum;

  destroyed = weapons = armor = food = lightable
    = container = key = noget = destweap = destarm = destfood =
    destlight = destcont = destkey = 0;

  if (zone == -1) {
    objnum = numobs;
  } else {
    aa = zfirst_obj (zone);
    objnum = znumobs (zone);
  }

  for (aa = 0; aa < objnum; aa++) {
    if (zone != -1)
      oc = zobj_nr (aa, zone);

    if (!otstbit (oc, OFL_NOGET) && !otstbit (oc, OFL_DESTROYED))
      totalval += ovalue (oc);

    if (otstbit (oc, OFL_NOGET))
      noget++;

    if (otstbit (oc, OFL_DESTROYED))
      destroyed++;

    if (otstbit (oc, OFL_WEAPON)) {
      weapons++;
      if (otstbit (oc, OFL_DESTROYED))
	destweap++;
    }
    if (otstbit (oc, OFL_ARMOR)) {
      armor++;
      if (otstbit (oc, OFL_DESTROYED))
	destarm++;
    }
    if (otstbit (oc, OFL_FOOD)) {
      food++;
      if (otstbit (oc, OFL_DESTROYED))
	destfood++;
    }
    if (otstbit (oc, OFL_CONTAINER) &&
	!otstbit (oc, OFL_NOGET)) {
      container++;
      if (otstbit (oc, OFL_DESTROYED))
	destcont++;
    }
    if (otstbit (oc, OFL_KEY)) {
      key++;
      if (otstbit (oc, OFL_DESTROYED))
	destkey++;
    }
    if (otstbit (oc, OFL_LIGHTABLE)) {
      lightable++;
      if (otstbit (oc, OFL_DESTROYED))
	destlight++;
    }
    if (zone == -1)
      oc++;
  }

  if (zone == -1)
    proc = dopc (oc - destroyed, oc);
  else
    proc = dopc (znumobs (zone) - destroyed, znumobs (zone));

  if (zone == -1)
    bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Total Objects", oc, "Remaining", oc - destroyed);
  else
    bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Total Objects", znumobs (zone), "Remaining", znumobs (zone) - destroyed);

  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Destroyed", destroyed, "Percent Left", proc);
  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n\n", "NoGet", noget, "Value of Objects", totalval);
  bprintf ("&+W         Detail List of Objects Available to Mortals\n");
  bprintf ("&+C%s\n", Line2);
  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Weapons", weapons, "Destroyed", destweap);
  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Armor", armor, "Destroyed", destarm);
  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Food", food, "Destroyed", destfood);
  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Containers", container, "Destroyed", destcont);
  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Lightable", lightable, "Destroyed", destlight);
  bprintf ("&+w%-18s: %8d\t%-18s: %8d\n", "Keys", key, "Destroyed", destkey);
  bprintf ("&+C%s\n", Line1);
}

void
countrooms (int zone)
{
  int roomct;

  if (zone == -1)
    roomct = num_const_locs;
  else
    roomct = znumloc (zone);

  bprintf ("&+w%-18s: %8d\n", "Total Rooms", roomct);
}

/* objectscom()
 * 1995 by Illusion
 */
void
objectscom (void)
{
  int zone = -1;
  int i, c, oc = 0;
  char b[60], d[60];

  if (plev (mynum) < LVL_WIZARD) {
    bprintf ("You can't do that now.\n");
    return;
  }
  if (brkword () != -1) {
    if ((zone = get_zone_by_name (wordbuf)) == -1) {
      bprintf ("No Such Zone: %s\n", wordbuf);
      return;
    }
  }
  if (zone == -1) {
    for (i = 0; i < numobs; ++i) {
      oc++;
      c = findzone (oloc (i), b);
      sprintf (d, "%s%d", b, c);

      if (ocarrf (i) >= CARRIED_BY)
	strcpy (d, "Carried");
      else if (ocarrf (i) == IN_CONTAINER)
	strcpy (d, "In Container");

      bprintf ("&+w%-12s&+W%-13s", oname (i), d);
      if (oc % 3 == 0)
	bprintf ("\n");
    }
    if (oc % 3 != 0)
      bprintf ("\n");

    bprintf ("\n&+wTotal of &+W%d &+wobjects.\n", oc);
    return;
  }
  bprintf ("&+wObject List For Zone: &+W%s\n", zname (zone));

  for (i = zfirst_obj (zone); i != SET_END; i = znext_obj (zone)) {
    oc++;
    c = findzone (oloc (i), b);
    sprintf (d, "%s%d", b, c);

    if (ocarrf (i) >= CARRIED_BY)
      strcpy (d, "Carried");
    else if (ocarrf (i) == IN_CONTAINER)
      strcpy (d, "In Container");

    bprintf ("&+w%-12s&+W%-14s", oname (i), d);
    if (oc % 3 == 0)
      bprintf ("\n");
  }
  if (oc % 3 != 0)
    bprintf ("\n");

  bprintf ("\n&+wTotal of &+W%d &+wobjects.\n", oc);
}

/* mstatcom()
 * Status for mortals
 * 1995 by Illusion
 */
void
mstatcom ()
{
  int i, j = 0;
  char cond[50];
  char buff[100];

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  bprintf ("&+CLvl Name          Score  Str/Max Mag/Max DP AC Cnd Who/Where\n");
  bprintf ("&+b--------------------------------------------------------------------------------\n");
  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && plev (i) < LVL_WIZARD) {
      ++j;

      if (pfighting (i) >= 0)
	sprintf (cond, "&+RFgt &+w%s: %d (%s)", pname (pfighting (i)), pstr (pfighting (i)), xshowname (buff, ploc (i)));
      else if (phelping (i) >= 0)
	sprintf (cond, "&+BHlp &+w%s (%s)", pname (phelping (i)), xshowname (buff, ploc (i)));
      else if (psitting (i))
	sprintf (cond, "&+CSit &+w(%s)", xshowname (buff, ploc (i)));
      else
	sprintf (cond, "&+wStd &+w(%s)", xshowname (buff, ploc (i)));

      bprintf ("&+w%3d &+W%-13.13s &+c%-6d %s%3d&+C/&+W%-3d %s%3d&+C/&+W%-3d &+B%2d %2d &+w%-32.32s\n",
	       plev (i), pname (i), pscore (i),
	       str_color (i), pstr (i), maxstrength (i),
	       mag_color (i), pmagic (i), maxmagic (i), player_damage (i),
	       player_armor (i), cond);
    }
  }
  if (!j)
    bprintf ("&+W No Mortals On-Line\n");
  bprintf ("&+b--------------------------------------------------------------------------------\n");
}

/* The USERS command. Show names and locations of users. Possibly also
 * their local host if PFL_SHUSERS is set.
 */
void
usercom (void)
{
  char locnam[256], wiznam[128], src[256], buff[128], idlebuff[64];
  char header[100], levnam[40];
  char occ[3];
  int i;
  int a[256], a_len = 0;
  int me = real_mynum;

  for (i = 0; i < max_players; i++)
    if (!check_nooracle (i) && is_in_game (i) &&
	(pvis (i) <= plev (me) || i == me))
      a[a_len++] = i;

  qsort (a, a_len, sizeof (int), cmp_player);

  sprintf (header, "%-*s %-15s%s %10.10s  %s",
	   PNAME_LEN,
	   "User",
	   plev (me) >= LVL_WIZARD ? "Level     Vis" : "Level",
	   "Sex",
	   "Idle",
	   plev (me) >= LVL_WIZARD ? "Location" : " ");

  bprintf ("&+C%s\n", header);
  bprintf ("&+b------------------------------------------");
  if (plev (me) >= LVL_WIZARD)
    bprintf ("&+b-------------------------------------\n");
  else
    bprintf ("\n");

  for (i = 0; i < a_len; ++i) {

    if (ptstflg (mynum, PFL_SEEIDLE)) {
      strcpy (idlebuff, sec_to_hhmmss (global_clock - prlast_cmd (a[i])));
    } else {
      strcpy (idlebuff, sec_to_hhmmss (global_clock - plast_cmd (a[i])));
    }

    if (plev (a[i]) <= LVL_WIZARD)
      sprintf (levnam, "%s",
	       psex (a[i]) ? FLevels[plev (a[i])] : MLevels[plev (a[i])]);
    else
      sprintf (levnam, "%s", WizLevels[wlevel (plev (a[i]))]);

    if (!exists (ploc (a[i]))) {
      sprintf (locnam, "Not In Universe &+w(%d)&*", ploc (a[i]));
      sprintf (wiznam, "[Unknown]");
    } else {
      sprintf (wiznam, "%s", xshowname (buff, ploc (a[i])));
      strip_color (buff, sdesc (ploc (a[i])));
      sprintf (locnam, "%s", buff);
    }

    if (ststflg (a[i], SFL_OCCUPIED))
      strcpy (occ, "*");
    else
      strcpy (occ, " ");

    sprintf (src, "%-6d %6d", plev (a[i]), pvis (a[i]));

    bprintf ("&+w%-*s %-15s  &+w%c %10.10s %s%s %c%-.*s%c\n",
	     PNAME_LEN,
	     pname (a[i]),
	     plev (me) >= LVL_WIZARD ? src : levnam,
	     psex (a[i]) ? 'F' : 'M',
	     idlebuff,
	     plev (me) >= LVL_WIZARD ? occ : " ",
	     plev (me) >= LVL_WIZARD ? wiznam : "",
	     plev (me) >= LVL_WIZARD ? '(' : ' ',
      plev (me) >= LVL_WIZARD ? 79 - (PNAME_LEN + 35 + strlen (wiznam)) : 0,
	     plev (me) >= LVL_WIZARD ? locnam : "",
	     plev (me) >= LVL_WIZARD ? ')' : ' ');
  }
  bprintf ("&+b------------------------------------------");

  if (plev (me) >= LVL_WIZARD)
    bprintf ("&+b-------------------------------------\n");
  else
    bprintf ("\n");

}

/* Checks to see if we need to do a newline on the title line.
 */
int
do_who_newline (int total, char *message)
{
  char buffer[400];
  int len;

  strip_color (buffer, message);
  len = strlen (buffer);
  total += len;
  if (total >= 59) {
    total = 0;
    bprintf ("&>");
  }
  bprintf ("%s", message);
  return total;
}

/* Displays and formats messages for WHO/WHON
 */
void
do_who_messages (int cur_len, int plr)
{
  char awaymsg[100];
  int len = cur_len;

  if (ststflg (plr, SFL_NOGOSSIP))
    len = do_who_newline (len, "&+B[&*NoGossip&+B] ");
  if (ststflg (plr, SFL_NOANON))
    len = do_who_newline (len, "&+B[&*NoAnon&+B] ");
  if (ststflg (plr, SFL_BUSY))
    len = do_who_newline (len, "&+B[&*Busy&+B] ");
  if (ststflg (plr, SFL_CODING))
    len = do_who_newline (len, "&+B[&*Coding&+B] ");
  if (players[plr].newplr)
    len = do_who_newline (len, "&+C[&+WNew Player&+C] ");

  if (plev (mynum) >= LVL_WIZARD) {
    if (ststflg (plr, SFL_QUIET))
      len = do_who_newline (len, "&+B[&*Quiet&+B] ");
    if (ststflg (plr, SFL_NOWIZ))
      len = do_who_newline (len, "&+B[&*NoWiz&+B] ");
    if (ststflg (plr, SFL_NOWISH))
      len = do_who_newline (len, "&+B[&*NoWish&+B] ");
  }
  if (plev (mynum) > LVL_PROPHET) {
    if (ststflg (plr, SFL_NOPROPHET))
      len = do_who_newline (len, "&+B[&*NoProphet&+B] ");
  }
  if (plev (mynum) > LVL_ARCHWIZARD) {
    if (ststflg (plr, SFL_NOAWIZ))
      len = do_who_newline (len, "&+B[&*NoAWiz&+B] ");
  }
  if (plev (mynum) > LVL_ADVISOR) {
    if (ststflg (plr, SFL_NOADV))
      len = do_who_newline (len, "&+B[&*NoAdv&+B] ");
  }
  if (plev (mynum) > LVL_AVATAR) {
    if (ststflg (plr, SFL_NOGOD))
      len = do_who_newline (len, "&+B[&*NoGod&+B] ");
  }
  if (plev (mynum) > LVL_GOD) {
    if (ststflg (plr, SFL_NOUPPER))
      len = do_who_newline (len, "&+B[&*NoUpper&+B] ");
  }
  if (ststflg (plr, SFL_AWAY)) {
    sprintf (awaymsg, "&+B[&*Away&+B] [&+CReason: &*%s&+B] ",
	     players[plr].awaymsg);
    len = do_who_newline (len, awaymsg);
  }
}

/* The WHO/WHON command
 */
void
whocom (Boolean do_name)
{
  char buffer[400];
  int i, cur_len;
  int a[256], a_len = 0;

  for (i = 0; i < max_players; i++)
    if (is_in_game (i) && (pvis (i) <= plev (mynum) || i == mynum)) {
      a[a_len++] = i;
    }
  qsort (a, a_len, sizeof (int), cmp_player);

  bprintf ("&+C%-16.16s  Title\n", do_name ? "Name" : "Level");
  bprintf ("&+b---------------- --------------------------------------------------------------\n");

  for (i = 0; i < a_len; ++i) {
    if (do_name) {
      bprintf ("&+B[&+W%12s&+B]", pname (a[i]));
    } else {
      if (ststflg (a[i], SFL_FREAQ))
	bprintf ("&+B[&+G%12s&+B]", "FreaQ");
      else if (plev (a[i]) <= LVL_WIZARD)
	bprintf ("&+B[&+w%12s&+B]", psex (a[i]) ?
		 FLevels[plev (a[i])] : MLevels[plev (a[i])]);
      else
	bprintf ("&+B[&+W%12s&+B]", WizLevels[wlevel (plev (a[i]))]);
    }

    bprintf ("%s", pvis (a[i]) > 0 ? "&+R*" : " ");

    if (plev (mynum) < LVL_WIZARD)
      bprintf ("  ");
    else
      bprintf ("%s ", is_aliased (a[i]) ? "&+C*" : " ");

    if (ststflg (a[i], SFL_FREAQ)) {
      cur_len = strlen (pname (a[i]) + 33);
      bprintf ("&+w%s, tHe ToTAL aNd AbSoLuTE Frea&+GQ&*! ", pname (a[i]));
    } else {
      strip_color (buffer, make_title (ptitle (a[i]), pname (a[i])));
      cur_len = strlen (buffer) + 1;
      bprintf ("&+w%s ", make_title (ptitle (a[i]), pname (a[i])));
    }

    if (!ststflg (mynum, SFL_NOFLAG)) {
      do_who_messages (cur_len, a[i]);
    }
    bprintf ("\n");
  }
  bprintf ("&+b---------------- --------------------------------------------------------------\n");

  bprintf ("&+wThere %s &+C%d &+wvisible player%s currently on the game.\n",
	   (a_len > 1 ? "are" : "is"), a_len, (a_len > 1 ? "s" : ""));
}

/* The MWHO command
 */
#define COLUMNS (72/MNAME_LEN)
void
mwhocom (void)
{
  char buff[64];
  int i, ct, zone, nr;
  Boolean list_all = True;
  int first = max_players;
  int last = numchars;

  i = ct = zone = nr = 0;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if (brkword () != -1) {
    list_all = False;

    if ((zone = get_zone_by_name (wordbuf)) == -1) {

      bprintf ("No such zone: %s\n", wordbuf);
      return;
    }
    first = 0;
    last = znumchars (zone);
  }
  bprintf ("Live Mobiles\n------------\n\n");

  for (ct = first, nr = 0; ct < last; ++ct) {

    i = list_all ? ct : zmob_nr (ct, zone);

    if (!EMPTY (pname (i)) && pstr (i) >= 0 &&
	(pvis (i) <= 0 || pvis (i) <= plev (mynum))) {

      ++nr;

      if (pvis (i) > 0)
	sprintf (buff, "(%s)", pname (i));
      else
	strcpy (buff, pname (i));

      bprintf ("%-*s%c",
	       PNAME_LEN + 3, buff, (nr % COLUMNS) ? ' ' : '\n');
    }
  }

  bprintf ("\n\nThere are %d mobiles left (from %d)\n",
	   nr, last - first);
}

/* The STATS command WITHOUT arguments.
 */
static void
ustats (void)
{
  static char *WizLevs[] =
  {
    NULL, "Appr", "Dorq", "Emrt", "Wiz", "Prpt",
    "AWiz", "Advr", "Avtr", "God", "MUsr"};

  int i, j;
  int a[256], a_len = 0;
  char buff[256], loginbuff[64], idlebuff[64];

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  for (i = 0; i < max_players; i++)
    if (is_in_game (i) && (pvis (i) <= plev (mynum) || i == mynum))
      a[a_len++] = i;

  qsort (a, a_len, sizeof (int), cmp_player);

  bprintf ("&+CLevel  Name         Str/Max Mag/Max DP AC Vis     Idle   On For Location\n");
  bprintf ("&+b-------------------------------------------------------------------------------\n");

  for (j = 0; i = a[j], j < a_len; ++j) {
    if (ststflg (i, SFL_FREAQ))
      bprintf ("&+w&+B[&+G%4.4s&+B]", "FreQ");
    else if (plev (i) <= LVL_WIZARD)
      bprintf ("&+w&+B[&+w%4d&+B]", plev (i));
    else
      bprintf ("&+w&+B[&+W%4.4s&+B]", WizLevs[wlevel (plev (i))]);

    bprintf (" &+w%-*s", PNAME_LEN, pname (i));

    if (plev (i) < LVL_WIZARD)
      bprintf (" %s%3d&+C/&+W%-3d %s%3d&+C/&+W%-3d &+B%2d %2d ",
	       str_color (i), pstr (i), maxstrength (i),
	       mag_color (i), pmagic (i), maxmagic (i),
	       player_damage (i),
	       player_armor (i) > MAXARMOR ?
	       MAXARMOR : player_armor (i));
    else
      bprintf (" &+w---&+C/&+w--- ---&+C/&+w--- -- -- ");

    if (pvis (i) < 999)
      bprintf ("&+w%3d", pvis (i));
    else
      bprintf ("&+WInv&+w");
    if (ptstflg (mynum, PFL_SEEIDLE)) {
      strcpy (idlebuff, sec_to_hhmmss (global_clock - prlast_cmd (i)));
    } else {
      strcpy (idlebuff, sec_to_hhmmss (global_clock - plast_cmd (i)));
    }

    bprintf (" %8.8s", idlebuff);

    strcpy (loginbuff, sec_to_hhmmss (global_clock - plogged_on (i)));
    bprintf (" %8.8s", loginbuff);

    bprintf (" %-15.15s\n", xshowname (buff, ploc (i)));
  }
  bprintf ("&+b-------------------------------------------------------------------------------\n");
}

/* The STATS command
 */
void
showplayer (void)
{
  extern char *Mflags[];
  char buff[256];
  PERSONA d;
  int b;
  int max_str, max_mag;
  int armor;
  long w;
  char *title, *name;
  Boolean in_file, is_mobile;
  Boolean is_female;

  if (!ptstflg (mynum, PFL_STATS)) {
    erreval ();
    return;
  }
  if (brkword () == -1) {
    ustats ();
    return;
  }
  if ((b = fpbns (wordbuf)) != -1 && seeplayer (b)) {
    in_file = False;
    is_mobile = b >= max_players;
    player2pers (&d, NULL, b);
    name = pname (b);
    title = is_mobile ? NULL : ptitle (b);
    d.p_home = find_loc_by_id (is_mobile ? ploc_reset (b) : phome (b));
    max_str = is_mobile ? pstr_reset (b) : maxstrength (b);
    max_mag = is_mobile ? 0 : maxmagic (b);
    is_female = ststflg (b, SFL_FEMALE);
  } else if (getuaf (wordbuf, &d)) {
    in_file = True;
    is_mobile = False;
    title = d.p_title;
    name = d.p_name;
    max_str = pmaxstrength (d.p_level);
    max_mag = pmaxmagic (d.p_level);
    is_female = xtstbit (d.p_sflags.l, SFL_FEMALE);
  } else {
    bprintf ("Who's that?\n");
    return;
  }

  bprintf ("&+WName       &+C: &+w%s", name);

  if (!is_mobile) {
    bprintf ("\n&+WTitle      &+C: &+w%s", make_title (title, "<name>"));
    bprintf ("\n&+WScore      &+C: &+w%d", d.p_score);
    bprintf ("\n&+WLevel      &+C: &+w%d &+B(&*%s&+B)", d.p_level,
         ( d.p_level < 0 ) ? "Mobile\0" : 
	     d.p_level < LVL_WIZARD ? is_female ? FLevels[d.p_level]
	     : MLevels[d.p_level] : WizLevels[wlevel (d.p_level)]);
  }
  bprintf ("\n&+WStrength   &+C: &+w%d / %d", d.p_strength, max_str);

  if (!is_mobile)
    bprintf ("\n&+WMagic      &+C: &+w%d / %d", d.p_magic, max_mag);

  if (!in_file && (w = pwpn (b)) != -1 && iscarrby (w, b) && iswielded (w)) {
    int w_damage = is_mobile ? odamage (w) / 2 : odamage (w);

    w_damage += d.p_damage;

    bprintf ("\n&+WDamage     &+C: &+w%d &+B(&+CWielding: &*%s&+B)",
	     w_damage, oname (w));
  } else {
    bprintf ("\n&+WDamage     &+C: &+w%d", d.p_damage);
  }

  if (!in_file) {
    if (player_armor (b) > d.p_armor)
      bprintf ("\n&+WArmor      &+C: &+w%d / %d &+B(&+CWorn AC: &*%d&+B)",
	       player_armor (b), MAXARMOR, player_armor (b) - d.p_armor);
    else
      bprintf ("\n&+WArmor      &+C: &+w%d / %d", armor = d.p_armor, MAXARMOR);

  } else {
    bprintf ("\n&+WArmor      &+C: &+w%d / %d", armor = d.p_armor, MAXARMOR);
  }

  if (!is_mobile)
    bprintf ("\n&+WKill/Death &+C: &+w%d / %d &+B(&+CRatio: &*%d%%&+B)",
	     d.p_killed, d.p_died, make_kd_ratio (d.p_killed, d.p_died));

  bprintf ("\n&+WVisibility &+C: &+w%d", d.p_vlevel);

  if (is_mobile) {
    bprintf ("\n&+WAggression &+C: &+w%d %%", pagg (b));
    bprintf ("\n&+WSpeed      &+C: &+w%d", pspeed (b));
    bprintf ("\n&+W%s      &+C: &+w%s",
	     zpermanent (pzone (b)) ? "Zone " : "Owner", zname (pzone (b)));
  }
  bprintf ("\n&+WWimpy      &+C: &+w%d", d.p_wimpy);

  if (!in_file)
    bprintf ("\n&+WLanguage   &+C: &+w%s", Nflags[plang (b)]);

  if (exists (d.p_home))
    bprintf ("\n&+WStart      &+C: &+w%s &+B(&*%s&+B)",
	     sdesc (d.p_home), xshowname (buff, d.p_home));

  if (!in_file) {
    bprintf ("\n&+WLocation   &+C: &+w%s &+B(&*%s&+B)",
	     sdesc (ploc (b)), xshowname (buff, ploc (b)));

    bprintf ("\n&+WCondition  &+C:&+w");

    if (pfighting (b) >= 0)
      bprintf (" Fighting %s", pname (pfighting (b)));
    else if (phelping (b) >= 0)
      bprintf (" Helping %s", pname (phelping (b)));
    else if (psitting (b))
      bprintf (" Sitting");
    else
      bprintf (" Standing");

    if (is_mobile) {
      bprintf ("\n&+WBehavior   &+C: &+w");
      show_bits ((int *) &mflags (b), sizeof (MFLAGS) / sizeof (int), Mflags);
    } else if (ptstflg (mynum, PFL_SHUSER)) {
      bprintf ("\n&+WFrom Host  &+C: &+w%s",
	       !ptstflg (mynum, PFL_SEEUSER) ? players[b].hostname :
	       players[b].usrname);
    }
  } else {
    bprintf ("\n&+WLast on    &+C: &+w%s", time2ascii (d.p_last_on));
    if (ptstflg (mynum, PFL_SHUSER))
      bprintf ("\n&+WFrom Host  &+C: &+w%s",
	       !ptstflg (mynum, PFL_SEEUSER) ? d.p_last_host : d.p_usrname);
  }

  if ((d.p_sflags.l != 0) || (d.p_sflags.h != 0)) {
    if (!is_mobile)
      bprintf ("\n");

    bprintf ("&+WVarious    &+C: &+w");

    if (plev (mynum) <= d.p_level && b != mynum)
      bprintf ("Unknown\n");
    else {
      bprintf ("\n");
      show_bits ((int *) &(d.p_sflags), sizeof (SFLAGS) / sizeof (int), Sflags);
    }
  }
  bprintf ("\n");
}

/* The MOBILE command
 */
void
mobilecom (void)
{
  int i, ct, zone, live, dead;
  Boolean list_all = True;
  int first = max_players;
  int last = numchars;

  i = ct = zone = live = dead = 0;

  if (plev (mynum) < LVL_WIZARD) {
    erreval ();
    return;
  }
  if (brkword () != -1) {
    list_all = False;

    if ((zone = get_zone_by_name (wordbuf)) == -1) {

      bprintf ("No such zone: %s\n", wordbuf);
      return;
    }
    first = 0;
    last = znumchars (zone);
  }
  bprintf ("Mobiles");

  if (the_world->w_mob_stop)
    bprintf ("   [Currently STOPped]");

  bprintf ("\n\n");

  for (ct = first; ct < last; ++ct) {

    i = list_all ? ct : zmob_nr (ct, zone);

    bprintf ("[%d] %-*s %c", i + GLOBAL_MAX_OBJS, MNAME_LEN,
	     EMPTY (pname (i)) ? pname_reset (i) : pname (i),
	     ststflg (i, SFL_OCCUPIED) ? '*' : ' ');

    if (EMPTY (pname (i))) {
      bprintf ("<QUIT or KICKED OFF>\n");
      ++dead;
    } else {
      desrm (ploc (i), IN_ROOM);

      if (pstr (i) < 0)
	++dead;
      else
	++live;
    }
  }

  bprintf ("\nA total of %d live mobile(s) + %d dead = %d.\n",
	   live, dead, live + dead);
}
