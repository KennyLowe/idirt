
#include <stdlib.h>
#include "kernel.h"
#include "writer.h"
#include "bprintf.h"
#include "parse.h"
#include "mud.h"

void
start_writer (char *h, char *p, void *arg, int arglen,
	      void handler (void *x, void *arg, int arglen),
	      int flags, int max_lines)
{
  WrHead *w;

  if (handler == NULL || h == NULL || p == NULL || max_lines <= 0)
    return;
  w = NEW (WrHead, 1);
  w->previous = cur_player->writer;
  w->prevprompt = COPY (cur_player->cprompt);
  w->exit_handler = handler;
  w->old_inp_h = cur_player->inp_handler->inp_handler;
  w->prompt = COPY (p);
  w->arg = BCOPY (arg, arglen);
  w->arglen = arglen;
  w->flags = flags;
  w->max_lines = max_lines;
  w->num_lines = 0;
  w->first = NULL;
  w->last = NULL;
  cur_player->writer = w;
  strcpy (cur_player->cprompt, p);
  bprintf ("%s\n\n", h);
  write_handler (NULL);
}

void
write_handler (char *line)
{
  WrHead *w = (WrHead *) cur_player->writer;
  WrLine *l;
  int k;

  if ((k = (w->flags & WR_CMDCH)) == 0)
    k = '*';
  if (line == NULL) {
    replace_input_handler (write_handler);
  } else if (*line == k) {
    if (line[1] == k && line[2] == '\0') {
      /* Exit writer. */
      replace_input_handler (w->old_inp_h);	/* Back to old handler */
      FREE (w->prompt);		/* Don't need the prompt any more */
      if (w->first == NULL) {	/* Let it point to text instead */
	w->prompt = NULL;
      } else {
	w->prompt = w->first->s;
      }
      cur_player->writer = w->previous;
      strcpy (cur_player->cprompt, w->prevprompt);
      FREE (w->prevprompt);

/* Don't need max lines anymore, use it for unget buffer */

      w->max_lines = EOF;
      w->exit_handler (w, w->arg, w->arglen);
      bprintf ("\r%s", cur_player->cprompt);
      return;			/* Don't increment num_lines. */
    } else if ((w->flags & WR_CMD) != 0) {
      gamecom (line + 1, True);
    } else {
      bprintf ("You cannot execute any commands now.\n");
    }
  } else if (w->num_lines == w->max_lines) {
    bprintf ("You cannot type in more lines.\n");
  } else {
    l = NEW (WrLine, 1);
    l->s = COPY (line);
    l->next = NULL;
    if (w->first == NULL) {	/* First line. */
      w->first = w->last = l;
    } else {
      w->last->next = l;
      w->last = l;
    }
    ++w->num_lines;
  }
  bprintf ("\r%s", cur_player->cprompt);
}

int
wnum_lines (void *x)
{
  WrHead *w = x;

  return w == 0 ? 0 : w->num_lines;
}

int
wnum_chars (void *x)
{
  WrHead *w = x;
  WrLine *n;
  int k = 0;

  if (w == 0)
    return 0;
  for (n = w->first; n != 0; n = n->next)
    k += strlen (n->s);
  return k;
}

int
wgetc (void *x)
{
  WrHead *w = x;
  WrLine *l;
  WrLine *n;
  int k;

  if (w == NULL)
    return EOF;
  if ((k = w->max_lines) != EOF) {
    w->max_lines = EOF;
    return k;			/* return unget character */
  }
  if (w->prompt == NULL) {
    FREE (w->arg);
    FREE (w);
    return EOF;
  }
  if ((k = *w->prompt++) == '\0') {
    /* End of this line, kill it and return '\n' */
    l = w->first;
    if ((n = w->first = l->next) == NULL) {
      w->prompt = NULL;
    } else {
      w->prompt = n->s;
    }
    w->num_lines--;
    FREE (l->s);
    FREE (l);
    return '\n';
  } else {
    return k;
  }
}

int
wungetc (int c, void *x)
{
  WrHead *w;
  WrLine *l;

  if ((w = x) == NULL)
    return EOF;
  if (c == EOF) {
    while ((l = w->first) != NULL) {
      w->first = l->next;
      FREE (l->s);
      FREE (l);
    }
    w->prompt = NULL;
    w->num_lines = 0;
    return EOF;
  }
  if ((l = w->first) != NULL && w->prompt > l->s) {
    *--(w->prompt) = c;
    return c;
  }
  /* Try to put it into max_lines */
  if (w->max_lines != EOF)
    return EOF;			/* Unable to push character back */
  w->max_lines = c;
  return c;
}

char *
wgets (char *buf, int buflen, void *w)
{
  int i, k;
  char *s = buf;

  for (i = 1; i < buflen; i++) {
    if ((k = wgetc (w)) == EOF) {
      *s = 0;
      return NULL;
    }
    if ((*s++ = k) == '\n')
      break;
  }
  *s = 0;
  return buf;
}

void
terminate_writer (void *w)
{

  if (w != NULL) {
    (void) wungetc (EOF, w);
    (void) wgetc (w);
  }
}

void
terminate_all_writers (int plx)
{
  WrHead *w;
  WrHead *x;

  if (plx >= 0 && plx < max_players) {
    w = players[plx].writer;
    while ((x = w) != NULL) {
      w = w->previous;
      terminate_writer (x);
    }
  }
}
