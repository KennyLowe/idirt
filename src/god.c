
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "kernel.h"
#include "locations.h"
#include "sendsys.h"
#include "wizlist.h"
#include "bprintf.h"
#include "actions.h"
#include "pflags.h"
#include "sflags.h"
#include "mobile.h"
#include "timing.h"
#include "fight.h"
#include "rooms.h"
#include "verbs.h"
#include "parse.h"
#include "log.h"
#include "god.h"
#include "uaf.h"
#include "mud.h"

void
deletecom ()
{
  if (plev (mynum) < LVL_AVATAR) {
    erreval ();
    return;
  }
  if (brkword () == -1 || strlen (wordbuf) > PNAME_LEN) {
    bprintf ("Delete who?\n");
    return;
  }
  mudlog ("DELETE: %s deleted %s", pname (mynum), wordbuf);

  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_AVATAR, LVL_MAX, mynum,
	    NOBODY, "&+B[&+W\001p%s\003 &*has &+Cdeleted &+W%s&+B]\n",
	    pname (mynum), wordbuf);

  deluaf (wordbuf);
  update_wizlist (wordbuf, LEV_MORTAL);

  bprintf ("Deleted %s.\n", wordbuf);
}

/* The OPENGAME command.
 */
void
opengamecom ()
{
  if (!ptstflg (mynum, PFL_SHUTDOWN)) {
    erreval ();
    return;
  }
  if (unlink (NOLOGIN) < 0) {
    bprintf ("The MUD is already open.\n");
  } else {
    mudlog ("SYSTEM: Opengame by %s", pname (mynum));
    bprintf ("MUD is now &+WOpen&*.\n");

    send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_WIZARD, LVL_MAX,
	      mynum, NOBODY, "&+B[&+COpengame &*by &+W\001p%s\003&+B]\n",
	      pname (mynum));
  }
}

void
shutdowncom (Boolean crash)
{
  FILE *nologin_file;
  char s[MAX_COM_LEN];
  char pidfile[50];
  int i;
  char *t = "iDiRT is currently unavailable, please try again later.\n";
  char *gt = "iDiRT Daemon is shutting down.\n";

  if (!ptstflg (mynum, PFL_SHUTDOWN)) {
    erreval ();
    return;
  }
  getreinput (s);

  if (!EMPTY (s))
    t = s;

  if ((nologin_file = fopen (NOLOGIN, "w")) == NULL) {
    bprintf ("&#&+WUnable to write NOLOGIN file&*");
    mudlog ("ERROR: Could not write NOLOGIN file");
  } else {
    fprintf (nologin_file, "%s\n", t);
    fclose (nologin_file);
  }

  bprintf ("MUD is now &+WClosed&*%s\n",
	   crash ? ", and is being &+RShutdown&*." : ".");

  mudlog ("SYSTEM: %s by %s", crash ? "Crash" : "Shutdown", pname (mynum));

  send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	    "&+B[&+C%s &*by &+W\001p%s\003&+B]\n",
	    crash ? "Crash" : "Shutdown", pname (mynum));

  for (i = 0; i < max_players; i++) {
    if (is_in_game (i) && plev (i) < LVL_GOD) {
      sendf (i, "MUD is now &+WClosed&*%s\n",
	     crash ? ", and is being &+RShutdown&*." : ".");
      p_crapup (i, t, CRAP_SAVE | CRAP_UNALIAS | CRAP_RETURN);
    }
  }

  if (crash) {
    for (i = 0; i < max_players; i++) {
      if (is_in_game (i)) {
	p_crapup (i, gt, CRAP_SAVE | CRAP_UNALIAS | CRAP_RETURN);
      }
    }
    sprintf (pidfile, "%spid", data_dir);
    remove (pidfile);
    exit (0);
  }
}

void
bresetcom (void)
{
  if (!ptstflg (mynum, PFL_REBOOT)) {
    erreval ();
    return;
  }
  if (breset) {
    bprintf ("Cancelling reboot for next full reset.\n");
    mudlog ("BRESET: BootReset turned off by %s", pname (mynum));
    breset = False;
  } else {
    bprintf ("The MUD will now reboot during the next full reset.\n");
    mudlog ("BRESET: BootReset turned on by %s", pname (mynum));
    breset = True;
  }
}

void
idlecom (void)
{
  if (plev (mynum) < LVL_AVATAR) {
    bprintf ("Pardon?\n");
    return;
  }
  if (ptstflg (mynum, PFL_IDLE)) {
    bprintf ("You will no longer pretend to be idle.\n");
    pclrflg (mynum, PFL_IDLE);
    return;
  } else {
    bprintf ("You are now pretending to be idle.\n");
    psetflg (mynum, PFL_IDLE);
    return;
  }
}

void
seeidlecom (void)
{
  if (plev (mynum) < LVL_GOD) {
    bprintf ("Pardon?\n");
    return;
  }
  if (ptstflg (mynum, PFL_SEEIDLE)) {
    bprintf ("You will no longer see real idle times.\n");
    pclrflg (mynum, PFL_SEEIDLE);
    return;
  } else {
    bprintf ("You will now see real idle times.\n");
    psetflg (mynum, PFL_SEEIDLE);
    return;
  }
}

/* Socket handler */
void
socketcom ()
{
  static char *SockTable[] =
  {
    "view", "kill", "write", TABLE_END
  };

  int i, b, x, d;
  int old_mynum = mynum;

  Boolean noton = False;
  char idlebuff[64], loginbuff[64];
  char txt[MAX_COM_LEN];

  if (!ptstflg (mynum, PFL_SOCKET)) {
    bprintf ("Pardon?\n");
    return;
  }
  if (brkword () == -1) {
    bprintf ("&+CName                FD      Idle    On For  Host\n");
    bprintf ("&+B-------------------------------------------------------------------------------\n");
    for (i = 0; i < max_players; ++i) {
      if (see_player (mynum, i) && is_conn (i)) {
	strcpy (idlebuff, sec_to_hhmmss (global_clock - prlast_cmd (i)));
	strcpy (loginbuff, sec_to_hhmmss (global_clock - plogged_on (i)));
	bprintf ("&+w%-19s %2d  %8.8s  %8.8s  %-34.34s\n",
		 pname (i), players[i].fil_des,
		 idlebuff, loginbuff, !ptstflg (mynum, PFL_SEEUSER) ?
		 players[i].hostname : players[i].usrname);
      }
    }
    bprintf ("&+B-------------------------------------------------------------------------------\n");
    return;
  }
  if ((d = atoi (wordbuf)) == 0) {
    if ((b = find_player_by_name (wordbuf)) == -1) {
      bprintf ("Person not on-line.\n");
      return;
    }
    d = players[b].fil_des;
  } else {
    if ((b = find_pl_index (d)) == -1) {
      bprintf ("That descriptor is not in use.\n");
      return;
    }
    if (!is_in_game (b)) {
      if (!is_conn (b)) {
	bprintf ("That descriptor is not in use.\n");
	return;
      }
      noton = True;
    }
  }
  if (b >= max_players) {
    bprintf ("A mobile doesn't have a socket.\n");
    return;
  }
  if (is_in_game (b) && (pvis (b) > plev (mynum))) {
    bprintf ("That descriptor is not in use.\n");
    return;
  }
  if (brkword () == -1) {
    if (noton)
      bprintf ("What do you want to do with descriptor %d?\n", d);
    else
      bprintf ("What do you want to do with %s's descriptor?\n", pname (b));
    return;
  }
  if ((x = tlookup (wordbuf, SockTable)) < 0) {
    bprintf ("What are you trying to do?\n");
    return;
  }
  switch (x) {
  case 0:
    strcpy (idlebuff, sec_to_hhmmss (global_clock - prlast_cmd (b)));
    strcpy (loginbuff, sec_to_hhmmss (global_clock - plogged_on (b)));
    bprintf ("&+CDescriptor &+B: &+W%-2d\n", d);
    bprintf ("&+B----------------------------------\n");
    bprintf ("&+wUser Name  &+B: &+w%s\n", pname (b));
    bprintf ("&+wHostname   &+B: &+w%s\n",
	     !ptstflg (mynum, PFL_SEEUSER) ? players[b].hostname :
	     players[b].usrname);
    bprintf ("&+wIdle Time  &+B: &+w%s\n", idlebuff);
    bprintf ("&+wLogged On  &+B: &+w%s\n", loginbuff);
    bprintf ("&+B----------------------------------\n");
    break;
  case 1:
    bprintf ("Killing Descriptor %d (User: %s)\n", d, pname (b));
    mudlog ("SOCKET: %s killed descriptor %d (User: %s)", pname (mynum),
	    d, pname (b));

    if (noton)
      send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_SOCKET), LVL_WIZARD, LVL_MAX, b,
		mynum, "&+W[&+CSocket: &+w\001p%s\003 has killed descriptor "
		"%d (Logging In)&+W]\n", pname (mynum), d);
    else
      send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_SOCKET), LVL_WIZARD, LVL_MAX, b,
		mynum, "&+W[&+CSocket: &+w\001p%s\003 has killed descriptor "
		"%d (User: \001p%s\003)&+W]\n", pname (mynum), d, pname (b));

    setup_globals (b);
    bflush ();

    if (noton)
      quit_player ();
    else
      crapup (NULL, NO_SAVE);

    setup_globals (old_mynum);
    break;
  case 2:
    if (noton)
      bprintf ("Writing text to descriptor %d.\n", d);
    else
      bprintf ("Writing text to %s's descriptor.\n", pname (b));

    getreinput (txt);
    setup_globals (b);
    bprintf ("%s\n", txt);
    setup_globals (old_mynum);
    break;
  }
}

void
toggleseesocket (void)
{
  if (!ptstflg (mynum, PFL_SOCKET)) {
    bprintf ("Pardon?\n");
    return;
  }
  if (!ptstflg (mynum, PFL_SEESOCKET)) {
    psetflg (mynum, PFL_SEESOCKET);
    bprintf ("You can now see socket messages.\n");
  } else {
    pclrflg (mynum, PFL_SEESOCKET);
    bprintf ("You will no longer see socket messages.\n");
  }
}

void
togglecoding (void)
{
  if (plev (mynum) < LVL_GOD) {
    erreval ();
    return;
  }
  if (!ststflg (mynum, SFL_CODING)) {
    ssetflg (mynum, SFL_CODING);
    bprintf ("You are marked as coding.\n");
    send_msg (DEST_ALL, 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "&+G[&+W%s &*is going away to code&+G]\n", pname (mynum));
  } else {
    sclrflg (mynum, SFL_CODING);
    bprintf ("You are no longer marked as coding.\n");
    send_msg (DEST_ALL, 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	      "&+G[&+W%s &*is back from coding&+G]\n", pname (mynum));
  }
}

void
wloadcom ()
{
  char work[100];
  char ps1[100];
  char ps2[300];

  FILE *fp;
  char command[50];

  if (plev (mynum) < LVL_GOD) {
    erreval ();
    return;
  }
  sprintf (command, "ps -ux | grep 'aberd' | grep '%d'", getpid ());
  if ((fp = popen ("uptime; ps -ux", "r")) == NULL) {
    bprintf ("Error getting system work load statistics.\n");
    return;
  }
  fgets (work, sizeof (work), fp);
  fgets (ps1, sizeof (ps1), fp);
  pclose (fp);

  if ((fp = popen (command, "r")) == NULL) {
    bprintf ("Error getting system work load statistics.\n");
    return;
  }
  fgets (ps2, sizeof (ps2), fp);
  pclose (fp);

  bprintf ("System statistics on server %s:\n", my_hostname);
  bprintf ("%s\n", work);

  bprintf ("iDiRT Daemon Statistics\n");
  bprintf ("-----------------------\n");
  bprintf ("%s", ps1);
  bprintf ("%-79.79s\n", ps2);
}

void
silentcom (void)
{
  if (plev (mynum) < LVL_GOD) {
    erreval ();
    return;
  }
  if (ststflg (mynum, SFL_SILENT)) {
    bprintf ("Disabling Silent Entry\n");
    sclrflg (mynum, SFL_SILENT);
  } else {
    bprintf ("Enabling Silent Entry\n");
    ssetflg (mynum, SFL_SILENT);
  }
}

void
ploccom (void)
{
  int x, loc;
  int me = real_mynum;
  char buff[100];

  if (plev (mynum) < LVL_GOD) {
    bprintf ("Pardon?\n");
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Who do you want to relocate?\n");
    return;
  }
  if (EMPTY (item2)) {
    bprintf ("Where do you want to relocate them to?\n");
    return;
  }
  if ((x = pl1) == -1) {
    bprintf ("That person can't be found.\n");
    return;
  }
  if ((loc = findroomnum (item2)) == 0) {
    bprintf ("Location does not exist!\n");
    return;
  }
  bprintf ("You relocate %s to %s (%s).\n",
	   pname (x), sdesc (loc), xshowname (buff, loc));

  send_msg (DEST_ALL, 0, LVL_GOD, LVL_MAX, mynum, x,
       "&+B[&+W\001p%s\003 &*has located &+W\001p%s\003 &*to %s (%s)&+B]\n",
	    pname (mynum), pname (x), sdesc (loc), xshowname (buff, loc));

  if (x < max_players) {
    setup_globals (x);
    setploc (mynum, loc);
    setup_globals (me);
  } else
    setploc (x, loc);
}

void
writelog (void)
{
  char data[MAX_COM_LEN];

  if (!ptstflg (mynum, PFL_CANLOG)) {
    erreval ();
    return;
  }
  getreinput (data);

  if (EMPTY (data)) {
    bprintf ("What do you want to log?\n");
    return;
  }
  mudlog ("LOG: %s", data);
}

void
bancom (char filename[100], int type)
{
  static char *log[] =
  {
    "BANHOST", "BANUSER", "BANLOGIN", "BANCHECK"
  };
  static char *info[] =
  {
    "the host", "the user", "logins from", "login checks from"
  };

  int val;
  char text[80], orig[80];

  if (!ptstflg (mynum, PFL_BAN)) {
    erreval ();
    return;
  }
  getreinput (text);

  if (EMPTY (text)) {
    bprintf ("Ban/Unban who or what?\n");
    return;
  }
  sprintf (orig, "%s", text);

  if (type == BANUSER)
    orig[0] = toupper (orig[0]);

  val = addordel (filename, text);

  if (val == 0) {
    bprintf ("Error has occured in ban.\n");
    mudlog ("%s: Error has occured.", log[type]);
    return;
  }
  if (val == ADDED) {
    bprintf ("You have banned %s %s.\n", info[type], orig);
    mudlog ("%s: %s banned %s.", log[type], pname (mynum), orig);
    return;
  }
  if (val == DELETED) {
    bprintf ("You have unbanned %s %s.\n", info[type], orig);
    mudlog ("%s: %s unbanned %s.", log[type], pname (mynum), orig);
    return;
  }
}

void
probationcom (void)
{
  int b;

  if (!ptstflg (mynum, PFL_PROBATION)) {
    erreval ();
    return;
  }
  if (EMPTY (item1)) {
    bprintf ("Put whom on probation?\n");
    return;
  }
  if ((b = pl1) == -1) {
    bprintf ("Who is that?\n");
    return;
  }
  if (b >= max_players) {
    bprintf ("I'm afraid that you can't place a mobile on probation.\n");
    return;
  }
  if (!do_okay (mynum, b, PFL_NOPROBATION)) {
    bprintf ("You cannot place that player under probation.\n");
    return;
  }
  if (!ststflg (b, SFL_ONPROBATION)) {
    mudlog ("PROBATION: %s placed %s ON probation", pname (mynum), pname (b));
    bprintf ("Placing %s on probation.\n", pname (b));
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, b,
	      "&+B[&+WProbation (On): &+C\001p%s\003 &*by &+C\001p%s\003"
	      "&+B]\n", pname (b), pname (mynum));
    ssetflg (b, SFL_ONPROBATION);

    if (plev (b) < LVL_WIZARD)
      return;

    pclrflg (b, PFL_EXOR);
    pclrflg (b, PFL_ZAP);
    pclrflg (b, PFL_MFLAGS);
    pclrflg (b, PFL_OBJECT);
    pclrflg (b, PFL_HEAL);
    pclrflg (b, PFL_ALIAS);
    pclrflg (b, PFL_ROOM);
    pclrflg (b, PFL_CLONE);
    pclrflg (b, PFL_PUNT);
    pclrflg (b, PFL_TIMEOUT);
    pclrflg (b, PFL_LD_STORE);
    pclrflg (b, PFL_BURN);
    pclrflg (b, PFL_SIC);

    if (wlevel (b) < LEV_PROPHET)
      return;

    pclrflg (b, PFL_RAW);
    pclrflg (b, PFL_CH_SCORE);
    pclrflg (b, PFL_CH_LEVEL);
  } else {
    mudlog ("PROBATION: %s placed %s OFF probation", pname (mynum), pname (b));
    bprintf ("Taking %s off probation.\n", pname (b));
    send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, b,
	      "&+B[&+WProbation (Off): &+C\001p%s\003 &*by &+C\001p%s\003"
	      "&+B]\n", pname (b), pname (mynum));
    sclrflg (b, SFL_ONPROBATION);

    if (plev (b) < LVL_WIZARD)
      return;

    psetflg (b, PFL_EXOR);
    psetflg (b, PFL_ZAP);
    psetflg (b, PFL_MFLAGS);
    psetflg (b, PFL_OBJECT);
    psetflg (b, PFL_HEAL);
    psetflg (b, PFL_ALIAS);
    psetflg (b, PFL_ROOM);
    psetflg (b, PFL_CLONE);
    psetflg (b, PFL_PUNT);
    psetflg (b, PFL_TIMEOUT);
    psetflg (b, PFL_LD_STORE);
    psetflg (b, PFL_BURN);
    psetflg (b, PFL_SIC);

    if (wlevel (b) < LEV_PROPHET)
      return;

    psetflg (b, PFL_RAW);
    psetflg (b, PFL_CH_SCORE);
    psetflg (b, PFL_CH_LEVEL);
  }
}

void
forgetlist (void)
{
  int b, i;

  if (plev (mynum) < LVL_GOD) {
    erreval ();
    return;
  }
  if (brkword () == -1) {
    bprintf ("Who?\n");
    return;
  }
  if ((b = fpbns (wordbuf)) != -1 && seeplayer (b)) {
    if (b >= max_players) {
      bprintf ("A mobile does not have a forget list.");
      return;
    }
    bprintf ("%s is currently forgetting:\n", pname (b));
    for (i = 0; i < 10; ++i) {
      if (players[b].forget[i] != -1) {
	if (!is_in_game (players[b].forget[i]))
	  players[b].forget[i] = -1;
	else
	  bprintf ("%s\n", pname (players[b].forget[i]));
      }
    }
    return;
  }
  bprintf ("That player is not online.\n");
}

void
reboot_actions (void)
{
  FILE *fp;

  if (plev (mynum) < LVL_GOD) {
    erreval ();
    return;
  }
  bprintf ("Rebooting Actions File...\n");
  if ((fp = fopen ("actions", "r")) == NULL) {
    bprintf ("Error: Cannot read actions file.\n");
    mudlog ("ERROR: File actions cannot be read (reboot_actions)");
    return;
  }
  boot_extern (fp, "actions");
  bprintf ("Actions File Rebooted.\n");
  mudlog ("SYSTEM: Actions reloaded by %s", pname (mynum));

  send_msg (DEST_ALL, MODE_QUIET, LVL_GOD, LVL_MAX, mynum, NOBODY,
	  "&+B[&+CReload Actions &*by &+W\001p%s\003&+B]\n", pname (mynum));

}

void
levforcecom (void)
{
  Boolean LessThan = False;
  int i, lev, me = real_mynum;
  char com[MAX_COM_LEN];

  if (plev (mynum) < LVL_ARCHWIZARD) {
    erreval ();
    return;
  }
  if (brkword () == -1) {
    bprintf ("LevForce at what level?\n");
    return;
  }
  if (strchr (wordbuf, '-') != NULL) {
    LessThan = True;
    wordbuf[strlen (wordbuf) - 1] = '\0';
  }
  if ((lev = atoi (wordbuf)) <= 0) {
    bprintf ("Invalid Level: %s.\n", wordbuf);
    return;
  }
  if (lev > plev (mynum)) {
    bprintf ("Sorry, you can't levforce that high.\n");
    return;
  }
  getreinput (com);

  if (EMPTY (com)) {
    bprintf ("Levforce %d%c what?\n", lev, LessThan ? '-' : '+');
    return;
  }
  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && see_player (me, i) && do_okay (me, i, PFL_NOFORCE)
	&& i != me) {
      if ((LessThan && plev (i) <= lev) || (!LessThan && plev (i) >= lev)) {
	setup_globals (i);
	bprintf ("\001p%s\003 has forced you to %s.\n", pname (me), com);
	cur_player->isforce = True;
	gamecom (com, True);
	cur_player->isforce = False;
      }
    }
  }

  setup_globals (me);

  send_msg (DEST_ALL, MODE_QUIET, LVL_WIZARD, LVL_MAX, mynum, NOBODY,
	    "&+B[&+CLevForce (%d%c) &*by &+W\001p%s\003 &*(%s)&+B]\n",
	    lev, LessThan ? '-' : '+', pname (mynum), com);
}

void 
noruncom (void)
{
  int i, me = real_mynum;

  if (plev (mynum) < LVL_GOD) {
    erreval ();
    return;
  }
  if (norun) {
    norun = False;
    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "&+B[&+WRunning Is Allowed Again&+B]\n");
    mudlog ("NORUN: Turned Off by %s", pname (mynum));
  } else {
    norun = True;
    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "&+B[&+WRunning Is Being Disabled&+B]\n");
    mudlog ("NORUN: Turned On by %s", pname (mynum));
  }
  for (i = 0; i < max_players; i++) {
    if (is_in_game (i) && plev (i) < LVL_WIZARD) {
      setploc (i, norun ? LOC_LIMBO_NORUN : randperc () < 50 ?
	       LOC_START_TEMPLE : LOC_START_CHURCH);
      setup_globals (i);
      trapch (ploc (mynum));
    }
  }
  setup_globals (me);
}
