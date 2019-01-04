

#include <sys/resource.h>
#include <sys/signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "kernel.h"
#include "locations.h"
#include "climate.h"
#include "bprintf.h"
#include "sendsys.h"
#include "timing.h"
#include "mobile.h"
#include "cflags.h"
#include "objsys.h"
#include "spell.h"
#include "rooms.h"
#include "log.h"
#include "mud.h"

#define  SECS_IN_A_MIN          60
#define  SECS_IN_AN_HOUR        (SECS_IN_A_MIN*60)
#define  SECS_IN_A_DAY          (SECS_IN_AN_HOUR*24)
#define  SECS_IN_A_WEEK         (SECS_IN_A_DAY*7)

#define  WARNING_1  15		/* Minutes before closing time to give warnings.  */
#define  WARNING_2   3		/* Only used if the mud is not open 24 hrs. a day */

#define NUM_INTERV 30

static int wrap = 0;		/* Number of times we wrap around */
int intervals = 0;

struct tm open_times[NUM_INTERV];

#define MUD_ALWAYS   0
#define MUD_OPEN     1
#define MUD_WARNING1 2
#define MUD_WARNING2 3
#define MUD_CLOSED   (-1)

int open_state = MUD_OPEN;
int wrap_point = -1;

/************************************************************************
 * 									*
 *  "DIRT"'s system for controlling it's opening and closing times.     *
 *									*
 *  mud_open() is called. It reads DATA/hours and returns true if mud   *
 *  is open, false if not. Read DATA/=README= for the format. The       *
 *  opening time (if closed) or closing time (if open) is placed in     *
 *  the global variable 'next_event' and checked regularly. Two         *
 *  warnings are given before the players are kicked off at closing     *
 *  time.								*
 ***********************************************************************/

time_t next_event;		/* closing time if open, opening time if closed. */


/* Return True if the time represented by a is later then that of b :
 * This procedure is to be used by the boot, where one and only one
 * interval may be later so one occurence should return true,
 * remaining false.
 * (We are only using the day, hour and min fields in tm.)
 */
static Boolean
blater (struct tm *a, struct tm *b)
{

  if (a->tm_wday > b->tm_wday ||
      (a->tm_wday == b->tm_wday && a->tm_hour > b->tm_hour) ||
      (a->tm_wday == b->tm_wday && a->tm_hour == b->tm_hour &&
       a->tm_min > b->tm_min))
    return True;

  ++wrap;
  if (wrap_point >= 0)
    return False;

  wrap_point = a - open_times;

  return True;
}

/* Return True if the time represented by a is later then that of b :
 * (We are only using the day, hour and min fields in tm.)
 */
static Boolean
later (struct tm *a, struct tm *b)
{

  if (a->tm_wday > b->tm_wday ||
      (a->tm_wday == b->tm_wday && a->tm_hour > b->tm_hour) ||
      (a->tm_wday == b->tm_wday && a->tm_hour == b->tm_hour &&
       a->tm_min > b->tm_min))
    return True;

  return False;
}


/* Return True if the time t occurs after a and before b, (between a and b).
 */
static Boolean
between (struct tm *t, struct tm *a, struct tm *b)
{
  return (later (a, b) ? !between (t, b, a) : later (t, a) && !later (t, b));
}


/* Return the time difference between two events in seconds.
 */
static time_t
diff (struct tm *from, struct tm *to)
{
  if (later (from, to))
    return (SECS_IN_A_WEEK - diff (to, from));
  else
    return ((to->tm_wday - from->tm_wday) * SECS_IN_A_DAY +
	    (to->tm_hour - from->tm_hour) * SECS_IN_AN_HOUR +
	    (to->tm_min - from->tm_min) * SECS_IN_A_MIN);
}


static int
next_time (struct tm *now)
{
  struct tm *u;
  int x;

  for (x = 0, u = open_times; x < intervals; x++, u++) {
    if (later (u, now))
      break;
  }
  if (x == 0) {
    now->tm_wday += 7;
    for (x = 0, u = open_times; x < intervals; x++, u++) {
      if (later (u, now))
	break;
    }
    now->tm_wday -= 7;
  }
  if (x == intervals)
    x = 0;
  return x;
}

static Boolean
ok_week_time (struct tm *t, char *d, char *fname, int lineno)
{
  static char *wdays[] =
  {"Sunday", "Monday", "Tuesday", "Wednesday",
   "Thursday", "Friday", "Saturday", TABLE_END};

  int x;

  if ((x = tlookup (d, wdays)) < 0) {
    printf ("%s:%d - No such day: %s\n", fname, lineno, d);
    return False;
  }
  t->tm_wday = x;
  if (t->tm_hour < 0 || t->tm_hour >= 24 || t->tm_min < 0 || t->tm_min >= 60) {
    printf ("%s:%d -  Illegal time: %d:%d\n",
	    fname, lineno, t->tm_hour, t->tm_min);
    return False;
  }
  return True;
}

static Boolean
is_later (struct tm *b, struct tm *a,
	  char *fname, int lineno, int x)
{
  if (blater (b, a))
    return True;
  mudlog ("ERROR: %s:%d - Illegal format %s \'->\', not increasing times.",
	  fname, lineno, (x == 0) ? "before" : "after");
  return False;
}

/* If AberMUD is open, return True and the closing-time in the variable
 * pointed to by next_ev (global UNIX time). If the game is closed,
 * return False, and the opening time in the variable pointed to by next_ev.
 */
Boolean
mud_open (time_t * next_ev, time_t * current_time)
{
  struct tm *tm_now;
  int ct;
  time_t now = *current_time;

  now = round_to_min (now);
  tm_now = localtime (&now);

  if (intervals > 0) {
    ct = next_time (tm_now);
    *next_ev = now + diff (tm_now, open_times + ct);
    return ODD (ct);
  } else {
    *next_ev = TIME_NEVER;
    return True;
  }
}

static int
xboot_times (FILE * f, char *fname)
{
  struct tm *i = open_times;
  FILE *fl;
  char dnamefrom[20];
  char dnameto[20];
  char b[256];
  int status, ct = 0, lineno = 0;


  if ((fl = f) == NULL && (fl = fopen (fname, "r")) == NULL) {
    intervals = 0;
    next_event = TIME_NEVER;
    open_state = MUD_ALWAYS;
    return 0;
  }
  while (fgets (b, sizeof b, fl)) {
    lineno++;
    if (ct >= NUM_INTERV) {
      printf ("%s:%d - Too many intervals in hours file.\n",
	      fname, lineno);
      fclose (fl);
      return -1;
    }
    status = sscanf (b, "%s %d:%d %*s %s %d:%d\n",
		     dnamefrom, &i->tm_hour, &i->tm_min,
		     dnameto, &i[1].tm_hour, &i[1].tm_min);
    if (status <= 0)
      continue;
    if (status != 6) {
      fclose (fl);
      printf ("status == %d\n", status);
      printf ("%s:%d - Illegal format in hours file: %s", fname, lineno, b);
      return -1;
    }
    if (!ok_week_time (i, dnamefrom, fname, lineno)
	|| ((ct > 0) && !is_later (i, i - 1, fname, lineno, 0))
	|| !ok_week_time (++i, dnameto, fname, lineno)
	|| !is_later (i, i - 1, fname, lineno, 1)) {
      fclose (fl);

      return -1;
    }
    ct += 2;
    ++i;
  }
  if (!is_later (open_times, i - 1, fname, 1, 0)) {
    fclose (fl);
    return -1;
  }
  if (wrap != 1) {
    printf ("%s:%d - hours file should wrap once.\n", fname, lineno);
    fclose (fl);
    return -1;
  }
  if (!feof (fl)) {
    printf ("%s:%d [%d]%s\n", fname, lineno, errno, sys_errlist[errno]);
    fclose (fl);
    return -1;
  }
  if (f == NULL)
    fclose (fl);
  intervals = ct;
  for (ct = wrap_point; ct < intervals; ct++) {
    open_times[ct].tm_wday += 7;
  }
  return 0;
}

int
boot_hours (FILE * f, char *fname)
{
  int k;
  time_t now = time (NULL);

  if ((k = xboot_times (f, fname)) < 0) {
    intervals = 0;
    next_event = TIME_NEVER;
    open_state = MUD_ALWAYS;
  } else if (open_state == MUD_ALWAYS)
    return k;
  else if (mud_open (&next_event, &now))
    open_state = MUD_OPEN;
  else
    open_state = MUD_CLOSED;

  return k;
}


/* Check if closing time has arrived. If so, kick everyone off (except
 * the 'masteruser' if he is on). Give 2 warnings ahead of time.
 */
static void
check_if_closed (time_t * now)
{
  int time_left;

  if (next_event == TIME_NEVER)
    return;

  time_left = next_event - *now;

  switch (open_state) {
  case MUD_ALWAYS:
    return;
  case MUD_OPEN:
    if (time_left < WARNING_1 * SECS_IN_A_MIN) {
      bprintf ("  MUD is closing in %s.\a\n",
	       sec_to_str (round_to_min (time_left)));
      open_state = MUD_WARNING1;
    }
    break;
  case MUD_WARNING1:
    if (time_left < WARNING_2 * SECS_IN_A_MIN) {
      bprintf ("  MUD is closing in %s.\a\n",
	       sec_to_str (round_to_min (time_left)));
      open_state = MUD_WARNING2;
    }
    break;
  case MUD_WARNING2:
    if (time_left < 0) {
      broad ("\t\tMUD has closed. Thank you for playing...");
      open_state = MUD_CLOSED;
      if (mud_open (&next_event, now)) {
	mudlog ("ERROR: mud_open is not False");
      }
    }
    break;
  case MUD_CLOSED:
    if (time_left < 0) {
      open_state = MUD_OPEN;
      if (!mud_open (&next_event, now)) {
	mudlog ("ERROR: mud_open is not True");
      }
    }
  }
}


/* Return a pointer to a string containing the a date, without the \n
 * supplied by ctime(3). If the argument is 0, use the current local time.
 */
char *
time2ascii (time_t t)
{
  char *str;

  if (t == 0)
    t = global_clock;

  *strchr ((str = ctime (&t)), '\n') = '\0';

  return (t == TIME_UNAVAIL) ? "Not Available" : str;
}


/* The TIME command. List game time elapsed, current time, last reset
 * and closing time if applicable.
 */
void
timecom (void)
{
  bprintf ("Elapsed Time : ");
  eltime ();
  bprintf ("Current Time : %s\n", time2ascii (TIME_CURRENT));
  bprintf ("Last Reset   : %s\n", ctime (&last_reset));

  bprintf ("Last Healall : %s",
	   last_healall != last_startup ?
	   ctime (&last_healall) : "Never\n");

  bprintf ("Time to Next : %s\n",
	   HEALALL_TIME ?
	   time2ascii (global_clock - last_healall + HEALALL_TIME) :
	   "No Time Limit Has Been Set.");

  if (next_event != TIME_NEVER) {
    bprintf ("Closing Time : %s\n", time2ascii (next_event));
  }
}

void
uptimecom (void)
{
  bprintf ("Elapsed Time      : %s\n", sec_to_str (global_clock - last_startup));
  bprintf ("Been Up Since     : %s", ctime (&last_startup));
  bprintf ("Number of Resets  : %d\n", numresets);
  bprintf ("Number of Reboots : %d\n", numreboots);
  bprintf ("Recovered Crashes : %d\n", numcrashes);
}

/* Prints the game time elapsed. Called by timecom() and main.
 */
void
eltime (void)
{
  long int et;

  switch (et = gametime ()) {
  case TIME_NEVER:
    bprintf ("AberMUD has yet to ever start!");
    break;
  case TIME_UNAVAIL:
    bprintf ("Current time is unavailable!");
    break;
  default:
    if (et > SECS_IN_A_DAY)
      bprintf ("Over a day!");
    else
      bprintf ("%s", sec_to_str (et));
    break;
  }
  bprintf ("\n");
}


long int
gametime (void)
{
  return global_clock - last_reset;
}



/*  Takes a number of seconds as input and converts this to seconds,
 *  minutes, hours, days, which is returned if the pointers != NULL.
 */
static void
split_time (long int sec, int *secs, int *min, int *hrs, int *days)
{
  int s = 0, m = 0, h = 0, d = 0;

  if (sec >= SECS_IN_A_DAY) {
    d = sec / SECS_IN_A_DAY;

    sec -= d * SECS_IN_A_DAY;
  }
  if (sec >= SECS_IN_AN_HOUR) {
    h = sec / SECS_IN_AN_HOUR;

    sec -= h * SECS_IN_AN_HOUR;
  }
  if (sec >= SECS_IN_A_MIN) {
    m = sec / SECS_IN_A_MIN;

    sec -= m * SECS_IN_A_MIN;
  }
  s = sec;

  /* Assign return values:
   */
  if (secs != NULL)
    *secs = s;
  if (min != NULL)
    *min = m;
  if (hrs != NULL)
    *hrs = h;
  if (days != NULL)
    *days = d;
}


/*  Takes a number of seconds as input and returns a pointer to a string
 *  containing the amount of time in english, ex: "2 hours, 3 minutes"..etc..
 */
char *
sec_to_str (long int seconds)
{
  static char str[50];

  int sec, min, hrs, days;

  char aux[15];

  *str = '\0';

  split_time (seconds, &sec, &min, &hrs, &days);

  if (days > 0) {
    sprintf (str, "%d day%s", days, (days == 1) ? "" : "s");
  }
  if (hrs > 0) {
    if (days > 0)
      strcat (str, ", ");
    sprintf (aux, "%d hour%s", hrs, (hrs == 1) ? "" : "s");
    strcat (str, aux);
  }
  if (min > 0) {
    if (days > 0 || hrs > 0)
      strcat (str, ", ");
    sprintf (aux, "%d minute%s", min, (min == 1) ? "" : "s");
    strcat (str, aux);
  }
  if (sec > 0) {
    if (days > 0 || hrs > 0 || min > 0)
      strcat (str, ", ");
    sprintf (aux, "%d second%s", sec, (sec == 1) ? "" : "s");
    strcat (str, aux);
  }
  return (str);
}


/*  Takes a number of seconds as input and returns a pointer to a string
 *  containing hh:mm:ss.
 */
char *
sec_to_hhmmss (long int seconds)
{
  static char str[25];

  int sec, min, hrs, days;

  split_time (seconds, &sec, &min, &hrs, &days);

  if (days > 0) {
    hrs += days * 24;
  }
  sprintf (str, "%02d:%02d:%02d", hrs, min, sec);

  return (str);
}


/* Returns time t rounded to the nearest minute.
 */
time_t
round_to_min (time_t t)
{
  t += SECS_IN_A_MIN / 2;

  return (t -= t % SECS_IN_A_MIN);
}


/* Like ctime(3) but strips the seconds, year-part and the \n.
 */
char *
my_ctime (time_t * clock)
{
  char *t = ctime (clock);

  t[16] = '\0';
  return t;
}


void
set_timer (void)
{
  alarm (TIMER_INTERRUPT);
}

void
on_timer (void)
{
  static long int n = 0;
  static const int interupts_per_hour = 3600 / TIMER_INTERRUPT;
  int plx;

  if (++n % interupts_per_hour != 0 || time (&global_clock) == -1) {
    global_clock += TIMER_INTERRUPT;
  }
  check_if_closed (&global_clock);
  move_mobiles ();
  special_events (SP_ALL);
  change_weather ();
  move_time ();

  for (plx = 0; plx < max_players; plx++) {
    if (is_in_game (plx)) {
      if (IDLE_MAX)
	check_idle (plx);
      regenerate (plx);
      calib_player (plx);
      handle_duration (plx);
      trace_handler (plx);
      setup_globals (plx);
    }
  }
}

void
trace_handler (int plr)
{
  static char *Carry[] =
  {
    "In Room", "In Container", "Carried By", "Worn By",
    "Wielded By", "Worn and Wielded By"
  };

  TRACE *Trace = &players[plr].Trace;
  char output[256], buffer[256];
  int obj, loc;

  if (Trace->trace == -1)
    return;

  if (Trace->is_obj) {
    if (getobjloc (Trace->trace) == LOC_PIT_PIT) {
      sendf (plr, "&+G(&+WTrace: Object Has Been Pitted&+G)\n");
      Trace->trace = -1;
      return;
    }
    if (Trace->loc != oloc (Trace->trace) ||
	Trace->oloc != getobjloc (Trace->trace) ||
	Trace->carry != ocarrf (Trace->trace)) {
      Trace->loc = oloc (Trace->trace);
      Trace->oloc = getobjloc (Trace->trace);
      Trace->carry = ocarrf (Trace->trace);
      obj = Trace->trace;
      loc = oloc (Trace->trace);
      if (Trace->carry == IN_CONTAINER) {
	while (ocarrf (obj) == IN_CONTAINER) {
	  obj = loc;
	  loc = oloc (obj);
	}
	sprintf (output, "&+B(&+w%s%s&+B) ", Carry[IN_CONTAINER], oname (loc));
      } else {
	output[0] = '\0';
      }
      if (ocarrf (obj) >= CARRIED_BY)
	sprintf (output, "%s&+B(&+w%s%s&+B)", output, Carry[ocarrf (obj)],
		 pname (oloc (obj)));

      sendf (plr, "&+G(&+WTrace: &+C%s &+B[&+w%d&+B]&+w; Loc: %s &+C%s&+G)\n",
	     oname (Trace->trace), Trace->trace, output,
	     xshowname (buffer, Trace->oloc));
    }
  }
  if (!Trace->is_obj) {
    if (!is_in_game (Trace->trace) || pvis (Trace->trace) > plev (plr)) {
      sendf (plr, "&+G(&+WTrace: Person/Mobile Has Exited Game&+G)\n");
      Trace->trace = -1;
      return;
    }
    if (Trace->loc != ploc (Trace->trace)) {
      Trace->loc = ploc (Trace->trace);
      sendf (plr, "&+G(&+WTrace: &+C%s &+B[&+w%d&+B]&+w; Loc: &+C%s&+G)\n",
	     pname (Trace->trace), Trace->trace,
	     xshowname (buffer, Trace->loc));
    }
  }
}

void
check_idle (int plr)
{
  int itime = plev (plr) < LVL_WIZARD ? IDLE_MAX : IDLE_MAX + 3600;

  if (plev (plr) > LVL_PROPHET)
    return;

  if (global_clock > (plast_cmd (plr) + itime)) {
    send_msg (DEST_ALL, MODE_QUIET, max (pvis (plr), LVL_MIN), LVL_MAX, plr,
	      NOBODY, "&+B[&+W%s &+whas timed-out&+B]\n", pname (plr));
    p_crapup (plr, "\tYou have timed-out.\n", CRAP_SAVE | CRAP_RETURN);
  }
}
