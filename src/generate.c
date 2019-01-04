








/*
 * **  Utility program to build the data files.
 * **  Alf, June 1990
 */

#include "kernel.h"
#include <stdlib.h>

void make_data (int argc, char **argv);

void
crapup (char *p)
{
  perror (p);
  exit (1);
}

/*
 * **  Open file for read/write or die trying
 */
static FILE *
Do_fopen (char *name, char *mode)
{
  FILE *file;

  if (*name == '-' && !name[1])
    return *mode == 'w' ? stdout : stdin;
  if (!(file = fopen (name, mode))) {
    perror (name);
    (void) fprintf (stderr, "Unable to open %s for %s\n",
		    name, *mode == 'w' ? "write" : "read");
    exit (1);
  }
  return file;
}

typedef struct _XTABLE {	/* move this inside make verbs ? */
  struct _XTABLE *next;
  int code;
  char name[30];
}

XTABLE;

/*
 * **  Make header file verbs.h
 */
static void
make_verbs (int argc, char *argv[])
{
  FILE *In, *Out, *H;
  char *p, *q;
  int vcode, num_v;
  XTABLE *first, *last, *v, *w;
  char buff[128], verb[30];

  if (argc < 3) {
    (void) fprintf (stderr, "Usage error: See Makefile\n");
    exit (1);
  }
  In = Do_fopen (argv[1], "r");
  Out = Do_fopen (argv[2], "w");
  H = Do_fopen (argv[3], "w");

  first = last = NULL;
  vcode = num_v = 0;
  while (fgets (buff, sizeof buff, In)) {
    for (p = buff, q = verb; isalpha (*p);)
      *q++ = *p++;
    if (q == verb)
      continue;
    *q = 0;
    lowercase (verb);

    v = (XTABLE *) xmalloc (1, sizeof (XTABLE));
    strcpy (v->name, verb);
    ++num_v;

    if (*p != '=') {
      v->code = ++vcode;
    } else {
      for (q = verb, ++p; isalpha (*p);)
	*q++ = *p++;
      *q = 0;
      lowercase (verb);

      for (w = first; w != NULL && strcmp (w->name, verb) != 0; w = w->next) ;
      if (w == NULL) {
	v->code = ++vcode;
      } else {
	v->code = w->code;
      }
    }

    v->next = NULL;
    if (first == NULL) {
      first = last = v;
    } else {
      last = (last->next = v);
    }
  }
  fclose (In);

  /* Print out header */
  (void) fprintf (H, "\
/*\n\
**\tVerb file header generated from %s\n\
**\tDON'T MAKE CHANGES HERE -- THEY WILL GO AWAY!\n\
*/\n\n\
#ifndef _VERBS_H\n\
#define _VERBS_H\n\n", argv[1]);
  fprintf (Out, "%d\n", num_v);

  /* Read thru verb file, creating #define's for each verb */

  for (w = first; w != NULL; w = w->next) {
    strcpy (verb, w->name);
    (void) fprintf (Out, "%s %d\n", verb, w->code);

    uppercase (verb);
    (void) fprintf (H, "#define\tVERB_%s\t%d\n", verb, w->code);
  }
  fclose (Out);
  fprintf (H, "\n#endif\n");
  fclose (H);
  exit (0);
}

int
main (int argc, char *argv[])
{
  argc--;
  argv++;

  if (argc) {
    if (EQ (*argv, "verbs")) {
      make_verbs (argc, argv);
    } else { 
      if (EQ (*argv, "data")) {
        make_data (argc, argv);
      } else {
        (void) fprintf (stderr, "Usage error: See Makefile\n");
        return 1;
      }
    }
  }
  return 0;
}
