
/* iDiRT UAF_RAND Convertor
 * 1.80.02 -> 1.80.13 (Language Support)
 * 1995, 1996 Illusion
 * -------------------------------------------------------------------
 * This will read from the standard output and rewrite to the standard
 * output. Look at the '../bin/conv' script to see how this program is
 * ran.
 *
 * Run 'make convert' to make the module. The module is created in the
 * '../bin' directory as 'convert'. Run the 'conv' script to convert
 * the uaf_rand file. See the 'conv' script for more details.
 *
 * Please feel free to edit this convertor in anyway that it will help
 * you if you change your PERSONA structure.
 */

#include "../kernel.h"
#include <time.h>
#include <errno.h>
#include "../pflags.h"
#include "../sflags.h"
#include "../levels.h"
#include "../nflags.h"
#include "../eflags.h"
#include "../quests.h"

int convert (FILE * from, FILE * to);

int
main (int argc, char *argv[])
{
  FILE *f;
  int errors = 0;

  if (argc == 1) {
    errors += convert (stdin, stdout);
  } else
    while (*++argv != NULL) {
      if ((f = fopen (*argv, "r")) == NULL) {
	perror ("Convert");
	++errors;
      } else {
	errors += convert (f, stdout);
	pclose (f);
      }
    }

  fprintf (stderr, "Errors: %d\n", errors);

  return errors;
}

/* The iDiRT 1.70.00 PERSONA Format
 */
typedef struct {
  char p_name[PNAME_LEN + 1];
  char p_title[TITLE_LEN + 1];
  char p_passwd[PASSWD_LEN];
  long int p_home;
  int p_score;
  int p_strength;
  int p_damage;
  int p_armor;
  SFLAGS p_sflags;
  PFLAGS p_pflags;
  PFLAGS p_mask;
  QFLAGS p_quests;
  NFLAGS p_nflags;
  int p_lang;
  int p_vlevel;
  int p_level;
  time_t p_last_on;
  int p_wimpy;
  long int p_id;
  char p_last_host[MAXHOSTNAMELEN];
  int p_magic;
  int p_channel;
  int p_killed;
  int p_died;
  char p_forget[20][PNAME_LEN + 1];
  int p_pager;
  char p_usrname[MAXHOSTNAMELEN + 20];
} OLD_PERSONA;

int
convert (FILE * from, FILE * to)
{
  PERSONA new;
  OLD_PERSONA old;

  while (fread (&old, sizeof (OLD_PERSONA), 1, from) > 0) {

#ifdef CLEAN_UAF
/* Remove players who have not been on for 2 months and score <= 2020.
 */
    time_t now = time (NULL);

    fprintf (stderr, "[Cleaning Out User File]\n");

    if (now - old.p_last_on > 5184000 && old.p_level <= 3
	&& old.p_score <= 2020) {
      fprintf (stderr, "Deleting %s.\n", old.p_name);
      continue;
    }
#endif

    if (old.p_score < 0 || (old.p_score > 200000 && old.p_level >= 12))
      old.p_score = 0;

    strcpy (new.p_name, old.p_name);
    strcpy (new.p_title, old.p_title);
    strcpy (new.p_passwd, old.p_passwd);
    strcpy (new.p_last_host, old.p_last_host);
    strcpy (new.p_usrname, old.p_usrname);

    new.p_home = old.p_home;

    new.p_score = old.p_score;
    new.p_strength = old.p_strength;
    new.p_damage = old.p_damage;
    new.p_armor = old.p_armor;
    new.p_sflags = old.p_sflags;
    new.p_pflags = old.p_pflags;
    new.p_mask = old.p_mask;
    new.p_quests = old.p_quests;
    new.p_id = old.p_id;

    new.p_vlevel = old.p_vlevel;
    new.p_level = old.p_level;
    new.p_last_on = old.p_last_on;
    new.p_magic = old.p_magic;
    new.p_channel = old.p_channel;
    new.p_wimpy = old.p_wimpy;
    new.p_killed = old.p_killed;
    new.p_died = old.p_died;
    new.p_pager = old.p_pager;
    new.p_lang = old.p_lang;
    new.p_nflags = old.p_nflags;

    /* Give people the Spell flags, and the original iDiRT spells. */
    new.p_eflags = 0;
    xsetbit (new.p_eflags, EFL_FIREBALL);
    xsetbit (new.p_eflags, EFL_MISSILE);
    xsetbit (new.p_eflags, EFL_FROST);
    xsetbit (new.p_eflags, EFL_SHOCK);

    fwrite (&new, sizeof (PERSONA), 1, to);
  }

  return ferror (from) || ferror (to);
}
