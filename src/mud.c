
#include "kernel.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "sendsys.h"
#include "pflags.h"
#include "sflags.h"
#include "mud.h"
#include "uaf.h"
#include "mobile.h"
#include "timing.h"
#include "locations.h"
#include "bprintf.h"
#include "fight.h"
#include "zones.h"
#include "rooms.h"
#include "mailer.h"
#include "log.h"
#include "parse.h"
#include "acct.h"

static Boolean login_ok (char *name);
static void talker (void);

char *pwait = "&+W(&+wPress &+C[Return] &+wto continue&+W)&*";
char *qwait = "&+W(&+wPress &+C[Return] &+wto continue, &+C'q' &+wto quit&+W)&*";
char *vismsg = "&+wEnter Vis Level (&+W0-%d&+w), '&+Wi&+w' for full invisibility,\n"
"or Press &+W[&+CEnter&+W]&+w To Keep Vis Level (Current:&+W %d&+w):";

static int vislev[] =
{0, LVL_PROPHET, LVL_PROPHET, LVL_PROPHET,
 LVL_PROPHET, LVL_ARCHWIZARD, LVL_ADVISOR,
 LVL_AVATAR, LVL_GOD, LVL_MASTER, LVL_MAX};

void
bannedmsg (void)
{
  if (cur_player->host_ban)
    send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_SEESOCKET), LVL_WIZARD,
	      LVL_MAX, NOBODY, NOBODY, "&+W[&+CSocket (%d): &+wBanned Host: "
	      "&+C%s&+W]\n", cur_player->fil_des, cur_player->hostname);

  if (cur_player->user_ban)
    send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_SEESOCKET), LVL_WIZARD, LVL_MAX,
	      NOBODY, NOBODY, "&+W[&+CSocket (%d): &+wUser/Who Banned Host: "
	      "&+C%s&+W]\n", cur_player->fil_des, cur_player->hostname);
}

static char *
test1 (int plr, int vis, char *msg)
{
  if (ptstflg (plr, PFL_SEESOCKET) && ptstflg (plr, PFL_SEEUSER) && vis < plev (plr))
    return msg;
  else
    return NULL;
}

static char *
test2 (int plr, int vis, char *msg)
{
  if (ptstflg (plr, PFL_SEESOCKET) && !ptstflg (plr, PFL_SEEUSER) && vis < plev (plr))
    return msg;
  else
    return NULL;
}

void
socketmsg (Boolean user, Boolean chkvis, char *format,...)
{
  va_list pvar;
  char msg[200];

  va_start (pvar, format);
  vsprintf (msg, format, pvar);
  va_end (pvar);

  if (is_host_silent (cur_player->hostname))
    return;

  if (chkvis) {
    if (user)
      send_g_msg (DEST_ALL, test1, pvis (mynum), msg);
    else
      send_g_msg (DEST_ALL, test2, pvis (mynum), msg);
  } else {
    if (user)
      send_g_msg (DEST_ALL, test1, -1, msg);
    else
      send_g_msg (DEST_ALL, test2, -1, msg);
  }
}

void
push_input_handler (void (*h) (char *str))
{
  INP_HANDLER *i;

  i = NEW (INP_HANDLER, 1);
  i->next = cur_player->inp_handler;
  i->inp_handler = h;
  cur_player->inp_handler = i;
}

void
pop_input_handler (void)
{
  INP_HANDLER *i = cur_player->inp_handler;
  INP_HANDLER *j;

  j = i->next;
  if (j != NULL) {
    cur_player->inp_handler = j;
    free (i);
  }
}

void
replace_input_handler (void (*h) (char *str))
{
  cur_player->inp_handler->inp_handler = h;
}

int
find_free_player_slot (void)
{
  PLAYER_REC *p;
  int i, v = max_players;
  int k;

  for (i = 0; i < v && players[i].inp_handler != NULL; i++) ;
  if (i >= v)
    i = -1;
  else {
    /* Initialize entry */
    p = players + i;
    p->fil_des = -1;
    p->stream = NULL;
    p->no_echo = False;
    p->isawiz = False;
    p->ismonitored = False;
    p->iamon = False;
    p->in_pbfr = False;
    p->aliased = False;
    p->me_ivct = 0;
    p->polymorphed = -1;
    p->i_follow = -1;
    p->snooptarget = -1;
    p->pretend = -1;
    p->snooped = 0;
    p->asmortal = 0;
    p->last_cmd = p->logged_on = global_clock;
    p->rlast_cmd = p->logged_on = global_clock;
    strcpy (p->prev_com, "quit");
    p->quit_next = -2;
    p->wd_it = "pit";
    p->wd_them = p->wd_him;
    *p->wd_him = *p->wd_her = '\0';
    p->inmailer = False;
    p->inpager = False;
    p->logged = False;
    p->isforce = False;
    p->pconverse = -1;
    p->pager.len = 0;
    p->writer = NULL;
    p->overwrite = 0;
    p->duration = NEW (SPELL_DURATION, 1);
    p->duration = NULL;
    p->Trace.trace = -1;
    p->replyplr = -1;
    sprintf (p->awaymsg, "No particular reason.");

/* Clear the forget list.
 */
    for (k = 0; k < 10; ++k)
      p->forget[k] = -1;

    v = numchars;
    for (k = max_players; k < v; k++) {
      if (alive (k) == -1 && pscore (k) == i) {
	/* Previous occupant of this slot has killed this mobile..not me */
	setpscore (k, -1);	/* forget who it was, it was someone else */
      }
    }
  }

  return i;
}

int
find_pl_index (int fd)
{
  int i, v = max_players;

  for (i = 0; i < v && players[i].fil_des != fd; i++) ;
  if (i >= v)
    i = -1;
  return i;
}

void
xsetup_globals (int plx)
{
  int x;

  mynum = real_mynum = x = plx;
  if (x < 0 || x >= max_players) {
    cur_player = NULL;
    cur_ublock = NULL;
    mynum = real_mynum = -1;
  } else {
    cur_player = players + plx;
    if ((x = cur_player->pretend) >= 0)
      mynum = x;
    cur_ublock = ublock + x;
  }
}

void
setup_globals (int plx)
{
  bflush ();
  xsetup_globals (plx);
}

Boolean
is_host_banned (char *hostname)
{
  return infile (BAN_HOSTS, hostname);
}

Boolean
is_login_banned (char *hostname)
{
  return infile (BAN_LOGIN, hostname);
}

Boolean
is_host_user_banned (char *hostname)
{
  return infile (BAN_USRHOSTS, hostname);
}

Boolean
is_host_silent (char *hostname)
{
  return infile (SILENT_LOGIN, hostname);
}

Boolean
is_player_banned (char *name)
{
  return infile (BAN_CHARS, name);
}

Boolean
is_illegal_name (char *name)
{
  return infile (ILLEGAL_FILE, name);
}

Boolean
privileged_user (char *name)
{
  return infile (PRIVED_FILE, name);
}

Boolean
is_monitored (char *name)
{
  return infile (MONITOR_FILE, name);
}

void
new_player (char usrname[20])
{
  if (usrname == NULL) {
    strcpy (cur_player->usrname, cur_player->hostname);
  } else {
    sprintf (cur_player->usrname, "%s@%s", usrname, cur_player->hostname);
  }

  setpname (mynum, "[Logging In]");

  cur_player->iamon = False;
  cur_player->is_conn = True;
  cur_player->newplr = False;
  setplev (mynum, 1);

  bprintf ("\n\001f" WELCOME "\003");
  bflush ();

  bprintf ("Game Time Elapsed : ");
  eltime ();
  bprintf ("MUD Time Elapsed  : %s\n", sec_to_str (global_clock - last_startup));

  bannedmsg ();
  socketmsg (False, False, "&+W[&+CSocket (%d): &+wConnection from &+C%s&+W]\n",
	     cur_player->fil_des, cur_player->hostname);
  socketmsg (True, False, "&+W[&+CSocket (%d): &+wConnection from &+C%s&+W]\n",
	     cur_player->fil_des, cur_player->usrname);

  push_input_handler (get_pname1);
  get_pname1 (NULL);
}

static Boolean
login_ok (char *name)
{
  Boolean priv = False;
  Boolean ok = False;

  if (mud_open (&next_event, &global_clock))
    ok = True;
  if (OPERATOR (name))
    ok = priv = True;
  else if (privileged_user (name))
    ok = priv = True;

  cur_player->isawiz = priv;

  if (ok) {
    if (!priv && access (NOLOGIN, R_OK) == 0) {
      bprintf ("\n\n\001f%s\003\n", NOLOGIN);
      bflush ();
      quit_player ();
      return False;
    }
    if (!priv && cur_player->host_ban) {
      bprintf ("\nSorry, your host has been banned from this game.\n");
      quit_player ();
      return False;
    }
    cur_player->ismonitored = is_monitored (name);
    return True;
  } else if (next_event == TIME_NEVER) {
    bprintf ("\nMUD is closed now, please try again later.\n");
  } else {
    bprintf ("\nAberMUD opens in %s  (on %s)\n",
	     sec_to_str (round_to_min (next_event -
				       time ((time_t) NULL))),
	     my_ctime (&next_event));
    bprintf ("Please come back then.\n\n");
  }
  quit_player ();
  return False;
}

void
get_pname1 (char *name)
{
  char *s;
  Boolean a_new_player;		/* Player not in UAF */

  if (name != NULL) {
    if (*name == 0) {
      bprintf ("Ok. bye then.\n");
      socketmsg (False, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
		 cur_player->fil_des, cur_player->hostname);
      socketmsg (True, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
		 cur_player->fil_des, cur_player->usrname);
      quit_player ();
      return;
    } else {
      for (s = name; *s && isalpha (*s); ++s) ;
      if (*s) {
	bprintf ("Sorry, the name may only contain letters.\n");
      } else if (s - name > PNAME_LEN) {
	bprintf ("Pick a name with %d characters or less.\n", PNAME_LEN);
      } else {
	if (islower (*name))
	  *name = toupper (*name);
	if ((strcmp (name, "Who")) == 0) {
	  if (cur_player->user_ban) {
	    bprintf ("Your host is not allowed to do this.\n");
	  } else {
	    whocom (False);
	    setpvis (mynum, 1);
	    socketmsg (False, False, "&+W[&+CSocket (%d): &+wChecking &+WWHO &+wat login&+W]\n",
		       cur_player->fil_des);
	    socketmsg (True, False, "&+W[&+CSocket (%d): &+wChecking &+WWHO &+wat login&+W]\n",
		       cur_player->fil_des);
	  }
	} else if ((strcmp (name, "Users")) == 0) {
	  if (cur_player->user_ban) {
	    bprintf ("Your host is not allowed to do this.\n");
	  } else {
	    usercom ();
	    setpvis (mynum, 1);
	    socketmsg (False, False, "&+W[&+CSocket (%d): &+wChecking &+WUSERS &+wat login&+W]\n",
		       cur_player->fil_des);
	    socketmsg (True, False, "&+W[&+CSocket (%d): &+wChecking &+WUSERS &+wat login&+W]\n",
		       cur_player->fil_des);
	  }
	} else if (is_illegal_name (name)) {
	  bprintf ("Sorry, I can\'t call you \"%s\".\n", name);
	} else if (is_player_banned (name)) {
	  bprintf ("Sorry, you have been banned from this game.\n");
	} else {
	  (void) setpname (mynum, name);
	  if ((a_new_player = !getuafinfo (name))) {
	    get_pname2 (NULL);
	  } else if (login_ok (name)) {
	    cur_player->no_logins = 0;
	    get_passwd1 (NULL);	/* Ask user for passwd */
	  }
	  return;
	}
      }
    }
  } else {
    replace_input_handler (get_pname1);
  }
  strcpy (cur_player->cprompt, "What name? ");
  bprintf ("\nBy what name shall I call you? ");
  bflush ();
}

void
get_pname2 (char *reply)
{
  if (reply == NULL) {
    sprintf (cur_player->cprompt, "Name right, %s? ", pname (mynum));
    bprintf ("\nDid I get the name right, %s? ", pname (mynum));
    replace_input_handler (get_pname2);
    return;
  } else if (*reply == 'y' || *reply == 'Y') {
    if (login_ok (pname (mynum))) {
      bprintf ("Creating character...\n");
      get_new_pass1 (NULL);
      return;
    }
  }
  get_pname1 (NULL);
}

void
get_new_pass1 (char *pass)
{
  if (pass == NULL) {
    /* IAC WILL ECHO */
    strcpy (cur_player->cprompt, "Password: ");
    bprintf ("\377\373\001\001\nPassword: ");
    cur_player->no_echo = True;
    replace_input_handler (get_new_pass1);
  } else if (*pass == 0) {
    bprintf ("Ok, bye then.\n");
    quit_player ();
  } else {
    my_crypt (cur_player->passwd, pass, sizeof (cur_player->passwd));
    get_new_pass2 (NULL);
  }
}

void
get_new_pass2 (char *pass)
{
  char b[sizeof (cur_player->passwd)];

  if (pass == NULL) {
    strcpy (cur_player->cprompt, "Confirm: ");
    bprintf ("\nPlease retype the password for confirmation: ");
    replace_input_handler (get_new_pass2);
  } else if (*pass == 0) {
    bprintf ("Ok, bye then.\n");
    socketmsg (False, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
	       cur_player->fil_des, cur_player->hostname);
    socketmsg (True, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
	       cur_player->fil_des, cur_player->usrname);
    quit_player ();
  } else {
    my_crypt (b, pass, sizeof (b));
    if (strcmp (b, cur_player->passwd) != 0) {
      bprintf ("\nPlease give same password both times.");
      get_new_pass1 (NULL);
    } else {
      bprintf ("\377\374\001\001");

      cur_player->no_echo = False;

      socketmsg (False, False, "&+W[&+CSocket (%d): &+w%s logging in "
	 "&+B(&+WNew Player&+B)&+W]\n", cur_player->fil_des, pname (mynum));
      socketmsg (True, False, "&+W[&+CSocket (%d): &+w%s logging in "
	 "&+B(&+WNew Player&+B)&+W]\n", cur_player->fil_des, pname (mynum));
      cur_player->newplr = True;
      get_gender (NULL);
    }
  }
}

void
get_passwd1 (char *pass)
{
  char b[sizeof (cur_player->passwd)];
  int plx;

  if (pass == NULL) {
    strcpy (cur_player->cprompt, "Password: ");
    bprintf ("\377\373\001\001\nPassword: ");
    cur_player->no_echo = True;
    cur_player->no_logins = 0;
    replace_input_handler (get_passwd1);
  } else if (*pass == 0) {
    bprintf ("\377\374\001\001Ok, bye then.\n");

    cur_player->no_echo = False;

    socketmsg (False, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
	       cur_player->fil_des, cur_player->hostname);
    socketmsg (True, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
	       cur_player->fil_des, cur_player->usrname);

    quit_player ();
  } else {
    my_crypt (b, pass, sizeof (b));
    if (strcmp (cur_player->passwd, b) == 0) {
      bprintf ("\377\374\001\001");
      cur_player->no_echo = False;
      if ((plx = check_if_player_exists (pname (mynum))) >= 0) {
	bprintf ("There is already someone named %s in the game.\n",
		 pname (mynum));
	if (plx >= max_players) {
	  bprintf ("You can't use %s as a name.\n", pname (mynum));
	  socketmsg (False, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
		     cur_player->fil_des, cur_player->hostname);
	  socketmsg (True, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
		     cur_player->fil_des, cur_player->usrname);
	  quit_player ();
	} else {
	  kick_out_yn (NULL);
	}
	return;
      }
      socketmsg (False, True, "&+W[&+CSocket (%d): &+C%s &+wlogging in&+W]\n",
		 cur_player->fil_des, pname (mynum));
      socketmsg (True, True, "&+W[&+CSocket (%d): &+C%s &+wlogging in&+W]\n",
		 cur_player->fil_des, pname (mynum));

      do_issue (NULL);
    } else {
      bprintf ("Incorrect password.\n");

      socketmsg (False, True, "&+W[&+CSocket (%d): &+wWrong password from &+C%s "
		 "&+B(&+W%s&+B)&+W]\n", cur_player->fil_des, pname (mynum),
		 cur_player->hostname);
      socketmsg (True, True, "&+W[&+CSocket (%d): &+wWrong password from &+C%s "
		 "&+B(&+W%s&+B)&+W]\n", cur_player->fil_des, pname (mynum),
		 cur_player->usrname);

      if (++cur_player->no_logins >= 3) {
	cur_player->no_echo = False;
	bprintf ("Bad password!\377\374\001\001\n");
	mudlog ("SYSTEM: Multiple login-failures: %s from %s",
		pname (mynum), cur_player->hostname);

	socketmsg (False, True, "&+W[&+RSystem: &+wBad password by &+W%s &+w(%s)&+W]\n",
		   pname (mynum), cur_player->hostname);
	socketmsg (True, True, "&+W[&+RSystem: &+wBad password by &+W%s &+w(%s)&+W]\n",
		   pname (mynum), cur_player->usrname);

	socketmsg (False, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
		   cur_player->fil_des, cur_player->hostname);
	socketmsg (True, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
		   cur_player->fil_des, cur_player->usrname);

	quit_player ();
      } else {
	bprintf ("Password: ");
      }
    }
  }
}

void
kick_out_yn (char *answer)
{
  int plx, fd, sin_len, ply;
  FILE *fp;
  PLAYER_REC *p, *cur;
  struct sockaddr_in sin;
  char host[MAXHOSTNAMELEN];

  if ((plx = fpbns (pname (mynum))) < 0) {
    do_issue (NULL);
    return;
  } else if (plx >= max_players) {
    bprintf ("You can't use the name of a mobile!\n");
    quit_player ();
    return;
  } else if (answer == NULL) {
    bprintf ("Want me to kick out %s? (Y/N) ", pname (mynum));
    sprintf (cur_player->cprompt, "Kick out %s? ", pname (mynum));
    replace_input_handler (kick_out_yn);
    return;
  } else {
    while (*answer == ' ' || *answer == '\t')
      ++answer;
    if (*answer == '\0') {
      bprintf ("%s", cur_player->cprompt);
      return;
    } else if (*answer != 'y' && *answer != 'Y') {
      bprintf ("Ok, bye then.\n");

      socketmsg (False, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
		 cur_player->fil_des, cur_player->hostname);
      socketmsg (True, False, "&+W[&+CSocket (%d): &+wConnection closed from &+C%s&+W]\n",
		 cur_player->fil_des, cur_player->usrname);

      quit_player ();
      return;
    } else {
      p = players + plx;
      cur = cur_player;
      fd = cur->fil_des;
      fp = cur->stream;
      sin = cur->sin;
      sin_len = cur->sin_len;
      strcpy (host, cur->hostname);
      cur->fil_des = p->fil_des;
      cur->stream = p->stream;
      cur->sin = p->sin;
      cur->sin_len = p->sin_len;
      strcpy (cur->hostname, p->hostname);
      p->fil_des = fd;
      p->stream = fp;
      p->sin = sin;
      p->sin_len = sin_len;
      strcpy (p->hostname, host);
      quit_player ();
      ply = real_mynum;
      setup_globals (plx);
      do_issue (NULL);
      setup_globals (ply);
    }
  }
}

void
enter_vis (char *v)
{
  int maxlev, lev;

  if (EMPTY (v) || v[0] == '\n') {
    do_motd (NULL);
  } else {
    maxlev = vislev[wlevel (plev (mynum))];

    if (v[0] == 'i' || v[0] == 'I') {
      setpvis (mynum, maxlev);
      do_motd (NULL);
    } else {
      lev = atoi (v);
      if (lev < 0 || lev > maxlev) {
	bprintf ("Invalid Input.\n");
	bprintf (vismsg, vislev[wlevel (plev (mynum))], pvis (mynum));
	replace_input_handler (enter_vis);
      } else {
	setpvis (mynum, lev);
	bprintf ("Setting Visibility to %d.\n", lev);
	do_motd (NULL);
      }
    }
  }
}

void
do_issue (char *cont)
{
  if (cont == NULL) {
    bprintf ("\n\001f" ISSUE "\003");
    if (plev (mynum) < LVL_WIZARD) {
      strcpy (cur_player->cprompt, "Hit return: ");
      bprintf (pwait);
      replace_input_handler (do_issue);
    } else {
      strcpy (cur_player->cprompt, "Enter vis: ");
      bprintf (vismsg, vislev[wlevel (plev (mynum))], pvis (mynum));
      replace_input_handler (enter_vis);
    }
  } else {
    do_motd (NULL);
  }
}

void
do_motd (char *cont)
{
  if (cont == NULL) {
    bprintf ("\n\001f" MOTD "\003");
    strcpy (cur_player->cprompt, "Hit return: ");
    bprintf (pwait);
    replace_input_handler (do_motd);
  } else {
    talker ();
  }
}

void
talker (void)
{
  int k;
  char msg[80];
  char b[50];
  char buff1[200];
  char buff2[200];

  setpwpn (mynum, -1);
  setphelping (mynum, -1);
  setpfighting (mynum, -1);
  setpsitting (mynum, 0);

  insert_entry (mob_id (mynum), mynum, &id_table);

  if (ptstflg (mynum, PFL_CLONE)
      && ((k = get_zone_by_name (pname (mynum))) < 0 || ztemporary (k))) {

    load_zone (pname (mynum), NULL, NULL, NULL, NULL, NULL, NULL);
  }
  if (norun && plev (mynum) < LVL_WIZARD)
    setploc (mynum, LOC_LIMBO_NORUN);
  else
    setploc (mynum, exists (k = find_loc_by_id (phome (mynum))) ? k :
	     randperc () > 50 ? LOC_START_TEMPLE : LOC_START_CHURCH);

  cur_player->iamon = True;
  fetchprmpt (mynum);

  if (is_monitored (pname (mynum))) {
    open_plr_log ();
  }
  if (!ststflg (mynum, SFL_SILENT)) {
    mudlog ("ENTRY: %s [Level: %d, Score: %d]",
	    pname (mynum), plev (mynum), pscore (mynum));

    mudlog ("SOCKET: %s connected from %s", pname (mynum), cur_player->hostname);
  }
  send_msg (DEST_ALL, MODE_QUIET | MP (PFL_SHUSER) | MODE_PFLAG,
	    max (pvis (mynum), LVL_WIZARD), LVL_MAX, mynum, NOBODY,
	    "&+B[&+W%s &+w(&+C%s&+w) has entered the game in &+W%s &*(%s: &+C%d&*)&+B]\n",
       pname (mynum), cur_player->hostname, xshowname (buff2, ploc (mynum)),
	    plev (mynum) < LVL_WIZARD ? "Lev" : "Vis",
	    plev (mynum) < LVL_WIZARD ? plev (mynum) : pvis (mynum));

  send_msg (DEST_ALL, MODE_QUIET | MP (PFL_SHUSER) | MODE_NPFLAG,
	    max (pvis (mynum), LVL_WIZARD), LVL_MAX, mynum, NOBODY,
	    "&+B[&+W%s has entered the game in &+W%s &*(%s: &+C%d&*)&+B]\n",
	    pname (mynum), xshowname (buff2, ploc (mynum)),
	    plev (mynum) < LVL_WIZARD ? "Lev" : "Vis",
	    plev (mynum) < LVL_WIZARD ? plev (mynum) : pvis (mynum));

  if (cur_player->newplr)
    send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+B[&+W%s &*is a &+CNew Player&+B]\n", pname (mynum));

  if ((k = the_world->w_lock) > plev (mynum)) {
    sprintf (msg, "I'm sorry, the game is currently %slocked - please try later.\n",
	     lev2s (b, k));
    crapup (msg, NO_SAVE);
    return;
  } else if (k != 0) {
    bprintf ("The game is currently %slocked.\n", lev2s (buff1, k));
  }
  for (k = 0; k < max_players; k++) {
    if (EQ(pname(k),pname(mynum)) && (k != mynum)) {
      mudlog("CONN: Multiple entries for player %s",pname(mynum));
      p_crapup(k,"\t\tNo need to be on more than once.",CRAP_SAVE|CRAP_RETURN);
    }
  }
  if (the_world->w_peace) {
    bprintf ("Everything is peaceful.\n");
  }
  if (plev (mynum) >= LVL_WIZARD && the_world->w_mob_stop != 0) {
    bprintf ("Mobiles are STOPed.\n");
  }
  trapch (ploc (mynum));

  send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
	    "%s\n", build_setin (buff1, cur_player->setqin, pname (mynum), NULL, NULL));
  check_files ();
  get_command (NULL);
}

void
get_command (char *cmd)
{
  Boolean x;

  if (cmd != NULL) {

    gamecom (cmd, True);
    /* Check if he is using same command input handler */
    x = (cur_player->inp_handler->inp_handler != get_command);
    if (x || cur_player->quit_next >= -1)
      return;
  } else {
    x = (cur_player->inp_handler->inp_handler != get_command);
  }

  strcpy (cur_player->cprompt, build_prompt (real_mynum));
  bprintf ("\r%s", cur_player->cprompt);
  if (x)
    replace_input_handler (get_command);
}

void
quit_player (void)
{
  if (cur_player->quit_next == -2) {
    cur_player->quit_next = quit_list;
    quit_list = real_mynum;
    cur_player->is_conn = False;
  }
}

void
check_files (void)
{
  struct stat s;
  time_t last_login;
  PERSONA p;
  char mailfile[100];

  getuaf (pname (mynum), &p);
  last_login = p.p_last_on;
  sprintf (mailfile, "%s/%s", MAIL_DIR, pname (mynum));

#ifdef SHOW_LAST_LOGIN
  bprintf ("&+wLast login : &+W%s", ctime (&p.p_last_on));
  bprintf ("&+wFrom host  : &+W%s\n\n", p.p_last_host);
#endif

  if (stat (BULL_DIR "/" BULLETIN1, &s) != -1)
    if (s.st_mtime > last_login)
      bprintf ("&#New bulletin as of %19.19s.\nType 'bulletin' to read it.\n",
	       ctime (&s.st_mtime));

  if ((stat (BULL_DIR "/" BULLETIN2, &s) != -1) && plev (mynum) >= LVL_WIZARD)
    if (s.st_mtime > last_login)
      bprintf ("&#New Wizard bulletin as of %19.19s.\nType 'wbull' to "
	       "read it.\n", ctime (&s.st_mtime));

  if ((stat (BULL_DIR "/" BULLETIN3, &s) != -1) &&
      plev (mynum) >= LVL_ARCHWIZARD)
    if (s.st_mtime > last_login)
      bprintf ("&#New Arch-Wizard+ bulletin as of %19.19s.\nType 'abull' "
	       "to read it.\n", ctime (&s.st_mtime));

  if ((stat (BULL_DIR "/" BULLETIN4, &s) != -1) && plev (mynum) >= LVL_AVATAR)
    if (s.st_mtime > last_login)
      bprintf ("&#New Arch-Wizard+ bulletin as of %19.19s.\nType 'ubull' "
	       "to read it.\n", ctime (&s.st_mtime));

  if (stat (mailfile, &s) != -1)
    if (s.st_size > 10000)
      bprintf ("&#&+RYour mail file is %sexcessive, please remove some of "
	       "your old mail.\n", s.st_size < 15000 ? "" : "&+Wrather&+R ");

  check_mail (pname (mynum));
}
