

/************************************************************************
 * iDiRT Exit Handler 1.02.00						*
 * 1995 by Illusion							*
 ************************************************************************/

/************************************************************************
 * The following functions handle all exit() calls from the MUD and 	*
 * handle them itself making sure that the MUD shuts down nicely and 	*
 * with as little problems as possible.					*
 ************************************************************************/

#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <a.out.h>
#include "kernel.h"
#include "mobile.h"
#include "sendsys.h"
#include "timing.h"
#include "time.h"
#include "exit.h"
#include "mudmacros.h"
#include "pflags.h"
#include "log.h"
#include "reboot.h"
#include "commands.h"
#include "bprintf.h"
#include "mud.h"
#include "uaf.h"
#include "parse.h"

typedef struct __DB__Info {
  unsigned long symsize;
  unsigned long symcount;
  unsigned long strsize;
  unsigned long symoffset;
  unsigned long stroffset;
}

_DB_Info;

_DB_Info DB_Info;
char *DB_Binary;
char *Syms, *Names;

/* __exit(): Handles the shutdown of the MUD. */
void
__exit (int status)
{
  mudlog ("SYSTEM: __exit(%d) called", status);

  send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	    "&+Y[&+RInternal error has occured&+Y]\n");

  autosave ();
  debug ();

#ifndef NOCATCH
  run_reboot (True, True);
#endif
  _exit (1);
}

/* sig_exit(): Handles the shutdown of the MUD due to signal error. */
void
sig_exit (char *sig, int signal)
{

  if (signal == SIGUSR1) {
    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "\001f" SIGNAL1 "\003");
  } else if (signal == SIGUSR2) {
    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "\001f" SIGNAL2 "\003");
  } else {
    send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+Y[&+RSignal Error: &+W%s&+Y]\n", sig);

    send_msg (DEST_ALL, 0, LVL_GUEST, (LVL_WIZARD - 1), NOBODY, NOBODY,
	      "&+Y[&+RInternal error has occured&+Y]\n");
  }

  autosave ();
  debug ();
#ifndef NOCATCH
  run_reboot (True, True);
#endif
  _exit (2);
}

/* autosave(): Saves all players. */
void
autosave (void)
{
  int plx;
  PERSONA d;

  for (plx = 0; plx < max_players; ++plx) {
    if (is_in_game (plx)) {
      if (!fclose (players[plx].Mailer.mailbox))
	sendf (plx, "&+R[&+WClosing Mailbox&+R]\n");
      if (!fclose (players[plx].Mailer.output)) {
	sendf (plx, "&+R[&+WAborting Message&+R]\n");
	unlink (players[plx].Mailer.outputname);
      }
      if (players[plx].aliased || players[plx].polymorphed >= 0) {
	unalias (plx);
	unpolymorph (plx);
	setup_globals (plx);
	sendf (plx, "&+R[&+WUnaliasing You&+R]\n");
      }
      sendf (plx, "&+R[&+WSaving Character&+R]\n");
      player2pers (&d, &global_clock, plx);
      bflush ();
      putuaf (&d);
    }
  }
}

/* debug(): Write last commands entered to system logs */
void
debug (void)
{
  int plx;
  int i;
  char *t;
  char nt[100];

  for (plx = 0; plx < max_players; ++plx)
    if (is_in_game (plx)) {
      t = ctime (&prlast_cmd (plx));

      t[19] = '\0';
      for (i = 0; i < 8; ++i)
	nt[i] = t[i + 11];

      nt[8] = '\0';

      mudlog ("DEBUG: Last Command (%s) %s: %s",
	      nt, pname (plx), players[plx].prev_com);
    }
}

void
signalcom (void)
{
  static char *SigTable[] =
  {"list", "sigsegv", "sigterm", "sigbus",
   "sigint", "sigusr1", "sigusr2", TABLE_END};

  int sig = 0, x = 0;

  if (!ptstflg (mynum, PFL_SIGNAL)) {
    erreval ();
    return;
  }
  if (brkword () == -1) {
    bprintf ("Send what signal?\n");
    return;
  }
  if ((x = tlookup (wordbuf, SigTable)) < 0) {
    bprintf ("Invalid signal.\n");
  }
  switch (x) {
  case 0:
    sig = -1;
    break;
  case 1:
    sig = SIGSEGV;
    break;
  case 2:
    sig = SIGTERM;
    break;
  case 3:
    sig = SIGBUS;
    break;
  case 4:
    sig = SIGINT;
    break;
  case 5:
    sig = SIGUSR1;
    break;
  case 6:
    sig = SIGUSR2;
    break;
  }

  if (sig == -1) {
    bprintf ("Signals: SIGSEGV, SIGTERM, SIGBUS, SIGINT, SIGUSR1, SIGUSR2.\n");
    return;
  }
  send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, mynum, NOBODY,
	    "&+B[&+CSignal &*has been called: &+W%s&+B]\n",
	    SigTable[x]);

  mudlog ("SIGNAL: %s has called signal %s", pname (mynum), SigTable[x]);
  kill (getpid (), sig);
}
