

/****************************************************************
 * iDiRT 1.x							*
 * 1994-1996 by Illusion					*
 ****************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include "kernel.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "jmp.h"

#ifdef VARGS
#include <stdarg.h>
#endif

#include "mud.h"
#include "log.h"
#include "bootstrap.h"
#include "sendsys.h"
#include "mobile.h"
#include "commands.h"
#include "timing.h"
#include "exit.h"
#include "s_socket.h"
#include "locations.h"
#include "pflags.h"
#include "update.h"
#include "climate.h"
#include "reboot.h"
#include "objsys.h"
#include "bprintf.h"
#include "zones.h"
#include "uaf.h"
#include "fight.h"
#include "rooms.h"
#include "ver.h"
#include "main.h"

#if USE_IDENT
#include "ident.h"
#endif

static int xmain (int fd);
static int xmain_reboot (int fd);
static int go_background (int pid_fd);
static int check_pid (void);
static void get_options (int argc, char **argv);
static void usage (void);
static void fullusage (void);
static void rm_pid_file (void);
static void end_connection (void);
static void new_connection (int fd);
static void handle_packet (int fd);
static void sig_handler (int sig);
static void main_loop (int m_socket);

#define MAX_PLAYERS     50
#define F_OK            0
#define EXEC_LOC        "../bin"

/* Some local variables */
fd_set sockets_fds;
fd_set buffer_fds;

int mud_port = PORT;
int main_socket;
int width;
int fildes[2];
char pidfile[50];

Boolean kill_other_mud = False;
Boolean clear_syslog_file = False;
Boolean stay_foreground = False;
Boolean sig_term_happened = False;
Boolean sig_timer_happened = False;
Boolean auto_open = False;
int old_proc_num = 0;
int update = 0;

static int inp_buf_c = 0;
static struct timeval zerotime =
{0, 0};

int
main (int argc, char **argv, char **ep)
{
  int fd, x;

  envp = ep;
  progname = argv[0];

  get_options (argc, argv);	/* Parse command line */

  printf ("\n%s #%d Daemon Loading...\n", VERSION, linknumber ());
  if (data_dir == NULL) {
    printf ("  iDiRT Daemon Error: data_dir is a NULL value.\n");
    printf ("  Halting Daemon!\n");
    exit (1);
  }
  printf ("  Data Directory:   %s\n", data_dir);
  printf ("  Maximum Players:  %d\n", max_players);
  printf ("  Port Selected:    %d\n", mud_port);
  printf ("  Clear System Log: %s\n", clear_syslog_file ? "Yes" : "No");

  sprintf (pidfile, "%spid", data_dir);
  if (!access (pidfile, 0))
    printf ("  Kill Other MUD:   %s\n", kill_other_mud ? "Yes" : "Ask User");

  chdir (data_dir);

  /* Check if the PID_FILE exists, and what it contains if it does */
  fd = check_pid ();

  /* Check to see if the MUD should be opened automatically */
  if (auto_open) {
    printf ("  Automatically opening MUD\n");
    sprintf (pidfile, "%snologin", data_dir);
    remove (pidfile);
  }
  /* We arrive here only if we are to continue and now we are alone. */
  /* Also, the PID_FILE is opened */
  if (!old_proc_num)
    x = xmain (fd);
  else
    x = xmain_reboot (fd);

  unlink (PID_FILE);
  if (x < 0) {
    mudlog ("BOOTUP: Abnormal termination of mud");
    exit (1);
  }
  mudlog ("BOOTUP: Normal termination of mud");
  exit(0);
}

static void
connect_ok (char *h, int port)
{
  char b[80];

  sprintf (b, "  Connected to port %d on %s.\n", port, h);

  if (stay_foreground) {
    printf ("%s", b);
  } else {
    write (fildes[1], b, sizeof (b));
    close (fildes[1]);
  }
}

static int
xmain (int fd)
{
  int s, k;

  /* We arrive here only if we are to continue and now we are alone. */
  /* Also, the PID_FILE is opened */

  if (open_logfile (LOG_FILE, clear_syslog_file) < 0) {
    close (fd);
    return -1;
  }
  if (bootstrap () < 0) {	/* Initialize data structures */
    printf ("\n\nBootstrap Has Failed, Please Read %s%s.\n",
	    data_dir, LOG_FILE);
    close (fd);
    return -1;
  }
  /* Now we go background */
  if (go_background (fd) < 0) {
    return -1;
  }
  k = 10;
  while ((main_socket = make_service (mud_port, my_hostname,
				      sizeof (my_hostname),
				      &my_hostent, &s_in)) == -4
	 && errno == EADDRINUSE && --k >= 0) {
    sleep (2);
  }

  s = main_socket;

  if (s < 0) {
    mudlog ("BOOTUP: Error code %d from make_service", s);
    progerror ("make_service");
    return -1;
  }
  if (s > 0) {			/* We want the main socket at fd 0 */
    dup2 (s, 0);
    close (s);
    s = main_socket = 0;
  }
  width = 1;

  connect_ok (my_hostname, mud_port);

  FD_ZERO (&sockets_fds);
  FD_ZERO (&buffer_fds);
  FD_SET (s, &sockets_fds);

  /* Initialize Variables */
  numresets = 0;
  numreboots = 0;
  numcrashes = 0;

  /* Initialize MUD Daemon Startup Time */
  time (&last_startup);

  /* Initialize MUD Time System */
  _set_time (1, 1, 0, 0);
  the_climate->daytime = False;

  /* Main program loop */
  main_loop (s);

  mudlog ("SYSTEM: Closing listening socket");
  close (s);

  return 0;
}

/* BSD style daemonizing */
static int
go_background (int fd)
{
  char b[80];
  int tty;

  /* Go background */
  signal (SIGHUP, SIG_IGN);
  signal (SIGINT, SIG_IGN);
  signal (SIGQUIT, SIG_IGN);
  signal (SIGTSTP, SIG_DFL);
  signal (SIGTTOU, SIG_DFL);
  signal (SIGTTIN, SIG_DFL);
  if (!stay_foreground) {
    if (pipe (fildes) < 0) {
      progerror ("go_background/pipe");
      exit (1);
    }
    switch (pid = fork ()) {
    case -1:
      progerror ("fork");
      close (fd);
      return -1;
    case 0:
      break;
    default:
      printf ("  PID: %d\n", pid);
      close (fd);
      close (fildes[1]);
      fflush (stdout);		/* Flush stdout */
      read (fildes[0], b, sizeof (b));
      printf ("%s", b);
      exit (0);			/* Let our parent process die */
    }
    fclose (stdin);
    fclose (stdout);
    close (fildes[0]);
  }
  pid = getpid ();		/* Get our process id */
  sprintf (b, "%d\n", pid);
  write (fd, b, strlen (b));
  close (fd);


  if (!stay_foreground) {

    if ((tty = open ("/dev/tty", O_RDWR, S_IRUSR | S_IWUSR)) >= 0) {

      if (ioctl (tty, TIOCNOTTY, 0) < 0) {
	progerror ("ioctl,TIOCNOTTY");
	return -1;
      }
      close (tty);

#ifdef _LINUX_			/* Linux made it alot simpler */
      setpgrp ();		/* Make our own process group */
#else
      setpgrp (pid, pid);	/* Make our own process group */
#endif

    } else if (errno != ENXIO) {
      progerror ("open,tty");
      return -1;
    }
    signal (SIGHUP, SIG_IGN);
    signal (SIGINT, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);
  } else {
    signal (SIGHUP, SIG_DFL);
    signal (SIGINT, sig_handler);
    signal (SIGQUIT, SIG_DFL);
  }

  signal (SIGTERM, sig_handler);
  signal (SIGTSTP, SIG_DFL);
  signal (SIGCONT, SIG_DFL);
  signal (SIGTTOU, SIG_DFL);
  signal (SIGTTIN, SIG_DFL);
  signal (SIGSEGV, sig_handler);/* Segmentation fault */
  signal (SIGBUS, sig_handler);	/* Bus error */
  signal (SIGSYS, sig_handler);	/* Bad argument to system call */
  signal (SIGPIPE, SIG_IGN);	/* Broken pipe */
  signal (SIGUSR1, sig_handler);/* User defined signal */
  signal (SIGUSR2, sig_handler);/* User defined signal */

  return 0;
}

static int
check_pid (void)
{
  int fd;
  int pid = -1;
  int c;
  FILE *f;
  char b[80];

  if ((fd = open (PID_FILE, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) < 0) {
    /* The file exist already */
    if (errno == EEXIST) {
      if ((f = fopen (PID_FILE, "r")) != NULL) {
	fgets (b, sizeof b, f);
	pid = atoi (b);
	fclose (f);
      } else {
	perror ("fopen," PID_FILE);
	exit (1);
      }
      if (pid > 0) {
	if (!kill_other_mud) {
	  printf ("  iDiRT is still running (or the PID file still exists), kill it? ");
	  while ((c = getchar ()) != 'N' && c != 'n' && c != 'Y' &&
		 c != 'y' && c != '\n' && c != '\r') ;
	  if (c == 'Y' || c == 'y')
	    kill_other_mud = True;
	}
	if (!kill_other_mud) {
	  printf ("  Aborting loading of this MUD.\n");
	  exit (0);
	}
	printf ("  Killing other MUD, loading this one (Old PID: %d)\n", pid);
	if (kill (pid, SIGTERM) < 0) {
	  if (errno != ESRCH) {
	    perror ("kill");
	    exit (1);
	  } else if (unlink (PID_FILE) < 0) {	/* PID_FILE without process */
	    perror ("unlink");
	    exit (1);
	  }
	}
      } else {
	if (unlink (PID_FILE) < 0) {	/* PID_FILE without process */
	  perror ("unlink");
	  exit (1);
	}
      }
      c = 6;
      while (True) {
	sleep (1);
	if ((fd = open (PID_FILE, O_WRONLY | O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR)) >= 0)
	  break;
	if (--c < 0) {
	  printf ("  Inactivity Timeout. Not loading MUD.\n");
	  exit (0);
	}
      }
    } else {
      perror ("open " PID_FILE);
      exit (1);
    }
  }
  return fd;
}


static void usage (void);

static void
get_options (int argc, char **argv)
{
  char *s;
  int x;

  if (argc == 1) {
    stay_foreground = False;
    clear_syslog_file = False;
    kill_other_mud = False;
    mud_port = PORT;
    max_players = 32;

    return;
  }
  while (--argc > 0) {
    s = *++argv;
    if (*s++ != '-') {
      usage ();
      exit (1);
    }
    x = *s++;
    switch (x) {
    case 'a':
      break;
    case 'h':
      usage ();
      break;
    case 'H':
      fullusage ();
      break;
    case 'p':
      if (*s != '\0' || (--argc > 0 && *(s = *++argv) != '\0')) {
	if ((mud_port = atoi (s)) < 1000 || mud_port > 65535) {
	  mud_port = PORT;
	}
      }
      break;
    case 'f':
      stay_foreground = True;
      break;
    case 'k':
      kill_other_mud = True;
      break;
    case 'c':
      clear_syslog_file = True;
      break;
    case 'o':
      auto_open = True;
      break;
    case 'v':
      printf ("%s #%d (%s)\n", VERSION, linknumber (), _HEADER_);
      exit (0);
      break;
    case 'V':
      printf ("\niDiRT Version Information\n");
      printf ("-------------------------\n");
      printf ("%s #%d (%s)\n", VERSION, linknumber (), _HEADER_);
      printf ("1994-1996 by Illusion\n\n");
      printf ("Derived from AberMUD DIRT (3.1.2)\n");
      printf ("1990, 1993 by Alf and Nicknack\n\n");
      exit (0);
      break;
    case 'u':
      update = 1;
    case 'r':
      if (*s != '\0' || (--argc > 0 && *(s = *++argv) != '\0'))
	old_proc_num = atoi (s);
      break;
    case 'n':
      if (*s != '\0' || (--argc > 0 && *(s = *++argv) != '\0')) {
	if ((max_players = atoi (s)) < 1 || max_players > 1000) {
	  max_players = 32;
	}
      }
      break;
    case 'd':
      if (*s != '\0' || (--argc > 0 && *(s = *++argv) != '\0')) {
	data_dir = s;
      }
      break;
    default:
      usage ();
      exit (1);
    }
  }
  if (argc > 0) {
    usage ();
    exit (1);
  }
}

static void
usage (void)
{
  fprintf (stderr, "usage: %s [-p port] [-d path] [-n #] [-f] [-k] [-c] [-v] [-V] [-o] [-H]\n", progname);
  fprintf (stderr, "(%s -H for detailed help)\n", progname);
  exit (1);
}

static void
fullusage (void)
{
  fprintf (stderr, "usage: %s [-p port] [-d path] [-n #] [-f] [-k] [-c] [-v] [-V] [-o] [-H]\n", progname);
  fprintf (stderr, "  -p port     : Alternate port to attach iDiRT to.\n");
  fprintf (stderr, "  -d path     : Path to data files.\n");
  fprintf (stderr, "  -n #        : Maximum number of players (Default is 32).\n");
  fprintf (stderr, "  -f          : Run iDiRT in the foreground.\n");
  fprintf (stderr, "  -k          : Automatically kill any other iDiRT Daemons that are running.\n");
  fprintf (stderr, "  -c          : Automatically clear system log.\n");
  fprintf (stderr, "  -v          : Display version.\n");
  fprintf (stderr, "  -V          : Display version information.\n");
  fprintf (stderr, "  -o          : Automatically open MUD.\n\n");
  exit (1);
}


static void new_connection (int fd);

static void
main_loop (int m_socket)
{
  int w;
  int v, i;
  int fd;
  int plx = 0;
  struct timeval *tv;
  fd_set r_fds, e_fds;

  time (&global_clock);
  time (&last_autosave);
  time (&last_healall);
  breset = False;
  norun = False;

  if (!old_proc_num)
    mudlog ("SYSTEM: iDiRT %s Daemon Started (PID: %d)", _VERSION_, getpid ());
  else
    mudlog ("SYSTEM: %s Successful, iDiRT Daemon Restarted",
	    update ? "Update" : "Reboot");

  if (clear_syslog_file)
    mudlog ("SYSTEM: System Log Cleared");

  /* Reset everything */
  if (!update) {
    last_reset = global_clock;
    setqdflags (0);

    for (i = 0; i < numzon; i++) {
      move_pouncie ();
      reset_zone (i, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }
  }
  cur_player = NULL;
  quit_list = -1;
  signal (SIGALRM, sig_handler);
  set_timer ();
  while (!sig_term_happened) {
    switch (v = setjmp (to_main_loop)) {
    case JMP_QUITTING:
      bflush ();
      end_connection ();
      setup_globals (-1);
      ++fd;
      break;
    }
    if (sig_timer_happened) {
      sig_timer_happened = False;
      signal (SIGALRM, sig_handler);
      setup_globals (-1);	/* bflush() and disable mynum */
      set_timer ();
      on_timer ();
    } else {
      while (quit_list >= 0)
	end_connection ();

      r_fds = e_fds = sockets_fds;

      if (inp_buf_c > 0)
	tv = &zerotime;
      else
	tv = NULL;

#ifdef SYS_HP_UX
      if ((v = select (width, (int *) &r_fds, NULL, (int *) &e_fds, tv)) < 0) {
#else
      if ((v = select (width, &r_fds, NULL, &e_fds, tv)) < 0) {
#endif

	if (errno == EINTR)
	  continue;
	progerror ("select");
	exit (1);
      }
      for (fd = 0, w = width; fd < w; fd++) {
	if (FD_ISSET (fd, &e_fds)) {
#ifdef LOG_FDPEND
	  mudlog ("SOCKET: Exception pending with fd = %d", fd);
#endif
	  --v;
	}
	if (FD_ISSET (fd, &r_fds) || FD_ISSET (fd, &buffer_fds)) {
	  --v;

	  if (fd == m_socket) {
	    new_connection (fd);
	  } else {
	    handle_packet (fd);
	  }
	  bflush ();
	}
	while (quit_list >= 0)
	  end_connection ();
      }
    }
  }
  mudlog ("SIGNAL: SIGTERM Handled");
  for (plx = 0; plx < max_players; plx++) {
    if (!EMPTY (pname (plx))) {
      setup_globals (plx);
      crapup ("\tSomething very unpleasant seems to have happened...\n",
	      CRAP_UNALIAS | CRAP_SAVE | CRAP_RETURN);
    }
  }
  rm_pid_file ();
}

static void
rm_pid_file (void)
{
  if (unlink (PID_FILE) < 0) {
    progerror ("rm_pid_file");
  }
}

static void
end_connection (void)
{
  INP_HANDLER *i, *j;
  int fd;
  int me = real_mynum;
  int x;

  if ((x = quit_list) < 0)
    return;
  if (x == real_mynum)
    me = -1;
  setup_globals (x);
  quit_list = cur_player->quit_next;
  cur_player->quit_next = -2;

  fd = cur_player->fil_des;
  bflush ();
  i = cur_player->inp_handler;
  while (i != NULL) {
    j = i;
    i = i->next;
    FREE (j);
  }
  cur_player->inp_handler = NULL;

  if (FD_ISSET (fd, &buffer_fds))
    inp_buf_c--;
  FD_CLR (fd, &sockets_fds);
  FD_CLR (fd, &buffer_fds);
  close (fd);
  fclose (cur_player->stream);
  cur_player->stream = NULL;
  cur_player->inp_buf_p = cur_player->inp_buf_end = cur_player->inp_buffer;
  cur_player->sock_buf_p = cur_player->sock_buf_end = cur_player->sock_buffer;
  cur_player->is_conn = False;

  setup_globals (me);
}


#ifdef SYS_INET_NTOA_BUG
/* If the system include file for inet_ntoa contains erroneous
 * prototypes.
 */
static char *
my_inet_ntoa (struct in_addr *in)
{
  static char addr[20];

  sprintf (addr, "%d.%d.%d.%d",
	   (int) ((in->s_addr >> 24) & 0xff),
	   (int) ((in->s_addr >> 16) & 0xff),
	   (int) ((in->s_addr >> 8) & 0xff),
	   (int) (in->s_addr & 0xff));

  return addr;
}

#endif


static void
new_connection (int m_socket)
{
  PLAYER_REC *pl;
  FILE *f;
  struct hostent *h;
  int plx;
  int fd;
  int sin_len;
  Boolean host_banned = False, host_b2 = False;
  Boolean user_banned = False, user_b2 = False;
  Boolean login_banned = False, login_b2 = False;
  struct sockaddr_in sin;
  char *host, *tmphost;
  char hostnum[MAXHOSTNAMELEN];

  bzero ((char *) &sin, sizeof (struct sockaddr_in));
  sin_len = sizeof (struct sockaddr_in);

  if ((fd = accept (m_socket, (struct sockaddr *) &sin, &sin_len)) < 0) {
    progerror ("accept");
  } else if ((f = fdopen (fd, "w")) == NULL) {
    progerror ("fdopen");
    exit (1);
  } else {

#ifdef SYS_INET_NTOA_BUG
    strcpy (hostnum, my_inet_ntoa (&sin.sin_addr));
#else
    strcpy (hostnum, inet_ntoa (sin.sin_addr));
#endif

    host_b2 = is_host_banned (hostnum);
    user_b2 = is_host_user_banned (hostnum);
    login_b2 = is_login_banned (hostnum);

    host = hostnum;

    if ((tmphost = read_hosts (host)) != NULL) {
      host = tmphost;
    } else {
      if ((h = gethostbyaddr ((char *) &sin.sin_addr, sizeof (sin.sin_addr),
			      AF_INET)) == NULL) {
#ifdef LOG_GETHOST
	mudlog ("SOCKET: gethostbyaddr: Couldn't find hostentry for %s", hostnum);
#endif
      } else {
	host_banned = is_host_banned ((char *) h->h_name);
	user_banned = is_host_user_banned ((char *) h->h_name);
	login_banned = is_login_banned ((char *) h->h_name);
	host = (char *) h->h_name;
      }
    }
    if (login_banned || login_b2) {
      send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_SEESOCKET), LVL_WIZARD,
	    LVL_MAX, NOBODY, NOBODY, "&+W[&+CSocket (%d): &+wBanned Login: "
		"&+C%s&+W]\n&+W[&+CSocket (%d): &+wClosing Connection&+W]\n",
		fd, host, fd);
      fprintf (f, "\nSorry, but your host is banned from logging in.\n");
      fflush (f);
      fclose (f);
      return;
    }
    if ((plx = find_free_player_slot ()) < 0) {
      fprintf (f, "\nSorry, but this mud is full, please come back later.\n");
      fflush (f);
      fclose (f);
      return;
    }
    setup_globals (plx);
    pl = cur_player;
    pl->host_ban = False;
    pl->user_ban = False;
    pl->sock_buf_p = pl->sock_buf_end = pl->sock_buffer;
    pl->sin_len = sin_len;
    pl->sin = sin;
    pl->fil_des = fd;
    pl->stream = f;

    strcpy (pl->hostname, host);

    if (host_banned || host_b2)
      pl->host_ban = True;

    if (user_banned || user_b2)
      pl->user_ban = True;

    /* Include this socket as a socket to listen to */
    if (fd >= width)
      width = fd + 1;
    FD_SET (fd, &sockets_fds);

#if USE_IDENT
    new_player (ident_id (fd, 0));
#else
    new_player (NULL);
#endif

  }
}

/************************************************************************
 * iDiRT Packet Handler 2.20						*
 * By Hastur, Changes by Illusion					*
 ************************************************************************/
void handle_packet (int fd) {

    static char *sock_msg[] = {
        "Connection Reset by Peer (ECONNRESET)",
        "No Route to Host (EHOSTUNREACH)",
        "Connection Timed Out (ETIMEDOUT)",
        "Network Unreachable (ENETUNREACH)",
        "Network Dropped Connection on Reset (ENETRESET)",
        "Network is Down (ENETDOWN)"
        "Empty Packets"
    };

    int plx = find_pl_index (fd);
    int x, y, g, msg;
    char *b, *p, *c, *k;

    x = y = g = msg = 0;

    setup_globals (plx);

    if (!cur_player->inp_handler || !cur_player->inp_handler->inp_handler) {
        mudlog ("SYSTEM: %s Lost Input Handler", pname (mynum));
        crapup ("\tInternal Error Has Occured: Lost Your Input Handler", CRAP_UNALIAS | CRAP_RETURN);
        return;
    }
    g = 0;

    if (cur_player->inp_buf_p >= cur_player->inp_buf_end) {
        errno = 0;
        bzero (cur_player->inp_buffer, sizeof (cur_player->inp_buffer));
        if ((x = read (fd, cur_player->inp_buffer, MAX_COM_LEN-32)) < 0) {
            if (errno == ECONNRESET ||/* Connection reset by peer */
	            errno == EHOSTUNREACH ||	/* No route to host */
	            errno == ETIMEDOUT ||	/* Connection timed out */
	            errno == ENETUNREACH ||	/* Network is unreachable */
	            errno == ENETRESET ||	/* Network dropped connect on reset */
	            errno == ENETDOWN ||	/* Network is down */
	            !x) {			/* Empty packets */
	            switch (errno) {
	                case ECONNRESET:
	                    msg = 0;
                        mudlog ("SYSTEM: %s Connection Reset", pname (mynum));
	                    break;
	                case EHOSTUNREACH:
	                    msg = 1;
                        mudlog ("SYSTEM: %s Host Unreachable", pname (mynum));
	                    break;
	                case ETIMEDOUT:
	                    msg = 2;
                        mudlog ("SYSTEM: %s Timed Out", pname (mynum));
	                    break;
	                case ENETUNREACH:
	                    msg = 3;
                        mudlog ("SYSTEM: %s Network Unreachable", pname (mynum));
	                    break;
	                case ENETRESET:
	                    msg = 4;
                        mudlog ("SYSTEM: %s Network Reset", pname (mynum));
	                    break;
	                case ENETDOWN:
	                    msg = 5;
                        mudlog ("SYSTEM: %s Network Down", pname (mynum));
	                    break;
	            }
	            if (!x)
	                msg = 6;

	            if (cur_player->iamon) {
	                send_msg (DEST_ALL, 0, max (pvis (mynum), LVL_MIN), LVL_WIZARD - 1,
		                    mynum, NOBODY, "%s has lost link to this realm.\n", pname (mynum));

	                send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD),
		                LVL_MAX, mynum, NOBODY, "&+W[&+CSocket (%d): &+C%s "
		                "&+whas lost link &+B(&+W%s&+B)&+W]\n",
		                cur_player->fil_des, pname (mynum), sock_msg[msg]);

	                mudlog ("SOCKET: %s has lost link (%s)", pname (mynum), sock_msg[msg]);
	            } else {
	                send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_SEESOCKET), LVL_WIZARD,
		                LVL_MAX, mynum, NOBODY, "&+W[&+CSocket (%d): &*Connecting "
		                "socket has lost link &+B(&+W%s&+B)&+W]\n",
		                cur_player->fil_des, pname (mynum), sock_msg[msg]);
	                mudlog ("SOCKET: Connecting socket has lost link (%s)", sock_msg[msg]);
	            }
	            crapup (NULL, CRAP_SAVE | CRAP_UNALIAS | CRAP_RETURN);
	            errno = 0;
	            return;
            }
            mudlog ("SOCKET: Error reading data from %s\n", pname (mynum));
            progerror (pname (mynum));
        }
        if (x == 0) {
            if (cur_player->iamon) {
	            send_msg (DEST_ALL, 0, max (pvis (mynum), LVL_MIN), LVL_WIZARD - 1,
		            mynum, NOBODY, "%s has lost link to this realm.\n",
		            pname (mynum));
	            send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD),
		            LVL_MAX, mynum, NOBODY, "&+W[&+CSocket (%d): &+C%s "
		            "&+whas lost (cut) connection&+W]\n", cur_player->fil_des,
		            pname (mynum), sock_msg[msg]);
	            mudlog ("SOCKET: %s has lost (cut) connection", pname (mynum));
            } else {
	            send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_SEESOCKET),
		            max (pvis (mynum), LVL_WIZARD), LVL_MAX, mynum, NOBODY,
		            "&+W[&+CSocket (%d): &*Connecting socket has lost (cut) "
		            "connection&+W]\n", cur_player->fil_des, pname (mynum));
	            mudlog ("SOCKET: Connecting socket has lost (cut) connection");
            }
            crapup (NULL, CRAP_SAVE | CRAP_UNALIAS | CRAP_RETURN);
            errno = 0;
            quit_player ();
            return;
        }

        if (cur_player->overwrite) {
            if (strchr (cur_player->inp_buffer, '\n') ||
	            strchr (cur_player->inp_buffer, '\r'))
	            cur_player->overwrite = 0;
            return;
        }
        inp_buf_c++;
        FD_SET (fd, &buffer_fds);
        b = cur_player->inp_buf_p = cur_player->inp_buffer;
        cur_player->inp_buf_end = b + x;
    } else {
        // b is start of input buffer
        // x is length of input buffer - 1
        b = cur_player->inp_buf_p;
        x = cur_player->inp_buf_end - b;
    }

    // check for some char as first char. if it's there, skip ahead 3 chars and do some other stuff.
    if (*b == '\377') {
        b += 3;
        x -= 3;
        if (x <= 0) {
            inp_buf_c--;
            FD_CLR (fd, &buffer_fds);
            cur_player->inp_buf_p = cur_player->inp_buf_end;
            cur_player->overwrite = 0;
            return;
        }
    }
    c = cur_player->sock_buf_p;

    for (y = 0; y < x && c < (cur_player->sock_buffer + (MAX_COM_LEN-32))  &&
        b[y] && b[y] != '\n' && b[y] != '\r'; ++y) {
        if (b[y] == '\010' || b[y] == '\177') {
            if (c > cur_player->sock_buffer)
	            c--;
        } else {
            *c++ = b[y];
        }
    }

    cur_player->sock_buf_p = c;

    if (y > (MAX_COM_LEN-33)) {
        *--c = '\0';
        y--;
        bprintf ("&+B(&*Maximum Input Exceeded&+B)\n");
        cur_player->overwrite = 1;
        b[MAX_COM_LEN - 33] = '\n';
    }

    if (y < x && c < (cur_player->sock_buffer + (MAX_COM_LEN-32))) {
        while (y < x && (b[y] == '\n' || b[y] == '\r' || !b[y]))
            y++;
        *c = 0;
        k = c;
        c = cur_player->sock_buffer;

        for (p = c; *p; p++)
            if (iscntrl (*p))
                *p = ' ';

        if (cur_player->logged)
            write_plr_log("> %s", c);
        if (cur_player->snooped > 0) {
            char tmp[MAX_COM_LEN + 2];
            strcpy (tmp, "\r&+GI&+W>&* ");
            strcat (tmp, c);
            strcat (tmp, "\n");
            print_buf (tmp, True);
        }
        g = 1;
    }

    cur_player->sock_buf_p = c;
    cur_player->inp_buf_p = b + y;

    if (y >= x) {
        FD_CLR (fd, &buffer_fds);
        inp_buf_c--;
    }

    if (g) {
        cur_player->inp_handler->inp_handler (c);
        cur_player->sock_buf_p = cur_player->sock_buffer;
        if (!ptstflg (mynum, PFL_IDLE)) {
            plast_cmd (real_mynum) = global_clock;
        }
        prlast_cmd (real_mynum) = global_clock;
    }
}

void
sig_handler (int sig)
{
  char msg[30];

  switch (sig) {
  case SIGTERM:
    mudlog ("SIGNAL: SIGTERM");
    sig_term_happened = True;
    return;
  case SIGALRM:
    sig_timer_happened = True;
    return;
  case SIGSEGV:
    mudlog ("SIGNAL: SIGSEGV[%d]", sig);
    sprintf (msg, "SIGSEGV[%d]", sig);
    break;
  case SIGBUS:
    mudlog ("SIGNAL: SIGBUS[%d]", sig);
    sprintf (msg, "SIGBUS[%d]", sig);
    break;
  case SIGINT:
    mudlog ("SIGNAL: SIGINT[%d]", sig);
    sprintf (msg, "SIGINT[%d]", sig);
    break;
  case SIGUSR1:
    mudlog ("SIGNAL: SIGUSR1[%d]", sig);
    sprintf (msg, "SIGUSR1[%d]", sig);
    break;
  case SIGUSR2:
    mudlog ("SIGNAL: SIGUSR2[%d]", sig);
    sprintf (msg, "SIGUSR2[%d]", sig);
    break;
  default:
    mudlog ("SIGNAL: %d", sig);
    sprintf (msg, "Unknown - %d", sig);
    break;
  }
  rm_pid_file ();

  sig_exit (msg, sig);
}

/************************************************************************
 * iDiRT Reboot Code (v1.35)						*
 * 1995 by Illusion							*
 * (Taken from Kender's Dirt 3.0 Reboot Code)				*
 ************************************************************************/

/* 1995, Illusion */
void
initialize_slot (int plx)
{
  players[plx].fil_des = -1;
  players[plx].stream = NULL;
  players[plx].no_echo = False;
  players[plx].isawiz = False;
  players[plx].ismonitored = False;
  players[plx].iamon = False;
  players[plx].is_conn = False;
  players[plx].in_pbfr = False;
  players[plx].aliased = False;
  players[plx].me_ivct = 0;
  players[plx].polymorphed = -1;
  players[plx].i_follow = -1;
  players[plx].snooptarget = -1;
  players[plx].pretend = -1;
  players[plx].snooped = 0;
  players[plx].asmortal = 0;
  players[plx].last_cmd = players[plx].logged_on = global_clock;
  strcpy (players[plx].prev_com, "quit");
  players[plx].quit_next = -2;
  players[plx].wd_it = "pit";
  players[plx].wd_them = players[plx].wd_him;
  *players[plx].wd_him = *players[plx].wd_her = '\0';
  players[plx].writer = NULL;
  players[plx].pconverse = -1;
  players[plx].overwrite = 0;

  /*** Patch for now, actually needs to be written out with the user data
   *** and read back into the MUD.
   ***/
  players[plx].duration = NULL;
}

static int
xmain_reboot (int fd)
{
  int pid, i, s, nplayers, nwiz_zones, plx, loc, vis, j;
  char p[10], newname[256], tmpstr[50];
  REBOOT_REC r_rec;
  RPLR_REC p_rec;
  FILE *fp;

  /* Allow mud to catch signals again */
  signal (SIGTERM, sig_handler);
  signal (SIGTSTP, SIG_DFL);
  signal (SIGCONT, SIG_DFL);
  signal (SIGTTOU, SIG_DFL);
  signal (SIGTTIN, SIG_DFL);
  signal (SIGSEGV, sig_handler);/* Segmentation fault */
  signal (SIGBUS, sig_handler);	/* Bus error */
  signal (SIGSYS, sig_handler);	/* Bad argument to system call */
  signal (SIGPIPE, SIG_IGN);	/* Broken pipe */
  signal (SIGUSR1, sig_handler);/* User defined signal */
  signal (SIGUSR2, sig_handler);/* User defined signal */

  pid = getpid ();		/* Get process ID */
  sprintf (p, "%d\n", pid);
  write (fd, p, strlen (p));
  close (fd);

  sprintf (newname, "reboot_file.%d", old_proc_num);

  if ((fp = fopen (newname, "rb")) == NULL) {
    mudlog ("REBOOT: Datafile %s could not be opened for reading", newname);
    return -1;
  }
  fread (&r_rec, sizeof (r_rec), 1, fp);

  if (!EQ (r_rec.version, _VERSION_))
    mudlog ("UPGRADE: Old Ver: %s; New Ver: %s", r_rec.version, _VERSION_);

  s = r_rec.main_socket;
  last_startup = r_rec.last_startup;
  numresets = r_rec.numresets;
  numreboots = r_rec.numreboots;
  numcrashes = r_rec.numcrashes;
  last_reset = r_rec.last_reset;
  the_climate->time = r_rec.climate_time;
  the_climate->day = r_rec.climate_day;
  the_climate->month = r_rec.climate_month;
  nwiz_zones = r_rec.nwiz_zones;
  nplayers = r_rec.nplayers;

  if (bootstrap () < 0) {	/* Initialize data structures */
    mudlog ("REBOOT: Bootstrap failed on reboot");
    return -1;
  }
  main_socket = s;
  FD_ZERO (&sockets_fds);
  FD_ZERO (&buffer_fds);
  FD_SET (s, &sockets_fds);

  /* Do full reset */
  for (i = 0; i < numzon; i++) {
    reset_zone (i, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  }

  for (i = 0; i < nwiz_zones; i++) {
    fread (tmpstr, 50, 1, fp);
    load_zone (tmpstr, NULL, NULL, NULL, NULL, NULL, NULL);
  }

  for (i = 0; i < nplayers; i++) {
    fread (&p_rec, sizeof (p_rec), 1, fp);
    plx = p_rec.plx;

    initialize_slot (plx);
    setup_globals (plx);

    cur_player->fil_des = p_rec.fil_des;
    setpname (mynum, p_rec.pname);
    cur_player->logged_on = p_rec.logged_on;
    cur_player->last_cmd = p_rec.last_cmd;
    cur_player->rlast_cmd = p_rec.rlast_cmd;
    loc = p_rec.ploc;
    vis = p_rec.pvis;
    strcpy (cur_player->hostname, p_rec.hostname);
    strcpy (cur_player->usrname, p_rec.usrname);

    for (j = 0; j < 10; ++j)
      cur_player->forget[j] = p_rec.forget[j];

    if (cur_player->fil_des >= width)
      width = cur_player->fil_des + 1;
    FD_SET (cur_player->fil_des, &sockets_fds);
    cur_player->sock_buf_p = cur_player->sock_buffer;
    cur_player->sock_buf_end = cur_player->sock_buffer;
    cur_player->stream = fdopen (cur_player->fil_des, "w");
    getuafinfo (pname (mynum));
    setpwpn (mynum, -1);
    setphelping (mynum, -1);
    setpfighting (mynum, -1);
    setpsitting (mynum, 0);
    cur_player->iamon = True;
    cur_player->is_conn = True;
    fetchprmpt (mynum);
    push_input_handler (get_command);
    get_command (NULL);
    if (exists (loc))
      setploc (mynum, loc);
    else if (exists (phome (mynum)))
      setploc (mynum, phome (mynum));
    else
      setploc (mynum, randperc () > 50 ? LOC_START_TEMPLE : LOC_START_CHURCH);
    setpvis (mynum, vis);
    setpconv (mynum, p_rec.pconv);
    sprintf (cur_player->awaymsg, "%s", p_rec.awaymsg);
    cur_player->Trace = p_rec.tracedata;
  }

  fclose (fp);

  unlink (newname);

  if (update) {
    update_world (old_proc_num);
    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "You open your eyes and see the %s beginning to fade "
	      "back\ninto reality. Suddenly, the world rearranges "
	      "itself back to the way it was.\n", MUD_NAME);

    send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+W[&+CUpdate Completed&+W]\n");
  } else {
    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "You open your eyes and see the %s beginning to fade "
	      "back\ninto reality. It all seems the same, yet somehow "
	      "different.\n", MUD_NAME);

    send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+W[&+CReboot Completed&+W]\n");
  }

  /* Update Climate */
  which_season ();
  the_climate->daytime = climate_day ();

  main_loop (s);
  mudlog ("REBOOT: Closing listening socket");
  close (s);
  return 0;
}

void
rebootcom (void)
{
  if (!ptstflg (mynum, PFL_REBOOT)) {
    bprintf ("You cannot reboot the MUD.\n");
    return;
  }
  run_reboot (False, False);
}

void
updatecom (void)
{
  if (!ptstflg (mynum, PFL_REBOOT)) {
    bprintf ("You cannot update the MUD.\n");
    return;
  }
  run_reboot (False, True);
}

void
run_reboot (Boolean crash, Boolean will_update)
{
  int i, plx, nwiz_zones, new, nplayers;
  char exec_path[256], new_path[256], filename[256];
  char port[10], max[10], pid[10], *intest;
  REBOOT_REC reboot_rec;
  RPLR_REC rplr_rec;
  FILE *fp;

  intest = strstr (progname, EXEC_LOC);

  if (intest == NULL) {
    sprintf (exec_path, EXEC_LOC "/%s", progname);
    sprintf (new_path, EXEC_LOC "/%s.new", progname);
  } else {
    sprintf (exec_path, "%s", progname);
    sprintf (new_path, "%s.new", progname);
  }

  if (access (new_path, F_OK) != -1)
    new = 1;
  else if (access (exec_path, F_OK) != -1)
    new = 0;
  else {
    bprintf ("Could not find an executable to reboot the MUD.\n");
    mudlog ("REBOOT: Error: Could not find executable");
    return;
  }

  signal (SIGALRM, SIG_IGN);

  if (crash) {
    send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+W[&+CEmergency %s To Prevent Crash&+W]\n",
	      will_update ? "Update" : "Reboot");

    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "A cry is heard from the heavens as the Gods lose control "
	      "over the\n%s. The world begins to shake violently as a "
	      "giant\nearthquake swallows you as the Gods reconstruct "
	      "the world..\n", MUD_NAME);

    mudlog ("REBOOT: Crash has occured, rebooting system");
  } else if (will_update) {
    send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+W[&+wUpdate %sby &+C\001p%s\003&+W]\n",
	      new ? "&+WNew &+w" : "", pname (mynum));

    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "The %s begin to move and shift violently around you. "
	      "You\nclose your eyes tight, hoping that this will all "
	      "end soon..\n", MUD_NAME);

    mudlog ("UPDATE: Update %sby %s", new ? "New " : "", pname (mynum));
  } else if (breset) {
    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "The %s begin to move and shift violently around you. "
	      "You\nclose your eyes tight, hoping that this will all "
	      "end soon..\n", MUD_NAME);

    mudlog ("REBOOT: Rebooted %sby BootReset", new ? "New " : "");
  } else {
    send_msg (DEST_ALL, 0, LVL_WIZARD, LVL_MAX, NOBODY, NOBODY,
	      "&+W[&+wReboot %sby &+C\001p%s\003&+W]\n",
	      new ? "&+WNew &+w" : "", pname (mynum));

    send_msg (DEST_ALL, 0, LVL_MIN, LVL_MAX, NOBODY, NOBODY,
	      "The %s begin to move and shift violently around you. "
	      "You\nclose your eyes tight, hoping that this will all "
	      "end soon..\n", MUD_NAME);

    mudlog ("REBOOT: Rebooted %sby %s", new ? "New " : "", pname (mynum));
  }

  if (crash)
    ++numcrashes;
  else
    ++numreboots;

  sprintf (filename, "reboot_file.%d", getpid ());

  for (i = num_const_zon, nwiz_zones = 0; i < numzon; ++i)
    if (!ztemporary (i))
      nwiz_zones++;

  for (i = 0, nplayers = 0; i < max_players; ++i)
    if (players[i].iamon)
      nplayers++;

  strcpy (reboot_rec.version, _VERSION_);
  reboot_rec.main_socket = main_socket;
  reboot_rec.last_startup = last_startup;
  reboot_rec.numresets = numresets;
  reboot_rec.numreboots = numreboots;
  reboot_rec.numcrashes = numcrashes;
  reboot_rec.last_reset = last_reset;
  reboot_rec.climate_time = the_climate->time;
  reboot_rec.climate_day = the_climate->day;
  reboot_rec.climate_month = the_climate->month;
  reboot_rec.nwiz_zones = nwiz_zones;
  reboot_rec.nplayers = nplayers;

  if ((fp = fopen (filename, "wb")) == NULL)
    mudlog ("ERROR: open() failed");

  fwrite (&reboot_rec, sizeof (reboot_rec), 1, fp);

  for (i = num_const_zon; i < numzon; ++i)
    if (!ztemporary (i))
      fwrite (zname (i), 50, 1, fp);

  for (plx = 0; plx < max_players; ++plx) {
    if (!players[plx].inp_handler)
      continue;

    setup_globals (plx);

    if (!players[plx].iamon) {
      bprintf ("%s is currently rebooting, please reconnect momentarily.\n",
	       MUD_NAME);
      bflush ();
      close (cur_player->fil_des);
      fclose (cur_player->stream);
      send_msg (DEST_ALL, MODE_PFLAG | MP (PFL_SEESOCKET), LVL_MIN, LVL_MAX,
		NOBODY, NOBODY, "&+W[&+CSocket (%d): &+wConnection from %s "
		"disconnected for reboot&+W]\n", players[plx].fil_des, players[plx].hostname);
    } else {
      saveme ();
      rplr_rec.plx = plx;
      rplr_rec.fil_des = cur_player->fil_des;
      strcpy (rplr_rec.pname, pname (mynum));
      rplr_rec.logged_on = cur_player->logged_on;
      rplr_rec.last_cmd = cur_player->last_cmd;
      rplr_rec.rlast_cmd = cur_player->rlast_cmd;
      rplr_rec.ploc = ploc (mynum);
      rplr_rec.pvis = pvis (mynum);
      rplr_rec.pconv = pconv (mynum);
      strcpy (rplr_rec.hostname, cur_player->hostname);
      strcpy (rplr_rec.usrname, cur_player->usrname);
      sprintf (rplr_rec.awaymsg, "%s", cur_player->awaymsg);

      for (i = 0; i < 10; ++i)
	rplr_rec.forget[i] = cur_player->forget[i];
      rplr_rec.tracedata = cur_player->Trace;
      fwrite (&rplr_rec, sizeof (rplr_rec), 1, fp);
    }
  }

  fclose (fp);

  for (i = 0; i < max_players; ++i) {
    if (players[i].iamon) {
      setup_globals (i);
      bflush ();
    }
  }

  if (will_update) {
    run_update ();
  }
  if (new) {
    unlink (exec_path);
    link (new_path, exec_path);
    unlink (new_path);
  }
  if (!access (PID_FILE, 0))
    unlink (PID_FILE);

  sprintf (port, "%d", mud_port);
  sprintf (max, "%d", max_players);
  sprintf (pid, "%d", getpid ());

  if (mud_port != PORT)
    if (max_players != MAX_PLAYERS)
      execl (exec_path, exec_path, "-p", port, "-n", max, will_update ? "-u" : "-r", pid, NULL);
    else
      execl (exec_path, exec_path, "-p", port, will_update ? "-u" : "-r", pid, NULL);
  else if (max_players != MAX_PLAYERS)
    execl (exec_path, exec_path, "-n", max, will_update ? "-u" : "-r", pid, NULL);
  else
    execl (exec_path, exec_path, will_update ? "-u" : "-r", pid, NULL);
}

/************************************************************************
 * iDiRT Hosts Code (v1.00)						*
 * 1995 by Illusion							*
 * The following code reads the iDiRT hosts file to get different	*
 * hostnames for certain IP addresses.					*
 ************************************************************************/

void
process_data (char data[300], char *ip, char *host)
{
  int i = 0, j = 0;

  while (!isspace (data[i])) {
    ip[i] = data[i];
    ++i;
  }

  ip[i] = '\0';

  for (j = 0; i < strlen (data); ++j) {
    ++i;
    host[j] = data[i];
  }

  host[j - 2] = '\0';
}

char *
read_hosts (char search[20])
{
  static char host[256];
  char data[300];
  char ip[20];

  FILE *fp;

  if ((fp = fopen (HOSTS, "rt")) == NULL)
    return NULL;

  while (!feof (fp)) {
    fgets (data, 300, fp);
    process_data (data, ip, host);
    if (strstr (search, ip)) {
      fclose (fp);
      return host;
    }
  }

  fclose (fp);
  return NULL;
}

void
usesocketcom (void)
{
  int fd, ct;

  bprintf ("&+wTotal Sockets Allocated: &+W%d\n", width);
  bprintf ("&+B------------------------------------------------------------------------------\n");
  for (fd = 0, ct = 1; fd < width; fd++, ct++) {
    bprintf ("FD %d: %s", fd, fd == main_socket ? "MainSock" :
	     FD_ISSET (fd, &sockets_fds) ? "InUse" : "Unused");
    if ((ct % 6 == 5) || (fd == width - 1))
      bprintf ("\n");
    else
      bprintf ("\t");
  }
  bprintf ("&+B------------------------------------------------------------------------------\n");
}
