
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "kernel.h"
#include "global.h"
#include "utils.h"
#include "crypt.h"

/************************************************************************
 * Functions to use with qsort().					*
 ************************************************************************/
/* Compare players by their level, finding the higher level.
 */
int
cmp_player (const void *a, const void *b)
{
  return plev (*(int *) b) - plev (*(int *) a);
}

/* Compare strings and return which one comes first alphabetically.
 */
int
cmp_alpha (const void *a, const void *b)
{
  return (strcasecmp (a, b));
}

/************************************************************************
 * Functions to handle random number generation.			*
 ************************************************************************/
void
init_rand (void)
{
#ifndef SYS_NO_RANDOM
  srandom (9876);
#else
  srand (9876);
#endif
}

int
randperc (void)
{
  return my_random () % 100;
}

/************************************************************************
 * Functions to handle string comparisions and to operate string 	*
 * comparing tables.							*
 ************************************************************************/

/* Searches a string using a table using a case-sensitive substring
 * search method. The position of the beginning and the end of the
 * substring is stored in the *begin and *end variables.
 * (1996, Illusion)
 */
int
strtlookup (char *elem, char **table, int *begin, int *end)
{
  char **t, *ptr;
  int x;

  for (t = table, x = 0; *t != TABLE_END; ++t, ++x) {
    if (*t == NULL)
      continue;
    if ((ptr = strstr (elem, *t)) != NULL) {
      *begin = ptr - elem;
      *end = (ptr - elem) + strlen (*t);
      return x;
    }
  }
  return (-1);
}

Boolean
match (char *p, char *q)
{
  register char *s = p;
  register char *t = q;
  register char c;
  register char d;

  for (;;) {
    if ((c = *s++) == 0)
      return (*t == 0);
    if ((d = *t++) == 0)
      break;
    if (isupper (c))
      c = tolower (c);
    if (isupper (d))
      d = tolower (d);
    if (c == d)
      continue;
    if (d != '*')
      break;
    if ((d = *t++) == 0)
      return True;
    while (*s++ != d) {
      if (*(s - 1) == 0)
	return False;
    }
  }
  return False;
}

Boolean
infile (char *file, char *line)
{
  register char *p;
  register char *q;
  FILE *fl;
  Boolean invert;
  char a[80];
  char b[80];

  if ((fl = fopen (file, "r")) == 0)
    return False;
  invert = False;
  if (!fgets (a, sizeof a, fl)) {
    fclose (fl);
    return False;
  }
  if (*a == '!') {
    invert = True;
    if (!fgets (a, sizeof a, fl)) {
      fclose (fl);
      return False;
    }
  }
  for (p = line; isspace (*p); ++p) ;
  for (q = b; !isspace (*p) && *p != 0; ++p)
    *q++ = *p;
  *q = 0;

  do {
    for (p = a; isspace (*p); ++p) ;
    for (q = p; !isspace (*q) && *q != 0; ++q) ;
    *q = 0;

    if (match (b, p)) {
      fclose (fl);
      return !invert;
    }
  }
  while (fgets (a, sizeof a, fl));
  fclose (fl);
  return invert;
}

#ifdef SYS_EQBUG
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific written prior permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static u_char charmap[] =
{
  '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
  '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
  '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
  '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
  '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
  '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
  '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
  '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
  '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
  '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
  '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
  '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
  '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
  '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
  '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
  '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
  '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
  '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
  '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
  '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
  '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
  '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
  '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
  '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
  '\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
  '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
  '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
  '\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
  '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
  '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
  '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
  '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int
strcasecmp (char *s1, char *s2)
{
  register u_char *cm = charmap, *us1 = (u_char *) s1, *us2 = (u_char *) s2;

  while (cm[*us1] == cm[*us2++])
    if (*us1++ == '\0')
      return (0);
  return (cm[*us1] - cm[*--us2]);
}

int
strncasecmp (const char *s1, const char *s2, register size_t n)
{
  register u_char *cm = charmap, *us1 = (u_char *) s1, *us2 = (u_char *) s2;

  while (--n >= 0 && cm[*us1] == cm[*us2++])
    if (*us1++ == '\0')
      return (0);
  return (n < 0 ? 0 : cm[*us1] - cm[*--us2]);
}

#endif

int
glookup (char *elem, int n, char **table,
	 int (*strcmpfun) (const char *s1, const char *s2, size_t n))
{
  char **t;
  int x;

  for (t = table, x = 0; *t != TABLE_END; ++t, ++x) {
    if (*t == NULL)
      continue;
    if (strcmpfun (elem, *t, n) == 0)
      return x;
  }
  return (-1);
}

int
tlookup (char *elem, char **table)
{
  return glookup (elem, strlen (elem), table, strncasecmp);
}

/* the function used for xlookup */
static int
xstrcasecmp (const char *s1, const char *s2, size_t n)
{
  /* n is ignored */
  return strcasecmp (s1, s2);
}

/* Like tlookup but uses strcasecmp (actually xstrcasecmp) instead of
 * strncasecmp.
 */
int
xlookup (char *elem, char **table)
{
  return glookup (elem, 0, table, xstrcasecmp);
}

/* Convert a string to lowercase */

char *
lowercase (char *str)
{
  char *p;

  for (p = str; *p; p++)
    if (isupper (*p))
      *p = tolower (*p);
  return str;
}

/* Convert a string to uppercase. */

char *
uppercase (char *str)
{
  char *p;

  for (p = str; *p; p++)
    if (islower (*p))
      *p = toupper (*p);
  return str;
}

/************************************************************************
 * Memory Handling Functions						*
 ************************************************************************/
void
xfree (void *ptr)
{
  free ((void *) ptr);
}

void *
xmalloc (int nelem, int elem_size)
{
  void *p;

  if ((p = calloc (nelem, elem_size)) == NULL) {
    write_date_stderr ();
    fprintf (stderr, "ERROR: xmalloc(); No room to allocate bytes.\n");
    abort ();
  }
  return p;
}

void *
resize_array (void *start, int elem_size, int oldlen, int newlen)
{
  void *p = NULL;

  if (newlen != 0)
    p = xmalloc (newlen, elem_size);

  if (start != NULL) {

    if (newlen != 0) {
      memcpy (p, start, min (oldlen, newlen) * elem_size);
    }
    FREE (start);
  }
  return p;
}

/************************************************************************
 * Miscellaneous Functions						*
 ************************************************************************/
void
write_date_stderr (void)
{
  time_t t;
  char *z;

  time (&t);
  z = ctime (&t);
  z[19] = '\0';
  fprintf (stderr, "%s: ", z);
}

char *
x_strcpy (char *d, char *s)
{
  /* Like strcpy except it returns the pointer just past last char in d */
  while ((*d++ = *s++) != '\0') ;
  return d - 1;
}

char *
my_crypt (char *buf, char *pw, int len)
{
  char *s = crypt (pw, "Mu");

  s[len - 1] = 0;
  return strcpy (buf, s);
}

char *
mk_string (char *b, char *str, int k, int stopch)
{
  unsigned char *s = (unsigned char *) str;
  char *t = b;
  int c;

  while ((c = 0377 & *s++) != stopch && --k >= 0) {
    switch (c) {
    case '\\':
      *t++ = '\\';
      *t++ = '\\';
      break;
    case '\n':
      *t++ = '\\';
      *t++ = 'n';
      break;
    case '\t':
      *t++ = '\\';
      *t++ = 't';
      break;
    case '\b':
      *t++ = '\\';
      *t++ = 'b';
      break;
    case '\f':
      *t++ = '\\';
      *t++ = 'f';
      break;
    case '\r':
      *t++ = '\\';
      *t++ = 'r';
      break;
    case '\"':
      *t++ = '\\';
      *t++ = '\"';
      break;
    default:
      if (isprint (c))
	*t++ = c;
      else {
	*t++ = '\\';
	*t++ = '0' + ((c >> 6) & 3);
	*t++ = '0' + ((c >> 3) & 7);
	*t++ = '0' + (c & 7);
      }
      break;
    }
  }
  *t = 0;
  return b;
}

/************************************************************************
 * A package for handling sets of integers.				*
 * 1993, Nicknack							*
 ************************************************************************/
static Boolean check_for_possible_resize (int_set * p);

void
init_intset (int_set * p, int len)
{
  p->len = p->current = 0;
  p->list = resize_array (NULL, sizeof (int), 0, p->maxlen = len);
}

void
free_intset (int_set * p)
{
  if (p->list != NULL)
    FREE (p->list);
}

Boolean
add_int (int n, int_set * p)
{
  int i;

  check_for_possible_resize (p);

  for (i = 0; i < p->len; i++) {
    if (p->list[i] == n)
      return False;
  }

  p->list[p->len++] = n;

  return True;
}

Boolean
remove_int (int n, int_set * p)
{
  int i;

  if (p->list == NULL)
    return False;

  for (i = 0; i < p->len && p->list[i] != n; i++) ;

  if (i == p->len)
    return False;

  if (i < p->current) {
    if (i < p->current - 1)
      p->list[i] = p->list[p->current - 1];
    i = --p->current;
  }
  p->list[i] = p->list[--p->len];

  check_for_possible_resize (p);

  return True;
}

int
find_int (int n, int_set * p)
{
  int i;

  if (p->list == NULL)
    return -1;

  for (i = 0; i < p->len && p->list[i] != n; i++) ;

  return i < p->len ? i + 1 : 0;
}

int
find_int_number (int n, int_set * p)
{
  return (n < 0 || n >= p->len) ? SET_END : p->list[n];
}

int
foreach_int (int_set * p, int (*func) (int))
{
  int i;
  int n = 0;

  for (i = 0; i < p->len; i++) {

    if (func (p->list[i]))
      n++;
  }

  return n;
}

void
remove_current (int_set * p)
{
  p->list[--p->current] = p->list[--p->len];

  check_for_possible_resize (p);
}

int
get_set_mem_usage (int_set * p)
{
  return p->len * sizeof (int);
}

static Boolean
check_for_possible_resize (int_set * p)
{
  int oldlen = p->maxlen;

  if (p->len == p->maxlen) {

    p->maxlen = p->len < 20 ? 2 * (p->len + 1) : p->len + 25;
  } else if (p->maxlen > 0 && p->len < p->maxlen / 5) {

    p->maxlen /= 2;
  } else {
    return False;
  }

  p->list = resize_array (p->list, sizeof (int), oldlen, p->maxlen);

  return True;
}


/************************************************************************
 * A table package for handling long integer [key + match]'es.		*
 * 1993, Nicknack							*
 ************************************************************************/
static int
hash (long int value, int tbl_size)
{
  return value % tbl_size;
}

void
init_inttable (int_table * p, int size)
{
  p->table = NEW (table_entry *, p->len = size);

  while (--size >= 0)
    p->table[size] = NULL;
}

void
free_inttable (int_table * p)
{
  FREE (p->table);
}

static table_entry *
new_entry (long int key, long int value, table_entry * next)
{
  table_entry *p = NEW (table_entry, 1);

  p->key = key;
  p->value = value;
  p->next = next;

  return p;
}

static void
unlink_entry (table_entry ** entry)
{
  table_entry *p = *entry;

  *entry = (*entry)->next;

  FREE (p);
}


/* Return a pointer to the pointer to the entry that represents 'key',
 * starting the search at the entry to which *p points.
 * If it does not exist, return a pointer to the pointer to the entry that
 * would succede 'key', had it existed.
 */
static table_entry **
find_position (long int key, table_entry ** p)
{
  while (*p != NULL && (*p)->key > key)
    p = &(*p)->next;

  return p;
}

static table_entry *
find_entry (long int key, int_table * p)
{
  table_entry **r = find_position (key, &p->table[hash (key, p->len)]);

  return (*r != NULL && (*r)->key == key) ? *r : NULL;
}

Boolean
insert_entry (long int key, long int value, int_table * p)
{
  table_entry **q = find_position (key, &p->table[hash (key, p->len)]);

  if ((*q) == NULL) {
    *q = new_entry (key, value, NULL);
  } else if ((*q)->key < key) {
    *q = new_entry (key, value, *q);
  } else
    return False;

  return True;
}


Boolean
remove_entry (long int key, int_table * p)
{
  table_entry **q = find_position (key, &p->table[hash (key, p->len)]);

  if ((*q) != NULL && (*q)->key == key) {
    unlink_entry (q);
    return True;
  } else {
    return False;
  }
}

long int
lookup_entry (long int key, int_table * p)
{
  table_entry *q = find_entry (key, p);

  return (q != NULL) ? (q)->value : NOT_IN_TABLE;
}

long int
change_entry (long int key, long int new_value, int_table * p)
{
  table_entry *q = find_entry (key, p);

  if (q != NULL) {
    long int v = q->value;

    q->value = new_value;
    return v;
  } else {
    return NOT_IN_TABLE;
  }
}

int
get_table_mem_usage (int_table * p)
{
  return p->len * sizeof (table_entry *);
}

/************************************************************************
 * Bit Manipulation Functions						*
 ************************************************************************/
/* Test if all of the bits set in M is also set in W */
Boolean
tstbits (int w, int m)
{
  return (bits (w, m) == m);
}

Boolean
tst_bit (LongInt * f, int b)
{
  if (b >= 32)
    return xtstbit (f->h, (b - 32));
  return xtstbit (f->l, b);
}

Boolean
dtst_bit (DLongInt * f, int b)
{
  if (b >= 64)
    return xtstbit (f->u, (b - 64));
  if (b >= 32)
    return xtstbit (f->h, (b - 32));
  return xtstbit (f->l, b);
}

void
set_bit (LongInt * f, int b)
{
  if (b >= 32)
    xsetbit (f->h, (b - 32));
  else
    xsetbit (f->l, b);
}

void
dset_bit (DLongInt * f, int b)
{
  if (b >= 64)
    xsetbit (f->u, (b - 64));
  else if (b >= 32)
    xsetbit (f->h, (b - 32));
  else
    xsetbit (f->l, b);
}

void
clr_bit (LongInt * f, int b)
{
  if (b >= 32)
    xclrbit (f->h, (b - 32));
  else
    xclrbit (f->l, b);
}

void
dclr_bit (DLongInt * f, int b)
{
  if (b >= 64)
    xclrbit (f->u, (b - 64));
  else if (b >= 32)
    xclrbit (f->h, (b - 32));
  else
    xclrbit (f->l, b);
}

/************************************************************************
 * Ban File Utilities							*
 ************************************************************************/
int
addordel (char file_name[80], char check[80])
{
  FILE *fl;

  if ((fl = fopen (file_name, "r")) != NULL)
    if (infile (file_name, check)) {
      fclose (fl);
      return delname (file_name, check);
    } else {
      fclose (fl);
      return addname (file_name, check);
  } else
    return 0;
}

int
addname (char file_name[80], char name[80])
{
  FILE *fl;

  if ((fl = (fopen (file_name, "a"))) == NULL) {
    return 0;
  }
  fprintf (fl, name);
  fprintf (fl, "\n");
  fclose (fl);
  return ADDED;
}

int
delname (char file_name[80], char name[80])
{
  FILE *a, *b;
  char name_in_file[80];

  if ((a = fopen (file_name, "r")) == NULL)
    return 0;
  if ((b = fopen ("TMP_FILE", "w")) == NULL) {
    fclose (a);
    return 0;
  }
  strcat (name, "\n");
  while (fgets (name_in_file, sizeof name_in_file, a))
    if (!EQ (name, name_in_file))
      fprintf (b, "%s", name_in_file);
  fclose (a);
  fclose (b);
  unlink (file_name);
  rename ("TMP_FILE", file_name);
  return DELETED;
}

/************************************************************************
 * Text File Seeking Utilities						*
 * 1996, Illusion							*
 ************************************************************************/
int
fnumlines (char file[200])
{
  FILE *fp;
  char tmp[300];
  int num = 0;

  if ((fp = fopen (file, "rt")) == NULL)
    return -1;

  while (fgets (tmp, sizeof (tmp), fp))
    num++;
  fclose (fp);
  return num;
}

void
fileseek (FILE * file, int lines)
{
  char tmp[300];
  int loc;

  fseek (file, 0L, SEEK_SET);
  for (loc = 0; loc < lines; loc++)
    fgets (tmp, sizeof (tmp), file);
}

/************************************************************************
 * bcopy() and bzero() replacement functions.				*
 * 1996, Illusion							*
 ************************************************************************/
#ifdef NO_BCOPY
void
bcopy (const void *src, void *dest, int n)
{
  memcpy (dest, src, n);
}

void
bzero (void *s, int n)
{
  memset (s, 0, n);
}

#endif

/************************************************************************
 * Compressed File Handling Utilities					*
 * 1996, Illusion							*
 ************************************************************************/
int
check_file_magic (char *filename)
{
  char magic[3];
  FILE *fp;

  if ((fp = fopen (filename, "rb")) == NULL) {
    write_date_stderr ();
    fprintf (stderr, "ERROR: Cannot find file %s.\n", filename);
    return -1;
  }
  fgets (magic, sizeof (magic), fp);

  if (magic[0] != MAGIC_ZHEADER)
    return 0;
  if (magic[1] == MAGIC_GZIP)
    return IS_GZIP;
  if (magic[1] == MAGIC_COMPRESS)
    return IS_COMPRESS;
  return 0;
}

FILE *
zopen (char *filename)
{
  char pipecom[100];
  FILE *fp;
  int type;

  if ((type = check_file_magic (filename)) == -1) {
    write_date_stderr ();
    fprintf (stderr, "ERROR: Call to check_file_magic() failed.\n");
    return NULL;
  }
  if (type != 0) {
    write_date_stderr ();
    fprintf (stderr, "ZOPEN: Compressed file access (Type=%s)\n",
	     IS_GZIP ? "GZIP" : "COMPRESS");
    sprintf (pipecom, "zcat %s", filename);
    if ((fp = popen (pipecom, "r")) == NULL) {
      write_date_stderr ();
      fprintf (stderr, "ZOPEN: Error, popen() failed.\n");
      return NULL;
    }
  } else {
    if ((fp = fopen (filename, "r")) == NULL) {
      write_date_stderr ();
      fprintf (stderr, "ZOPEN: Error, fopen() failed.\n");
      return NULL;
    }
  }
  return (fp);
}

void
zclose (FILE * fp)
{
  struct stat zstat;

  if (fstat (fileno (fp), &zstat) < 0) {
    write_date_stderr ();
    fprintf (stderr, "ZCLOSE: Error, fstat() failed.\n");
    return;
  }
  if (S_ISFIFO (zstat.st_mode))
    pclose (fp);
  else
    fclose (fp);
}
