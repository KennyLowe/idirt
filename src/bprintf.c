
/*  This module handles all the Input/Output of the system */

#include <unistd.h>
#include <stdlib.h>
#include <strings.h>

#include "kernel.h"
#include "sflags.h"
#include "pflags.h"
#include "lflags.h"
#include "bprintf.h"
#include "mud.h"
#include "mobile.h"
#include "exit.h"
#include "log.h"
#include "rooms.h"
#include "parse.h"

#ifdef VARGS
#include <stdarg.h>
#endif

static void kiputs (char *s, FILE * file);
static void dcprnt (char *ct, FILE * file);

/*
 * ** Output Functions
 * **
 */

#define SYSBUFSIZE 4096
#define MAX_ARGS   2

#define prnt(x, f) if ((x).nest) dcprnt((x).str, f); else kiputs((x).str, f);

static char *bufptr, *sysbuf;
static int buflen;
static int snooplev = 0;
static Boolean new_line = True;	/* True when next char is first in a line */

struct arg {
  char *str;
  int nest;
};

static void pansi (struct arg[], FILE *);
static void pcls (struct arg[], FILE *);
static void ppnblind (struct arg[], FILE *);
static void pfilter (struct arg[], FILE *);
static void pryou (struct arg[], FILE *);
static void pryour (struct arg[], FILE *);
static void ppndeaf (struct arg[], FILE *);
static void pndark (struct arg[], FILE *);
static void pndeaf (struct arg[], FILE *);
static void pfile (struct arg[], FILE *);
static void prname (struct arg[], FILE *);
static void pcansee (struct arg[], FILE *);

struct _code {
  char name;
  int args;
  void (*func) (struct arg[], FILE *);
};

static struct _code codes[] =
{
  {'A', 1, pansi},
  {'C', 0, pcls},
  {'D', 1, ppnblind},
  {'F', 1, pfilter},
  {'N', 1, pryou},
  {'P', 1, ppndeaf},
  {'c', 1, pndark},
  {'d', 1, pndeaf},
  {'f', 1, pfile},
  {'n', 1, pryour},
  {'p', 1, prname},
  {'s', 2, pcansee}};

#define NUM_CODES (sizeof codes / sizeof(struct _code))

static void
start_line (FILE * file)
{
  char snoop_prompt[40];
  int i;

  fix_color (snoop_prompt, "\r&+BO&+W> &*");
  if (new_line && snooplev > 0) {
    for (i = 0; i < snooplev; i++) {
      fprintf (file, "%s", snoop_prompt);
    }
  }
}

static void
kiputs (char *s, FILE * file)
{
  char *t, buff[32768], buff2[65536], cbuff[65536];

  if (strchr (s, '@') != NULL && strlen (s) <= 32768) {
    strcpy (buff, s);
    special_codes (cbuff, buff);
    s = cbuff;
  }
  if (strchr (s, '&') != NULL && strlen (s) <= 32768) {
    strcpy (buff, s);
    fix_color (buff2, buff);
  } else
    strcpy (buff2, s);

  s = buff2;

  start_line (file);
  while ((t = (char *) strchr (s, '\n')) != NULL) {
    *t = '\0';
    fputs (s, file);
    if (ststflg (mynum, SFL_COLOR))
      fputs ("\033[40m\033[0m", file);
    fputs ("\n\r", file);
    *t = '\n';
    new_line = True;
    start_line (file);
    s = t + 1;
  }
  if (*s != '\0') {
    fputs (s, file);
    new_line = False;
  }
}

static void
makebfr ()
{
  if ((bufptr = sysbuf = NEW (char, SYSBUFSIZE + BUFSIZ)) == NULL) {
    mudlog ("ERROR: Out of Memory");
    __exit (1);
  }
  sysbuf[0] = 0;
  buflen = 0;
}


static int
_code_cmp (const void *c, const void *code)
{
  return (*((const char *) c) - ((const struct _code *) code)->name);
}

static int
tocontinue (char **ct)
{
  register char *s = *ct;
  register int n = 1;
  register int nest = 0;
  struct _code *code;

  for (; n; s++) {
    switch (*s) {
    case '\002':
    case '\003':
      --n;
      break;
    case '\001':
      if (*++s != '\001') {
	code = (struct _code *) bsearch (s, (char *) codes, NUM_CODES,
					 sizeof (struct _code), _code_cmp);

	if (code == NULL) {
	  mudlog ("ERROR: tocontinue(): Unknown control code %3o",
		  0377 & *s);
	  __exit (2);
	}
	n += code->args;
	nest = 1;
      }
      break;
    case '\0':
      mudlog ("ERROR: tocontinue(): Buffer overrun\n");
      mudlog ("-----: txt: %s", *ct);
      __exit (2);
      break;
    }
  }

  *ct = s;
  return (nest);
}


static void
dcprnt (char *ct, FILE * file)
{
  char *str;
  char bk[100];
  struct arg args[MAX_ARGS + 1];
  struct _code *code;
  int n;

  while (*(str = ct)) {
    if ((ct = strchr (str, '\001')) == NULL) {
      kiputs (str, file);
      break;
    }
    *ct = '\0';
    kiputs (str, file);
    *ct++ = '\001';

    if (*ct == '\001') {
      kiputs ("\001", file);
      ct++;
    } else {
      code = (struct _code *) bsearch ((char *) ct, (char *) codes, NUM_CODES,
				       sizeof (struct _code), _code_cmp);

      if (code == NULL) {
	mudlog ("ERROR: dcprnt(): Unknown control code %c (%3o) \n",
		*ct, 0377 & *ct);
	mudlog ("-----: ct[0..19] = \"%s\".", mk_string (bk, ct, 20, -1));
	__exit (2);
      } else {
	args[0].str = ++ct;

	for (n = 1; n <= code->args; n++) {
	  args[n - 1].nest = tocontinue (&ct);
	  ct[-1] = '\0';
	  args[n].str = ct;
	}

	code->func (args, file);
	for (n = 1; n <= code->args; n++)
	  args[n].str[-1] = '\003';
      }
    }
  }
}

static void
pfilter (struct arg *args, FILE * file)
{
  FILE *a;
  char x[BUFSIZ];

  if ((a = popen (args[0].str, "r")) == NULL)
    mudlog ("ERROR: [Cannot find filter ->%s]\n", args[0].str);
  else {
    while (fgets (x, sizeof x, a))
      kiputs (x, file);
    pclose (a);
  }
}

static void
pfile (struct arg *args, FILE * file)
{
  FILE *a;
  char x[BUFSIZ];

  if ((a = fopen (args[0].str, "r")) == NULL)
    mudlog ("ERROR: [Cannot find file ->%s]\n", args[0].str);
  else {
    while (fgets (x, sizeof (x), a))
      kiputs (x, file);
    fclose (a);
  }
}

static void
pndeaf (struct arg *args, FILE * file)
{
  if (!ststflg (mynum, SFL_DEAF)) {
    prnt (args[0], file);
  }
}

static void
pcls (struct arg *args, FILE * file)
{
  bprintf ("\033[2J\033[H");
}


static void
pcansee (struct arg *args, FILE * file)
{
  int a;

  a = fpbns (args[0].str);
  if (seeplayer (a) && (a != -1)) {
    prnt (args[1], file);
  }
}


static void
prname (struct arg *args, FILE * file)
{
  kiputs (seeplayer (fpbns (args[0].str)) ? args[0].str : "Someone", file);
}

static void
pryou (struct arg *args, FILE * file)
{
  kiputs ((fpbns (args[0].str) == mynum) ? "you" : args[0].str, file);
}

/*
 * Prints "your" if player is the receiver, otherwise prints the
 * possessive form of the receiver's name
 */

static void
pryour (struct arg *args, FILE * file)
{
  if (fpbns (args[0].str) == mynum)
    kiputs ("your", file);
  else {
    kiputs (args[0].str, file);
    kiputs ("'s", file);
  }
}


static void
pndark (struct arg *args, FILE * file)
{
  if ((!isdark ()) && (!ststflg (mynum, SFL_BLIND))) {
    prnt (args[0], file);
  }
}

static void
pansi (struct arg *args, FILE * file)
{
  if (cur_player->iamon && ststflg (mynum, SFL_COLOR)) {
    prnt (args[0], file);
  }
}

static void
ppndeaf (struct arg *args, FILE * file)
{
  if (!ststflg (mynum, SFL_DEAF))
    prname (args, file);
}

static void
ppnblind (struct arg *args, FILE * file)
{
  if (!ststflg (mynum, SFL_BLIND))
    prname (args, file);
}

void
print_buf (char *b, Boolean notself)
{
  int plr;
  int ct = 0;
  int me = real_mynum;
  Boolean n1, n2;

  if (cur_player->in_pbfr)
    return;
  cur_player->in_pbfr = True;
  n1 = new_line;
  if (!notself)
    dcprnt (b, cur_player->stream);
  n2 = new_line;
  if (cur_player->iamon && ploc (mynum) < 0 && !EMPTY (pname (mynum)) &&
      cur_player->snooped > 0) {
    ++snooplev;
    for (plr = 0; plr < max_players; plr++) {
      if (!is_in_game (plr))
	continue;
      if (players[plr].snooptarget == me) {
	++ct;
	xsetup_globals (plr);
	new_line = n1;
	print_buf (b, False);
	fflush (cur_player->stream);
      }
    }
    xsetup_globals (me);
    new_line = n2;
    --snooplev;
    if (ct != cur_player->snooped) {
      mudlog ("ERROR: Internal error, snooped = %d, ct = %d, check print_buf",
	      cur_player->snooped, ct);
    }
  }
  cur_player->in_pbfr = False;
}

void
pbfr (void)
{
  if (sysbuf == NULL) {
    makebfr ();
  }
  if (buflen > 0 && cur_player != NULL && cur_player->stream != NULL) {
    print_buf (sysbuf, False);
    sysbuf[0] = 0;		/* clear buffer */
    bufptr = sysbuf;
    buflen = 0;
    new_line = True;
  }
}

void
bflush (void)
{
  if (cur_player != NULL && cur_player->stream != NULL) {
    pbfr ();
    fflush (cur_player->stream);
  }
}


void
snoopcom (void)
{
  Boolean stop_snoop = False;
  int plr;

  if (!ptstflg (mynum, PFL_SNOOP)) {
    erreval ();
    return;
  }
  if (cur_player->snooptarget >= 0) {
    bprintf ("Stopped snooping on %s.\n", pname (cur_player->snooptarget));
    snoop_off (mynum);
    stop_snoop = True;
  }
  if ((plr = pl1) < 0) {
    if (!stop_snoop)
      bprintf ("Snoop Who?\n");
    return;
  } else if (plr >= max_players) {
    bprintf ("You can't snoop %s!\n", pname (plr));
    return;
  } else if (plr == mynum) {
    bprintf ("You can't snoop yourself!\n");
    return;
  }
  if (!do_okay (mynum, plr, PFL_NOSNOOP)) {
    bprintf ("Your magical vision is obscured.\n");
    return;
  }
  /* Is this a PRIVATE room?  If so don't let them snoop. */
  if (ltstflg (ploc (plr), LFL_PRIVATE) && plev (mynum) < LVL_ARCHWIZARD) {
    bprintf ("I'm sorry, %s, but the room is private.\n",
	     psex (mynum) ? "Madam" : "Sir");
    return;
  }
  cur_player->snooptarget = plr;
  ++(players[plr].snooped);	/* One more to snoop him */

  bprintf ("Started to snoop on %s.\n", pname (plr));
}

/* Make player no longer snoop his target
 */
void
snoop_off (int plr)
{
  int target;

  if ((target = players[plr].snooptarget) >= 0) {

    players[plr].snooptarget = -1;

    /* One less to snoop him: */
    if (--players[target].snooped < 0)
      players[target].snooped = 0;
  }
}


#ifdef VARGS

void
bprintf (char *format,...)
{
  va_list pvar;
  register int len;

  if (!sysbuf)
    makebfr ();

  if (cur_player == NULL) {
    return;
    va_start (pvar, format);
    vprintf (format, pvar);
    va_end (pvar);
  } else if (cur_player->stream == NULL) {
    return;
  } else {
    va_start (pvar, format);
    vsprintf (bufptr, format, pvar);
    len = strlen (bufptr);
    buflen += len;
    va_end (pvar);

    if (buflen >= SYSBUFSIZE)
      pbfr ();
    else
      bufptr += len;
  }
}

#else

void
bprintf (char *format, char *a1, char *a2, char *a3, char *a4, char *a5,
	 char *a6, char *a7, char *a8, char *a9)
{
  register int len;

  if (!sysbuf)
    makebfr ();

  if (cur_player == NULL) {
    printf (format, a1, a2, a3, a4, a5, a6, a7, a8, a9);
  } else {
    sprintf (bufptr, format, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    len = strlen (bufptr);
    buflen += len;
    if (buflen >= SYSBUFSIZE)
      pbfr ();
    else
      bufptr += len;
  }
}

#endif

/****************************************************************
 * iDiRT Color Parser						*
 *								*
 * Handles color parsing of output/input to the game.		*
 *								*
 * Written by ErIC for Northern Lights, 1993			*
 * [Changes by Illusion]					*
 *								*
 * This parser handles screen clearing codes and newline codes.	*
 * (Illusion, 1995)						*
 *								*
 * Blinking Characters Added					*
 * (Illusion, April 27, 1995)					*
 ****************************************************************/

#define colorcode(x) ( (x>=64 && x<=127) ? color_table[x-64] : 0 )

char color_table[] =
{				/* Not beautiful, but efficient :) */
  0, 0, '4', '6',
  0, 0, 0, '2',
  0, 0, 0, 0,
  '0', '5', 0, 0,
  0, 0, '1', 0,
  0, 0, 0, '7',
  0, '3', 0, 0,
  0, 0, 0, 0,
  0, 0, '4', '6',
  0, 0, 0, '2',
  0, 0, 0, 0,
  '0', '5', 0, 0,
  0, 0, '1', 0,
  0, 0, 0, '7',
  0, '3', 0, 0,
  0, 0, 0, 0,
};

void
fix_color (char *dests, char *srcs)
{

  unsigned char *dest = (unsigned char *) dests;
  unsigned char *src = (unsigned char *) srcs;

  while (*src != 0) {
    if (*src != '&') {
      *dest++ = *src++;
    } else {
      switch (*(src + 1)) {

      case '&':
	src += 2;
	*dest++ = '&';
	continue;

      case '+':
	if (colorcode (*(src + 2))) {
	  if (ststflg (mynum, SFL_COLOR)) {
	    strcpy ((char *) dest, "\033[1;30m");
	    dest[5] = colorcode (*(src + 2));
	    if (*(src + 2) >= 96)
	      dest[2] = '0';
	    dest += 7;
	  }
	  src += 3;
	  continue;
	} else {
	  *dest++ = *src++;
	  continue;
	}

      case '-':
	if (colorcode (*(src + 2))) {
	  if (ststflg (mynum, SFL_COLOR)) {
	    strcpy ((char *) dest, "\033[1;40m");
	    dest[5] = colorcode (*(src + 2));
	    if (*(src + 2) >= 96)
	      dest[2] = '0';
	    dest += 7;
	  }
	  src += 3;
	  continue;
	} else {
	  *dest++ = *src++;
	  continue;
	}

      case '=':
	if (colorcode (*(src + 2)) && colorcode (*(src + 3))) {
	  if (ststflg (mynum, SFL_COLOR)) {
	    strcpy ((char *) dest, "\033[1;40;30m");
	    dest[5] = colorcode (*(src + 3));
	    dest[8] = colorcode (*(src + 2));
	    if (*(src + 2) >= 96)
	      dest[2] = '0';
	    dest += 10;
	  }
	  src += 4;
	  continue;
	} else {
	  *dest++ = *src++;
	  continue;
	}

      case '*':
	if (ststflg (mynum, SFL_COLOR)) {
	  strcpy ((char *) dest, "\033[40m\033[0m");
	  dest += strlen ((char *) dest);
	}
	src += 2;
	continue;

	/* Blinking */
      case 'B':
	if (ststflg (mynum, SFL_COLOR)) {
	  if (ststflg (mynum, SFL_NOBLINK))
	    strcpy ((char *) dest, "");
	  else
	    strcpy ((char *) dest, "\033[5m");
	  dest += strlen ((char *) dest);
	}
	src += 2;
	continue;

	/* Carriage return for titles */
      case '>':
	strcpy ((char *) dest, "\r\n                 ");
	dest += strlen ((char *) dest);
	src += 2;
	continue;

	/* Carriage return */
      case '/':
	strcpy ((char *) dest, "\r\n");
	dest += strlen ((char *) dest);
	src += 2;
	continue;

	/* Beep */
      case '#':
	if (ststflg (mynum, SFL_NOBEEP))
	  strcpy ((char *) dest, "");
	else
	  strcpy ((char *) dest, "\a");
	dest += strlen ((char *) dest);
	src += 2;
	continue;

      default:
	*dest++ = *src++;
	continue;
      }
    }
  }
  *dest = 0;
}

void
strip_color (char *dests, char *srcs)
{

  unsigned char *dest = (unsigned char *) dests;
  unsigned char *src = (unsigned char *) srcs;

  while (*src != 0) {
    if (*src != '&') {
      *dest++ = *src++;
    } else {
      switch (*(src + 1)) {

      case '&':
	src += 2;
	*dest++ = '&';
	continue;

      case '+':
	if (colorcode (*(src + 2))) {
	  src += 3;
	  continue;
	} else {
	  *dest++ = *src++;
	  continue;
	}

      case '-':
	if (colorcode (*(src + 2))) {
	  src += 3;
	  continue;
	} else {
	  *dest++ = *src++;
	  continue;
	}

      case '=':
	if (colorcode (*(src + 2)) && colorcode (*(src + 3))) {
	  src += 4;
	  continue;
	} else {
	  *dest++ = *src++;
	  continue;
	}

      case '*':
	src += 2;
	continue;

      case 'B':
	src += 2;
	continue;

      case '#':
	src += 2;
	continue;

      default:
	*dest++ = *src++;
	continue;
      }
    }
  }
  *dest = 0;
}

/* Checks for special codes in output
 * 1995, Illusion
 */
void
special_codes (char *dests, char *srcs)
{
  unsigned char *dest = (unsigned char *) dests;
  unsigned char *src = (unsigned char *) srcs;
  int ct;

  while (*src != 0) {
    if (*src != '@') {
      *dest++ = *src++;
    } else {
      switch (*(src + 1)) {

      case '@':
	src += 2;
	*dest++ = '@';
	continue;

	/* Display Version Number */
      case 'V':
	strcpy ((char *) dest, _VERSION_);
	dest += strlen ((char *) dest);
	src += 2;
	continue;

	/* Clear screen */
      case 'C':
	strcpy ((char *) dest, "\033[2J\033[H");
	dest += strlen ((char *) dest);
	src += 2;
	continue;

	/* Display a File */
      case 'F':
	ct = 0;

      default:
	*dest++ = *src++;
	continue;
      }
    }
  }
  *dest = 0;
}

/****************************************
 * File Pager                           *
 * 1995, Illusion                       *
 ****************************************/

void
file_pager (char filename[512])
{
  FILE *file;

  if (!ppager (mynum)) {
    bprintf ("\001f%s\003", filename);
    return;
  }
  if ((file = fopen (filename, "r")) == NULL) {
    bprintf ("&+RError reading file: &+Y%s\n", filename);
    mudlog ("ERROR: Cannot read file: %s", filename);
    return;
  }
  cur_player->pager.old_handler = (INP_HANDLER *) cur_player->inp_handler->inp_handler;

  cur_player->inpager = True;
  cur_player->pager.file = file;
  pager (NULL);
  return;
}

void
quit_pager (void)
{
  cur_player->inpager = False;
  replace_input_handler ((void *) cur_player->pager.old_handler);
  fclose (cur_player->pager.file);
  bprintf ("\n%s", cur_player->cprompt);

  if (cur_player->inmailer)
    bprintf ("%s", cur_player->cprompt);

  return;
}

void
pager (char *c)
{
  char pageprompt[] = "&+W(&*Press &+W[&+CReturn&+W] &*to continue, "
  "&+W'&+CQ&+W' &*to quit, &+W'&+C!<command>&+W' "
  "&*to use a MUD command&+W)&*";
  char ch[255];
  int ct;
  int len = ppager (mynum);

  if (c == NULL) {
    for (ct = 0; ct != len; ct++) {
      if (feof (cur_player->pager.file)) {
	quit_pager ();
	return;
      }
      ch[0] = '\0';
      fgets (ch, 250, cur_player->pager.file);
      bprintf ("%s", ch);
    }
    bprintf ("%s", pageprompt);
    replace_input_handler (pager);
  } else {
    if (c[0] == 'q' || c[0] == 'Q' || feof (cur_player->pager.file)) {
      quit_pager ();
      return;
    }
    if (c[0] == '!') {
      gamecom (&c[1], False);
      bprintf ("%s", pageprompt);
      return;
    }
    for (ct = 0; ct != len; ct++) {
      if (feof (cur_player->pager.file)) {
	quit_pager ();
	return;
      }
      ch[0] = '\0';
      fgets (ch, 250, cur_player->pager.file);
      bprintf ("%s", ch);
    }

    bprintf ("%s", pageprompt);
    return;
  }
}
