/*
 * Utility program for AberMUD [Dirt 3.0]
 * (Modified for iDIRT by Illusion)
 *
 * Dumps the user-activity-file (uaf) given as argument in human-readable
 * form to the standard output. If no argument is given, the file UAF_RAND
 * is assumed.
 *
 * Options:    -l [level]   -Level. Only show players >= the specified level.
 *                           The default is 1, but ignore level if -a is on.
 *                           This option is really unneccesary on UNIX systems,
 *                           I'd like to remove it, but it's there...
 *
 *             -f           -Flags. Show full names of flags. Default is hex.
 *                           (Not imlemented yet.)
 *
 *             -h           -Header. Include a header that explains the
 *                           columns. Useful if the standard output is a tty.
 *
 *             -d [days]    -Show only players who have been on at least once
 *                           the last specified number of days.
 *
 *             -a           -All. Do not skip entries with empty names. For
 *                           debugging purposes where we want the whole file.
 *
 * Gjermund S. (Nicknack), March 1991
 * Illusion, January 1995
 */

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include "../kernel.h"
#include "../macros.h"
#include "../sflags.h"
#include "../pflags.h"
#include "../pflagnames.h"

extern int getopt (int argc, char **argv, char *optstr);
extern int optind;
extern char *optarg;

void usage (char *progname);

int
main (int argc, char *argv[])
{
  PERSONA p;
  FILE *fl;
  int option;
  char *filename;
  time_t earliest = 0;

  const int secs_in_a_day = 86400;

  int incl = 0, flags = 0, debug = 0, level = 1, days = 0;

  while ((option = getopt (argc, argv, "ad:l:fh")) != -1) {
    switch (option) {
    case 'a':
      debug++;
      break;
    case 'h':
      incl++;
      break;
    case 'f':
      flags++;
      break;
    case 'l':
      level = atoi (optarg);
      break;
    case 'd':
      days = atoi (optarg);
      break;
    case '?':
      usage (*argv);
      exit (1);
    }
  }

  filename = optind < argc ? argv[optind] : DATA_DIR UAF_RAND;

  if ((fl = fopen (filename, "r")) == NULL) {
    perror (filename);
    exit (1);
  }
  if (days > 0)
    earliest = time ((time_t *) NULL) - days * secs_in_a_day;

  if (incl) {
    printf ("Name        Sex    Level    Score      Str  PFlags\n");
    printf ("-----------------------------------------------------------------------\n");
  }
  while (fread (&p, sizeof (p), 1, fl) > 0) {
    if (!debug && (p.p_level < level || *p.p_name == '\0' ||
		   (days > 0 && p.p_last_on < earliest)))
      continue;

    printf ("%-13.12s%c   %7d %8d %8d  0x%08lx%08lx%08lx\n",
	    p.p_name,
	    xtstbit (p.p_sflags.l, SFL_FEMALE) ? 'F' : 'M',
	    p.p_level,
	    p.p_score,
	    p.p_strength,
	    p.p_pflags.u,
	    p.p_pflags.h,
	    p.p_pflags.l);
  }
  fclose (fl);
  return (0);
}

void
usage (char *progname)
{
  fprintf (stderr, "Usage: %s <options> <uaf-file>\n\n", progname);
  fprintf (stderr, "Options: -a        All entries (including empty ones)\n");
  fprintf (stderr, "         -d <#>    Only players on the last # days.\n");
  fprintf (stderr, "         -l <lvl>  Only players > level. Default = 1.\n");
  fprintf (stderr, "         -h        Include Header explaning output\n");
}
