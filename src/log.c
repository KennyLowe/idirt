
#include "kernel.h"

#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include "log.h"


int
open_logfile (const char *logfile, Boolean clear_flag)
{
  int fd;
  int x = O_WRONLY | O_CREAT;

  if (clear_flag)
    x |= O_TRUNC;
  else
    x |= O_APPEND;

  if ((fd = open (logfile, x, S_IRUSR | S_IWUSR)) < 0) {
    fprintf (stderr, "\n%s: Cannot open logfile %s, program failed.\n",
	     progname, logfile);
    perror ("open");
    return -1;
  }
  dup2 (fd, fileno (stderr));
  close (fd);
  return 0;
}

void
close_logfile (void)
{
  fclose (stderr);
}

void
progerror (const char *name)
{
  mudlog ("PERROR %s: [%d] %s", name, errno, strerror(errno));
}

void
vmudlog (const char *format, va_list pvar)
{
  time_t tm_t;
  char *z, timestr[25];
  int pos;

  time (&tm_t);
  z = ctime (&tm_t);
  z[19] = '\0';

#ifdef COMPACT_LOG
  for (pos = 0; pos < 15; pos++)
    timestr[pos] = z[pos + 4];
  timestr[pos] = '\0';
  fprintf (stderr, "%s: ", timestr);
#else
  fprintf (stderr, "%s: ", z);
#endif

  vfprintf (stderr, format, pvar);
  putc ('\n', stderr);
  fflush (stderr);
}

void
mudlog (const char *format,...)
{
  va_list pvar;

  va_start (pvar, format);
  vmudlog (format, pvar);
  va_end (pvar);
}


/************************************************
 * Player Logging                               *
 * 1995, Illusion                               *
 ************************************************/
void
open_plr_log (void)
{
  char filename[100];

  sprintf (filename, "%s/%s", LOG_DIR, pname (mynum));

  if ((cur_player->log = fopen (filename, "a")) == NULL) {
    cur_player->logged = False;
    return;
  }
  cur_player->logged = True;
  write_plr_log ("LOG: Log Started");
  mudlog ("MONITOR: Player Log Opened For %s", pname (mynum));
}

void
close_plr_log (void)
{
  if (cur_player->logged) {
    write_plr_log ("LOG: Log Closed");
    fclose (cur_player->log);
    cur_player->logged = False;
    mudlog ("MONITOR: Player Log Closed For %s", pname (mynum));
  }
}

void
vwrite_plr_log (const char *format, va_list pvar)
{
  time_t tm_t;
  char *z, timestr[25];
  int pos;

  time (&tm_t);
  z = ctime (&tm_t);
  z[19] = '\0';

#ifdef COMPACT_LOG
  for (pos = 0; pos < 15; pos++)
    timestr[pos] = z[pos + 4];
  timestr[pos] = '\0';
  fprintf (cur_player->log, "%s: ", timestr);
#else
  fprintf (cur_player->log, "%s: ", z);
#endif

  vfprintf (cur_player->log, format, pvar);
  putc ('\n', cur_player->log);
}

void
write_plr_log (const char *format,...)
{
  va_list pvar;

  if (!cur_player->logged)
    return;

  va_start (pvar, format);
  vwrite_plr_log (format, pvar);
  va_end (pvar);
  fflush (cur_player->log);
}
