#include <stdlib.h>
#include "../kernel.h"
#include "../macros.h"
#include "../pflags.h"
#include "../pflagnames.h"
#include "../levelnames.h"

int num_levels[] =
{1, LVL_WIZARD, LVL_DORQ, LVL_EMERITI, LVL_FULLWIZ,
 LVL_ARCHWIZARD, LVL_ADVISOR, LVL_AVATAR, LVL_GOD,
 LVL_MASTER};

char *s;
int l;
PFLAGS p;
PFLAGS m;
char b[256];
char g[256];
char n[256];

static Boolean or_ok ();

static Boolean
test_bit (PFLAGS * f, int n)
{
  if (n >= 64)
    return xtstbit (f->u, (n - 64));
  if (n >= 32)
    return xtstbit (f->h, (n - 32));
  return xtstbit (f->l, n);
}

static int
lookup (char *e, char **t)
{
  register int l = strlen (e);
  register int x = 0;

  for (; *t != TABLE_END; ++t, ++x) {
    if (*t == NULL)
      continue;
    if (strncasecmp (e, *t, l) == 0)
      return x;
  }
  return -1;
}

static Boolean
ok ()
{
  Boolean j;
  int k;
  char *t;

  while (*s == ' ' || *s == '\t')
    ++s;
  switch (*s++) {
  case '(':
    j = or_ok ();
    if (*s != ')') {
      fprintf (stderr, "\nMissing ')' at %s.\n", s);
      exit (1);
    }
    ++s;
    return j;
  case '!':
    return !ok ();
  case 'U':
    return (l >= LVL_MASTER);
  case 'G':
    return (l >= LVL_GOD);
  case 'D':
    return (l >= LVL_AVATAR);
  case 'V':
    return (l >= LVL_ADVISOR);
  case 'A':
    return (l >= LVL_ARCHWIZARD);
  case 'R':
    return (l >= LVL_PROPHET);
  case 'W':
    return (l >= LVL_FULLWIZ);
  case 'X':
    return (l >= LVL_WIZARD);
  case 'L':
    if (*s == '-' || isdigit (*s)) {
      k = strtol (s, &t, 10);
      s = t;
    } else {
      for (t = n; isalpha (*s);)
	*t++ = *s++;
      *t = 0;
      if ((k = lookup (n, WizLevels)) >= 0)
	k = num_levels[k];
      else if ((k = lookup (n, MLevels)) < 0) {
	fprintf (stderr, "\nUnknown name of level: %s.\n", n);
	exit (1);
      }
    }
    return (l >= k);
  case 'P':
    for (t = n; isalpha (*s);)
      *t++ = *s++;
    *t = 0;
    if ((k = lookup (n, Pflags)) < 0) {
      fprintf (stderr, "\nUnknown name of Pflags: %s.\n", n);
      exit (1);
    }
    return test_bit (&p, k);
  case 'M':
    for (t = n; isalpha (*s);)
      *t++ = *s++;
    *t = 0;
    if ((k = lookup (n, Pflags)) < 0) {
      fprintf (stderr, "\nUnknown name of Pflags: %s.\n", n);
      exit (1);
    }
    return test_bit (&m, k);
  case 'Z':
    return (p.l == 0 && p.h == 0 && p.u == 0);
  case 'Y':
    return (m.h == 0 && m.h == 0 && m.u == 0);
  default:
    if (!isalpha (*s))
      return True;
    fprintf (stderr, "\nIllegal code, %c at %s\n", s[-1], s - 1);
    exit (1);
  }
}

static Boolean
and_ok ()
{
  Boolean j = True;

  do {
    if (!ok ())
      j = False;
    while (*s == ' ' || *s == '\t')
      ++s;
    if (*s == '&')
      while (*++s == ' ' || *s == '\t') ;
  }
  while (isalpha (*s) || *s == '!' || *s == '(');
  return j;
}

static Boolean
or_ok ()
{
  Boolean j = False;

  while (True) {
    if (and_ok ())
      j = True;
    if (*s != '|')
      return j;
    ++s;
  }
}

static Boolean
all_ok (char *t)
{
  Boolean j;

  s = t;
  j = or_ok ();
  if (*s == 0)
    return j;
  fprintf (stderr, "\nIllegal syntax in string at %s.\n", s);
  exit (1);
}

static void
filter (FILE * F)
{
  char *t;

  while (fgets (b, sizeof (b), F)) {
    if (*b != '[' || (t = strchr (b, ']')) == NULL) {
      fputs (b, stdout);
      continue;
    }
    *t = 0;
    if (*++t == '\\' && t[1] == '\n') {
      if (!fgets (g, sizeof (g), F))
	break;
      t = g;
    }
    if (all_ok (b + 1))
      fputs (t, stdout);
  }
}

int
main (int argc, char **argv)
{
  FILE *F;

  if (argc < 3) {
    fprintf (stderr, "\nToo few arguments.\n");
    return 1;
  }
  l = atoi (argv[1]);
  sscanf (argv[2], "0x%8lx:0x%8lx:0x%8lx", &(p.u), &(p.h), &(p.l));
  sscanf (argv[3], "0x%8lx:0x%8lx:0x%8lx", &(m.u), &(m.h), &(m.l));

  if (argc < 5)
    filter (stdin);
  else if ((F = fopen (argv[4], "r")) != NULL) {
    filter (F);
    fclose (F);
  } else if (strcmp (argv[4], "-"))
    filter (stdin);
  else {
    perror (argv[4]);
    return 1;
  }
  exit (0);
}
