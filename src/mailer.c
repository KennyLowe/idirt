
/************************************************************************
 * iDiRT Mailer	Version 2.x		     Copyright 1996 by Illusion	*
 *									*
 * The iDiRT Mailer has been coded solely by Illusion. If you wish to	*
 * use this mailer for your MUD, you must get Illusion's permission	*
 * before using it with your MUD.					*
 ************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "kernel.h"
#include "sendsys.h"
#include "bprintf.h"
#include "mobile.h"
#include "mailer.h"
#include "parse.h"
#include "log.h"
#include "mud.h"
#include "uaf.h"

void
mailcom (void)
{
  PERSONA p;

  if (brkword () != -1) {
    if (getuaf (wordbuf, &p)) {
      cur_player->inmailer = False;
      initialize_mailer ();
      sprintf (cur_player->Mailer.sendto, "%s", p.p_name);
      get_subject (NULL);
      return;
    } else {
      bprintf ("Player Not Found In Database or Online.\n");
      return;
    }
  } else {
    initialize_mailer ();
    bprintf ("&+CiDiRT Mailer Version %s\n", MAILVERSION);
    bprintf ("&+B-------------------------\n");
    check_mail (pname (mynum));
    open_mailbox (pname (mynum), READ);
    read_msgidx ();
    mail_menu (NULL);
    return;
  }
  return;
}

void
initialize_mailer (void)
{
  cur_player->Mailer.old_handler = cur_player->inp_handler->inp_handler;
  cur_player->Mailer.lastmsg = 0;
  strcpy (cur_player->Mailer.old_prompt, cur_player->prompt);
}

void
check_mail (char *name)
{
  FILE *fp;
  char fn[100], buffer[1024];
  int count, unread;

  count = unread = 0;
  sprintf (fn, "%s/%s", MAIL_DIR, name);

  if (access (fn, 0)) {
    bprintf ("&+CNo messages found in mailbox.\n");
    return;
  }
  if ((fp = fopen (fn, "r")) == NULL) {
    bprintf ("&+RCannot open mailbox.\n");
    mudlog ("ERROR: Cannot open mailbox (%s)", fn);
    return;
  }
  while (!feof (fp)) {
    fgets (buffer, sizeof (buffer), fp);
    if (feof (fp))
      break;
    count++;
    if (*buffer == 'T')
      unread++;
    while (*buffer != EOM_MARKER)
      fgets (buffer, sizeof (buffer), fp);
  }
  fclose (fp);

  if (count)
    bprintf ("&+W%d &+Cmessage%s found in mailbox.\n", count,
	     count == 1 ? "" : "s");
  else
    bprintf ("&+CNo messages found in mailbox.\n");

  if (unread)
    bprintf ("&+W%d &+Cmessage%s %s still unread.\n", unread,
	     unread == 1 ? "" : "s", unread == 1 ? "is" : "are");

  if (!cur_player->inmailer)
    cur_player->Mailer.lastmsg = count;
  return;
}

Boolean
open_mailbox (char *name, short mode)
{
  if (mode == READ) {
    sprintf (cur_player->Mailer.mailboxname, "%s/%s", MAIL_DIR, name);

    if (access (cur_player->Mailer.mailboxname, 0)) {
      cur_player->Mailer.mailbox = NULL;
      return False;
    }
    if ((cur_player->Mailer.mailbox = fopen (cur_player->Mailer.mailboxname, "r+")) == NULL) {
      bprintf ("Cannot open the mailbox.\n");
      mudlog ("ERROR: Cannot open mailbox (%s)", cur_player->Mailer.mailboxname);
      return False;
    }
  } else {
    sprintf (cur_player->Mailer.outputname, "%s/%s.to.%s", MAIL_DIR,
	     pname (mynum), name);

    if ((cur_player->Mailer.output = fopen (cur_player->Mailer.outputname, "w")) == NULL) {
      bprintf ("Cannot create temporary file.\n");
      mudlog ("ERROR: Cannot create file (%s)", cur_player->Mailer.outputname);
      return False;
    }
  }
  return True;
}

void
mail_headers (void)
{
  Boolean new;
  int loop;

  if (cur_player->Mailer.lastmsg <= 0) {
    bprintf ("Mailbox is empty.\n");
    return;
  }
  rewind (cur_player->Mailer.mailbox);

  bprintf ("&+W    Num  From            Date Sent     Subject\n");
  bprintf ("&+c-------------------------------------------------------------------------------\n");

  for (loop = 0; loop < cur_player->Mailer.lastmsg; loop++) {
    new = False;
    fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);

    if (feof (cur_player->Mailer.mailbox))
      return;

    if (cur_player->Mailer.buffer[0] == 'T')
      new = True;

    fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
    sprintf (msg_from (mynum), "%.15s", cur_player->Mailer.buffer);
    msg_from (mynum)[strlen (msg_from (mynum)) - 1] = '\0';

    fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
    sprintf (msg_subject (mynum), "%.40s", cur_player->Mailer.buffer);
    msg_subject (mynum)[strlen (msg_subject (mynum)) - 1] = '\0';

    fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
    msg_date (mynum) = atol (cur_player->Mailer.buffer);

    while (cur_player->Mailer.buffer[0] != EOM_MARKER)
      fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);

    bprintf ("%s%s &+B[&*%3d&+B] &+G%-15.15s &+w%-12.12s  &+C%-38.38s\n",
	     new ? "&+CN" : " ", msg_idx_delete (mynum, loop) ? "&+RD" : " ",
	     loop + 1, msg_from (mynum), make_mailtime (msg_date (mynum)),
	     msg_subject (mynum));

  }
  bprintf ("&+c-------------------------------------------------------------------------------\n");
}

void
mail_menu (char *Choice)
{
  if (Choice == NULL) {
    cur_player->inmailer = True;
    bprintf ("%s", MENU_PROMPT);
    strcpy (cur_player->prompt, MENU_PROMPT);
    strcpy (cur_player->cprompt, build_prompt (mynum));
    replace_input_handler (mail_menu);
    return;
  } else {
    *Choice = toupper (*Choice);
    switch (*Choice) {
    case 'D':
      if (Choice[1] != ' ' || !isdigit (Choice[2])) {
	bprintf ("Usage: D <number>.   Type '?' for further information.\n");
      } else {
	if (atoi (&Choice[2]) > cur_player->Mailer.lastmsg)
	  bprintf ("&+RNumber Out Of Range\n");
	else
	  msg_idx_delete (mynum, atoi (&Choice[2]) - 1) = True;
      }
      break;
    case 'U':
      if (Choice[1] != ' ' || !isdigit (Choice[2])) {
	bprintf ("Usage: U <number>.   Type '?' for further information.\n");
      } else {
	if (atoi (&Choice[2]) > cur_player->Mailer.lastmsg)
	  bprintf ("&+RNumber Out Of Range\n");
	else
	  msg_idx_delete (mynum, atoi (&Choice[2]) - 1) = False;
      }
      break;
    case 'H':
      mail_headers ();
      break;
    case 'M':
      if (strlen (Choice) < 3) {
	bprintf ("Usage: M <player>.  Enter '?' for more information.\n");
	break;
      } else {
	send_mail (&Choice[2]);
	return;
      }
    case 'F':
      if (strlen (Choice) < 5) {
	bprintf ("Usage: F <num> <plr>.  Enter '?' for more information.\n");
	break;
      } else {
	send_forward (&Choice[2]);
	return;
      }
    case 'R':
      if (strlen (Choice) < 3) {
	bprintf ("Usage: R <number>.   Type '?' for further information.\n");
	break;
      } else {
	send_reply (atoi (&Choice[2]));
	return;
      }
    case '?':
      bprintf ("\001f%s/%s\003", HELP_DIR, MAIL_HELP);
      break;
    case 'V':
      bprintf ("&+B----------------------------------\n");
      bprintf ("&*iDiRT Mailer Version %s\n", MAILVERSION);
      bprintf ("&*1996 by Illusion\n");
      bprintf ("&+B----------------------------------\n");
      break;
    case '*':
      if (EMPTY (&Choice[1]))
	bprintf ("Usage: *<command>.   Type '?' for further information.\n");
      else
	gamecom (&Choice[1], False);
      break;
    case 'Q':
      quit_mailer ();
      return;
    case '\0':
    case '\n':
      break;
    default:
      if (atoi (Choice) < 1 || atoi (Choice) > cur_player->Mailer.lastmsg)
	bprintf ("Invalid Command. Enter '?' For Help\n");
      else
	read_message (atoi (Choice));
    }
  }
  bprintf ("%s", MENU_PROMPT);
  return;
}

char *
make_mailtime (time_t tm_t)
{
  static char timestr[25];
  char *tm;
  int pos;

  tm = ctime (&tm_t);
  tm[16] = '\0';

  for (pos = 0; pos < 15; pos++)
    timestr[pos] = tm[pos + 4];
  timestr[pos] = '\0';

  return timestr;
}

void
read_msgidx (void)
{
  int loop;

  if (cur_player->Mailer.lastmsg <= 0)
    return;

  cur_player->Mailer.msgidx = NEW (MSGIDX, cur_player->Mailer.lastmsg);
  rewind (cur_player->Mailer.mailbox);
  for (loop = 0; loop < cur_player->Mailer.lastmsg; loop++) {
    cur_player->Mailer.buffer[0] = '\0';
    msg_idx_offset (mynum, loop) = ftell (cur_player->Mailer.mailbox);
    msg_idx_delete (mynum, loop) = False;
    while (cur_player->Mailer.buffer[0] != EOM_MARKER)
      fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  }
}

void
quit_mailer (void)
{
  int loop;

  sprintf (cur_player->Mailer.outputname, "%s/%s.tmp", MAIL_DIR, pname (mynum));
  if ((cur_player->Mailer.output = fopen (cur_player->Mailer.outputname, "w")) == NULL) {
    fclose (cur_player->Mailer.mailbox);
    bprintf ("Error making temporary file.\n");
    mudlog ("DEBUG: Error creating file (%s)", cur_player->Mailer.outputname);
  } else {
    rewind (cur_player->Mailer.mailbox);
    for (loop = 0; loop < cur_player->Mailer.lastmsg; loop++) {
      cur_player->Mailer.buffer[0] = '\0';
      while (cur_player->Mailer.buffer[0] != EOM_MARKER) {
	fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
	if (!msg_idx_delete (mynum, loop))
	  fprintf (cur_player->Mailer.output, "%s", cur_player->Mailer.buffer);
      }
    }
    fclose (cur_player->Mailer.mailbox);
    fclose (cur_player->Mailer.output);
    rename (cur_player->Mailer.outputname, cur_player->Mailer.mailboxname);
  }

  cur_player->inmailer = False;
  FREE (cur_player->Mailer.msgidx);
  bprintf ("&+WExiting &+Ci&+WDiRT Mailer\n");
  strcpy (cur_player->prompt, cur_player->Mailer.old_prompt);
  strcpy (cur_player->cprompt, build_prompt (mynum));
  bprintf ("%s", cur_player->cprompt);
  replace_input_handler (cur_player->Mailer.old_handler);
  return;
}

void
read_message (int number)
{
  if (msg_idx_delete (mynum, number - 1))
    bprintf ("&+C[&+WNote: &*This message is marked for deletion&+C]\n");

  fseek (cur_player->Mailer.mailbox, msg_idx_offset (mynum, number - 1), SEEK_SET);
  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);

  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  sprintf (msg_from (mynum), "%s", cur_player->Mailer.buffer);
  msg_from (mynum)[strlen (msg_from (mynum)) - 1] = '\0';

  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  sprintf (msg_subject (mynum), "%s", cur_player->Mailer.buffer);
  msg_subject (mynum)[strlen (msg_subject (mynum)) - 1] = '\0';

  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  msg_date (mynum) = atol (cur_player->Mailer.buffer);

  bprintf ("&+CFrom: &*%s\n", msg_from (mynum));
  bprintf ("&+CSubject: &*%s\n", msg_subject (mynum));
  bprintf ("&+CDate Sent: &*%s\n", ctime (&msg_date (mynum)));

  while (cur_player->Mailer.buffer[0] != EOM_MARKER) {
    fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
    if (cur_player->Mailer.buffer[0] != EOM_MARKER)
      bprintf ("%s", cur_player->Mailer.buffer);
  }
  fseek (cur_player->Mailer.mailbox, msg_idx_offset (mynum, number - 1), SEEK_SET);
  fprintf (cur_player->Mailer.mailbox, "F");
  fflush (cur_player->Mailer.mailbox);
  rewind (cur_player->Mailer.mailbox);
  return;
}

void
send_mail (char *send_to)
{
  PERSONA p;

  if (getuaf (send_to, &p)) {
    sprintf (cur_player->Mailer.sendto, "%s", p.p_name);
    get_subject (NULL);
    return;
  } else {
    bprintf ("Player Not Found In Database or Online.\n");
    mail_menu (NULL);
    return;
  }
  return;
}

void
get_subject (char *subject)
{
  if (subject == NULL) {
    bprintf ("%s", SUBJECT_PROMPT);
    strcpy (cur_player->prompt, SUBJECT_PROMPT);
    strcpy (cur_player->cprompt, build_prompt (mynum));
    replace_input_handler (get_subject);
    return;
  } else if (subject[0] == '\n') {
    bprintf ("Message Aborted.\n");
    if (cur_player->inmailer) {
      mail_menu (NULL);
      return;
    } else {
      strcpy (cur_player->prompt, cur_player->Mailer.old_prompt);
      strcpy (cur_player->cprompt, build_prompt (mynum));
      bprintf ("%s", cur_player->cprompt);
      replace_input_handler (cur_player->Mailer.old_handler);
      return;
    }
    return;
  } else {
    sprintf (msg_subject (mynum), "%s", subject);
    open_mailbox (cur_player->Mailer.sendto, WRITE);
    fprintf (cur_player->Mailer.output, "T\n%s\n%s\n%ld\n",
	     pname (mynum), msg_subject (mynum), (long) time (0));
    fflush (cur_player->Mailer.output);
    mail_input (NULL);
    return;
  }
  return;
}

void
mail_input (char *input)
{
  char filename[100];
  int plr;

  if (input == NULL) {
    bprintf ("%s", INPUT_INFO);
    bprintf ("%s", INPUT_PROMPT);
    strcpy (cur_player->prompt, INPUT_PROMPT);
    strcpy (cur_player->cprompt, build_prompt (mynum));
    replace_input_handler (mail_input);
    return;
  } else if (!strcasecmp (input, "=s") || !strcasecmp (input, "**")) {
    fprintf (cur_player->Mailer.output, "\n%c\n", EOM_MARKER);
    fclose (cur_player->Mailer.output);
    sprintf (filename, "%s/%s", MAIL_DIR, cur_player->Mailer.sendto);
    if (!append_file (filename, cur_player->Mailer.outputname)) {
      bprintf ("Error has occured while writing to mailbox.\n");
    } else {
      bprintf ("%s\n", SENT_MESSAGE);
      if ((plr = fpbns (cur_player->Mailer.sendto)) >= 0) {
	sendf (plr, "%s\n", NEW_MAIL_MSG);
	if (players[plr].inmailer)
	  reindex_mail (plr);
      }
    }
    unlink (cur_player->Mailer.outputname);
    if (cur_player->inmailer) {
      mail_menu (NULL);
      return;
    } else {
      strcpy (cur_player->prompt, cur_player->Mailer.old_prompt);
      strcpy (cur_player->cprompt, build_prompt (mynum));
      bprintf ("%s", cur_player->cprompt);
      replace_input_handler (cur_player->Mailer.old_handler);
      return;
    }
    return;
  } else if (!strcasecmp (input, "=a") || !strcasecmp (input, "*abort")) {
    bprintf ("&+RMessage Aborted.\n");
    fclose (cur_player->Mailer.output);
    unlink (cur_player->Mailer.outputname);
    if (cur_player->inmailer) {
      mail_menu (NULL);
      return;
    } else {
      strcpy (cur_player->prompt, cur_player->Mailer.old_prompt);
      strcpy (cur_player->cprompt, build_prompt (mynum));
      bprintf ("%s", cur_player->cprompt);
      replace_input_handler (cur_player->Mailer.old_handler);
      return;
    }
    return;
  } else if (input[0] == '!') {
    gamecom (&input[1], False);
    bprintf (INPUT_PROMPT);
    return;
  } else {
    fprintf (cur_player->Mailer.output, "%s\n", input);
    fflush (cur_player->Mailer.output);
    bprintf (INPUT_PROMPT);
    return;
  }
  return;
}

/* Appends file src to file dest.
 */
Boolean
append_file (char *dest, char *src)
{
  FILE *fin;
  FILE *fout;
  char x[1024];

  if ((fin = fopen (src, "r")) == NULL) {
    mudlog ("ERROR: Cannot open source file: %s (append_file)", src);
    return False;
  }
  if ((fout = fopen (dest, "a+")) == NULL) {
    mudlog ("ERROR: Cannot open destination file: %s (append_file)", dest);
    fclose (fin);
    return False;
  }
  while (!feof (fin)) {
    fgets (x, 1024, fin);
    fprintf (fout, "%s", x);
    x[0] = '\0';
  }
  fclose (fin);
  fclose (fout);
  return True;
}

void
send_forward (char *param)
{
  PERSONA p;
  char buffer[30], filename[100];
  int plr, num, pos = 0;

  while (param[pos] != ' ') {
    if (param[pos] == '\0' || pos >= strlen (buffer)) {
      bprintf ("Usage: F <num> <plr>.  Enter '?' for more information.\n");
      mail_menu (NULL);
      return;
    } else {
      buffer[pos] = param[pos++];
    }
  }

  buffer[pos] = '\0';
  num = atoi (buffer);

  if (num < 0 || num > cur_player->Mailer.lastmsg) {
    bprintf ("That message number is out of range.\n");
    mail_menu (NULL);
    return;
  }
  if (getuaf (&param[pos + 1], &p)) {
    sprintf (cur_player->Mailer.sendto, "%s", p.p_name);
  } else {
    bprintf ("Player Not Found In Database or Online.\n");
    mail_menu (NULL);
    return;
  }

  fseek (cur_player->Mailer.mailbox, msg_idx_offset (mynum, num - 1), SEEK_SET);
  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);

  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  sprintf (msg_from (mynum), "%s", cur_player->Mailer.buffer);
  msg_from (mynum)[strlen (msg_from (mynum)) - 1] = '\0';

  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  sprintf (msg_subject (mynum), "%s", cur_player->Mailer.buffer);
  msg_subject (mynum)[strlen (msg_subject (mynum)) - 1] = '\0';

  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  msg_date (mynum) = atol (cur_player->Mailer.buffer);

  open_mailbox (&param[pos], WRITE);

  fprintf (cur_player->Mailer.output, "T\n%s\n%s", pname (mynum),
	   msg_subject (mynum));
  if (strstr (msg_subject (mynum), "Fwd") == NULL)
    fprintf (cur_player->Mailer.output, " (Fwd)");
  fprintf (cur_player->Mailer.output, "\n%ld\n", (long) time (0));

  fprintf (cur_player->Mailer.output, "%s\n", FWD_NOTE);
  fprintf (cur_player->Mailer.output, "&+CFrom: &*%s\n", msg_from (mynum));
  fprintf (cur_player->Mailer.output, "&+CSubject: &*%s\n", msg_subject (mynum));
  fprintf (cur_player->Mailer.output, "&+CDate Sent: &*%s\n", ctime (&msg_date (mynum)));

  while (cur_player->Mailer.buffer[0] != EOM_MARKER) {
    fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
    if (cur_player->Mailer.buffer[0] != EOM_MARKER)
      fprintf (cur_player->Mailer.output, "%s", cur_player->Mailer.buffer);
  }
  fprintf (cur_player->Mailer.output, "\n%c\n", EOM_MARKER);
  fclose (cur_player->Mailer.output);
  sprintf (filename, "%s/%s", MAIL_DIR, cur_player->Mailer.sendto);
  if (!append_file (filename, cur_player->Mailer.outputname)) {
    bprintf ("Error has occured while writing to mailbox.\n");
  } else {
    bprintf ("%s\n", SENT_FORWARD);
    if ((plr = fpbns (cur_player->Mailer.sendto)) >= 0) {
      sendf (plr, "%s\n", NEW_MAIL_MSG);
      if (players[plr].inmailer)
	reindex_mail (plr);
    }
  }
  unlink (cur_player->Mailer.outputname);
  mail_menu (NULL);
  return;
}

void
send_reply (int num)
{

  if (num < 0 || num > cur_player->Mailer.lastmsg) {
    bprintf ("That message number is out of range.\n");
    mail_menu (NULL);
    return;
  }
  fseek (cur_player->Mailer.mailbox, msg_idx_offset (mynum, num - 1), SEEK_SET);
  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);

  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  sprintf (msg_from (mynum), "%s", cur_player->Mailer.buffer);
  msg_from (mynum)[strlen (msg_from (mynum)) - 1] = '\0';

  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  sprintf (msg_subject (mynum), "%s", cur_player->Mailer.buffer);
  msg_subject (mynum)[strlen (msg_subject (mynum)) - 1] = '\0';

  fgets (cur_player->Mailer.buffer, 256, cur_player->Mailer.mailbox);
  msg_date (mynum) = atol (cur_player->Mailer.buffer);

  sprintf (cur_player->Mailer.sendto, "%s", msg_from (mynum));
  open_mailbox (cur_player->Mailer.sendto, WRITE);

  fprintf (cur_player->Mailer.output, "T\n%s\n%s", pname (mynum),
	   msg_subject (mynum));
  if (strstr (msg_subject (mynum), "Re") == NULL)
    fprintf (cur_player->Mailer.output, " (Re)");
  fprintf (cur_player->Mailer.output, "\n%ld\n", (long) time (0));

  mail_input (NULL);
  return;
}

void
reindex_mail (int plr)
{
  MSGIDX *msgidxtmp;
  int me = real_mynum, i;

  setup_globals (plr);
  msgidxtmp = NEW (MSGIDX, cur_player->Mailer.lastmsg);
  for (i = 0; i < cur_player->Mailer.lastmsg; i++) {
    msgidxtmp[i].offset = msg_idx_offset (mynum, i);
    msgidxtmp[i].delete = msg_idx_delete (mynum, i);
  }
  FREE (cur_player->Mailer.msgidx);
  fclose (cur_player->Mailer.mailbox);
  open_mailbox (pname (mynum), READ);
  rewind (cur_player->Mailer.mailbox);
  cur_player->Mailer.lastmsg = 0;
  while (!feof (cur_player->Mailer.mailbox)) {
    if (fgetc (cur_player->Mailer.mailbox) == EOM_MARKER)
      ++cur_player->Mailer.lastmsg;
  }
  rewind (cur_player->Mailer.mailbox);
  read_msgidx ();
  setup_globals (me);
}
