

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

/* Return True if the time t occurs after a and before b, (between a and b).
 */
#if 0
static Boolean
between (struct tm *t, struct tm *a, struct tm *b)
{
  return (later (a, b) ? !between (t, b, a) : later (t, a) && !later (t, b));
}
#endif

/* Return a pointer to a string containing the a date, without the \n
 * supplied by ctime(3). If the argument is 0, use the current local time.
 */
const char *
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
  static const char *Carry[] =
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
