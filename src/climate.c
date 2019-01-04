
/************************************************************************
 * Climate Routines							*
 * 1995, Illusion							*
 ************************************************************************/

#include <stdlib.h>
#include "kernel.h"
#include "sendsys.h"
#include "climate.h"
#include "pflags.h"
#include "sflags.h"
#include "lflags.h"
#include "bprintf.h"
#include "parse.h"

/* The current weather is defined by the_climate->weather. The states are:
 *
 *      0 Clear         1 Showers       2 Rain          3 Stormy
 *      4 Hailing       5 Drizzle       6 Snow          7 Blizzard
 *
 * The current season is stored in the_climate->season. The states are:
 *
 *      0 Winter        1 Spring        2 Summer        3 Fall
 */

/* Modifies the weather depending on the location and current season. */
int
modify_weather (int loc)
{
  if (ltstflg (loc, LFL_COLD) || the_climate->season == winter) {
    if (the_climate->weather >= showers && the_climate->weather <= stormy)
      return the_climate->weather + 4;
    else
      return the_climate->weather;
  }
  if (ltstflg (loc, LFL_HOT) || the_climate->season == summer) {
    if (the_climate->weather >= drizzle && the_climate->weather <= blizzard)
      return the_climate->weather - 4;
    else
      return the_climate->weather;
  }
  return the_climate->weather;
}

/* The test function to see which users see the weather change. */
char *
test_func (int plr, int arg, char *msg)
{
  if (!ltstflg (ploc (plr), LFL_OUTDOORS) || ststflg (plr, SFL_QUIET) ||
      ststflg (plr, SFL_NOWET))
    return NULL;

  if (climate_day ()) {
    switch (modify_weather (ploc (plr))) {
    case clear:
      return "The sun comes out of the clouds, casting warmth across the land.\n";
    case showers:
      return "You look up as small drops of rain fall from the clouds.\n";
    case rain:
      return "Clouds form in the sky and a rain breaks out across the land.\n";
    case stormy:
      return "Dark clouds boil across the sky as a heavy storm breaks.\n";
    case drizzle:
      return "You shiver as it begins to drizzle.\n";
    case snow:
      return "You smile happily as the first snowflakes fall to the ground.\n";
    case blizzard:
      return "A strong wind kicks up as a blizzard sweeps across the land.\n";
    case hailing:
      return "You head for cover as hail starts to fall from the heavens.\n";
    }
  } else {
    switch (modify_weather (ploc (plr))) {
    case clear:
      return "The moon peeks out from behind the clouds.\n";
    case showers:
      return "You look up as small drops of rain fall from the clouds.\n";
    case rain:
      return "Clouds form in the sky and a rain breaks out across the land.\n";
    case stormy:
      return "Clouds form blocking out the stars as a heavy storm breaks.\n";
    case drizzle:
      return "You shiver as it begins to drizzle.\n";
    case snow:
      return "You smile happily as the first snowflakes fall to the ground.\n";
    case blizzard:
      return "A strong wind kicks up as a blizzard sweeps across the land.\n";
    case hailing:
      return "You head for cover as hail starts to fall from the heavens.\n";
    }
  }
  return NULL;
}

/* Tells the users that are outside the new weather conditions. */
void
adjust_weather (weather_type new)
{
  if (the_climate->weather != new) {
    the_climate->weather = new;
    send_g_msg (DEST_ALL, test_func, 0, NULL);
  }
}

/* Tells a player the weather as he/she enters a room. */
void
show_weather (void)
{
  if (!ltstflg (ploc (mynum), LFL_OUTDOORS))
    return;

  switch (modify_weather (ploc (mynum))) {
  case clear:
    bprintf ("%s\n", climate_day ()?
	     "The warm sun is clearly visible in the sky." :
	     "The moon casts a faint glow of light over the land.");
    break;
  case showers:
    bprintf ("%s\n", climate_day ()?
       "A light rain covers the land, creating a rainbow high in the sky." :
	     "A light rain covers the land making it damp and cool.");
    break;
  case rain:
    bprintf ("It is raining.\n");
    break;
  case stormy:
    bprintf ("%s\n", climate_day ()?
	     "The skies are dark and stormy." :
	     "Storm clouds block out the moon and the stars.");
    break;
  case drizzle:
    bprintf ("A light drizzle covers the land, making it chilly.\n");
    break;
  case snow:
    bprintf ("%s\n", climate_day ()?
	  "Soft snowflakes fall from the sky, glittering in the sunshine." :
	     "Snowflakes fall to the ground, surrounding you in white.");
    break;
  case blizzard:
    bprintf ("A blizzard is howling around you.\n");
    break;
  case hailing:
    bprintf ("It is hailing.\n");
    break;
  }
}

/* Called by the MUD to change the weather randomly. */
void
change_weather (void)
{
  int w = randperc ();

  if (randperc () >= 5)
    return;

  if (w <= 2)
    adjust_weather (clear);
  else if (w > 14 && w <= 16)
    adjust_weather (showers);
  else if (w > 28 && w <= 30)
    adjust_weather (rain);
  else if (w > 42 && w <= 44)
    adjust_weather (stormy);
  else if (w > 56 && w <= 58)
    adjust_weather (drizzle);
  else if (w > 70 && w <= 72)
    adjust_weather (snow);
  else if (w > 84 && w <= 86)
    adjust_weather (blizzard);
  else if (w > 98 && w <= 100)
    adjust_weather (hailing);
}

/* One word description of the weather */
char *
weather_desc (weather_type type)
{
  switch (type) {
  case clear:
    return "Clear";
  case showers:
    return "Showers";
  case rain:
    return "Rain";
  case stormy:
    return "Stormy";
  case hailing:
    return "Hailing";
  case drizzle:
    return "Drizzle";
  case snow:
    return "Snow";
  case blizzard:
    return "Blizzard";
  default:
    return "Clear";
  }
}

/* Quick description of the weather */
char *
climate_desc (weather_type type)
{
  switch (modify_weather (ploc (mynum))) {
  case clear:
    return "beautiful";
  case showers:
    return "slightly rainy";
  case rain:
    return "rainy";
  case stormy:
    return "stormy";
  case hailing:
    return "very stormy";
  case drizzle:
    return "drizzly";
  case snow:
    return "snowy";
  case blizzard:
    return "very snowy";
  default:
    return "beautiful";
  }
}

void
weathercom (void)
{
  static char *weather_table[] =
  {"clear", "showers", "rain", "stormy", "hailing",
   "drizzle", "snow", "blizzard", "list", TABLE_END};

  int x, list = 8;

  if (!ptstflg (mynum, PFL_WEATHER)) {
    erreval ();
    return;
  }
  if (brkword () == -1) {
    bprintf ("Change the weather to what? (List for listing)\n");
    return;
  }
  if ((x = tlookup (wordbuf, weather_table)) < 0) {
    bprintf ("I've never heard of that kind of weather.\n");
    return;
  }
  if (x == list) {
    bprintf ("Weather Types: \n\tClear, Showers, Rain, Stormy, "
	     "Hailing, Drizzle, Snow, Blizzard.\n");
    return;
  }
  adjust_weather (x);
  bprintf ("Changed weather to &+W%s&+w.\n", weather_desc (x));
  send_msg (DEST_ALL, MODE_SFLAG | MS (SFL_SEEEXT), LVL_MIN, LVL_MAX, mynum,
	    NOBODY, "&+B[&+WWeather &*by &+C\001p%s\003&*: &+W%s&+B]\n",
	    pname (mynum), weather_desc (x));
}

/************************************************************************
 * Climate Time System							*
 * Currently, the time system uses the MUD's internal clock tick system	*
 * which operates every 2 seconds. The climate system adds one minute	*
 * to the climate time every clock tick. With this system it takes the	*
 * climate system the following amounts of time to complete the 	*
 * specified cycles:							*
 *									*
 *	Climate Time	Real Time					*
 *	------------	---------					*
 *	2 Minute	2 Seconds					*
 *	1 Hour		1 Minutes					*
 *	1 Day		24 Minutes					*
 *	1 Month		12 Hours					*
 *	1 Year		6 Days						*
 ************************************************************************/

/* Move the climate time forward, and check to see if any of the cycles
 * have been completed. */
void
move_time (void)
{
  Boolean chk_light;

  the_climate->time += 2;

  if (the_climate->month == 12 && the_climate->day == 30 &&
      the_climate->time == 1440) {
    the_climate->month = 1;
    the_climate->day = 1;
    the_climate->time = 0;
  }
  if (the_climate->day == 30 && the_climate->time == 1440) {
    ++the_climate->month;
    the_climate->day = 1;
    the_climate->time = 0;
  }
  if (the_climate->time == 1440) {
    ++the_climate->day;
    the_climate->time = 0;
  }
  which_season ();
  chk_light = the_climate->daytime;
  the_climate->daytime = climate_day ();

  if (the_climate->daytime != chk_light) {
    if (the_climate->daytime)
      send_msg (DEST_ALL, MODE_NSFLAG | MS (SFL_NOWET), LVL_MIN, LVL_MAX,
		NOBODY, NOBODY,
		"The world starts to grow brighter as rays of sunlight "
		"reach across\nthe land, stretching from the eastern "
		"horizon. As the stars and moon\nstart to slowly fade "
		"away the sun slowly creeps into the sky.\n");
    else
      send_msg (DEST_ALL, MODE_NSFLAG | MS (SFL_NOWET), LVL_MIN, LVL_MAX,
		NOBODY, NOBODY,
		"Darkness sweeps over the land as the sun slowly sinks "
		"behind the\nwestern horizon. As the last rays of light "
		"disappear, the stars and\nmoon become visible, casting "
		"a faint light across the land.\n");
  }
}

/* Return which season it currently is, and place it into the season variable.
 */
void
which_season (void)
{
  switch (the_climate->month) {
  case 12:
  case 1:
  case 2:
    if (the_climate->season != winter)
      send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
		"A strong chill is felt in the air as winter sets in.\n");
    the_climate->season = winter;
    break;
  case 3:
  case 4:
  case 5:
    if (the_climate->season != spring)
      send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	"The land begins to warm up and become alive as spring sets in.\n");
    the_climate->season = spring;
    break;
  case 6:
  case 7:
  case 8:
    if (the_climate->season != summer)
      send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
		"It begins to turn hot as summer sets in.\n");
    the_climate->season = summer;
    break;
  case 9:
  case 10:
  case 11:
    if (the_climate->season != fall)
      send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
		"It begins to get a little cooler as fall sets in.\n");
    the_climate->season = fall;
    break;
  }
}

/* Sets the climate time */
void
_set_time (int month, int day, int hr, int min)
{
  the_climate->month = month;
  the_climate->day = day;
  the_climate->time = min + (hr * 60);
  which_season ();
}

/* A command to let players set the climate time */
void
settimecom (void)
{
  char temp[10];
  int hr, min;

  if (!ptstflg (mynum, PFL_SETTIME)) {
    erreval ();
    return;
  }
  if (brkword () == -1) {
    bprintf ("Usage: settime [hour] [minute]\n");
    return;
  }
  hr = atoi (wordbuf);
  strcpy (temp, wordbuf);

  if (brkword () == -1) {
    bprintf ("Usage: settime [hour] [minute]\n");
    return;
  }
  min = atoi (wordbuf);

  if (hr < 0 || hr > 23) {
    bprintf ("Hour setting out of range.\n");
    return;
  }
  if (min < 0 || min > 59) {
    bprintf ("Minute setting out of range.\n");
    return;
  }
  sprintf (temp, "%s:%s", temp, wordbuf);

  bprintf ("Setting time to %s.\n", temp);
  _set_time (the_climate->month, the_climate->day, hr, min);
}

/* Figure out the MUD climate time, returning the number of hours and
 * minutes from midnight. */
void
climate_split_time (int time, int *min, int *hrs)
{
  int h = 0;

  if (time >= 60) {
    h = time / 60;
    time -= h * 60;
  }
  *hrs = h;

  *min = time;
}

/* Check to see if it's daytime or nighttime */
Boolean
climate_day (void)
{
  int h, m;

  climate_split_time (the_climate->time, &m, &h);

  /* It's after 9pm, and before 6am, so it's nighttime */
  if (h >= 21 || h <= 5)
    return False;
  else
    return True;
}

/* Return the climate time in hh:mm (with am/pm) */
char *
climate_time (void)
{
  Boolean am;
  static char time[8];
  int hr, min;

  *time = '\0';
  climate_split_time (the_climate->time, &min, &hr);

  if (hr <= 12) {
    if (hr == 12)
      am = False;
    else {
      if (hr == 0)
	hr = 12;
      am = True;
    }
  } else {
    am = False;
    hr -= 12;
  }

  sprintf (time, "%d:%s%d%s", hr, min < 10 ? "0" : "", min, am ? "am" : "pm");
  return (time);
}

/* Give the name of the current season. */
char *
season_name (season_type type)
{
  switch (type) {
  case winter:
    return "Winter";
  case spring:
    return "Spring";
  case summer:
    return "Summer";
  case fall:
    return "Fall";
  default:
    return "Unknown";
  }
}

/* Show the current climate time. */
void
climatetime (void)
{
  static char *Month[] =
  {NULL, "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  bprintf ("Date (Time): %s %d (%s)\n", Month[the_climate->month],
	   the_climate->day, climate_time ());

  bprintf ("It is a %s %s %s.\n", climate_desc (the_climate->weather),
	   season_name (the_climate->season),
	   climate_day ()? "day" : "night");
}
