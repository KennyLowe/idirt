
#include <stdlib.h>
#include <string.h>
#include "kernel.h"
#include "mobiles.h"
#include "sendsys.h"
#include "bprintf.h"
#include "mobile.h"
#include "pflags.h"
#include "sflags.h"
#include "eflags.h"
#include "nflags.h"
#include "lflags.h"
#include "parse.h"
#include "flags.h"
#include "comm.h"
#include "uaf.h"

char *
make_com_text (char *b, char *s, char *t, int send_plr, int recv_plr)
{
  char *p, *q, *r;

  for (p = b, q = s; *q != 0;) {
    if (*q != '%')
      *p++ = *q++;
    else {
      switch (*++q) {
      case 'n':		/* Player Name */
	if (pvis (send_plr) > 0 && see_player (recv_plr, send_plr))
	  *p++ = '(';
	for (r = see_name (recv_plr, send_plr); *r != 0;)
	  *p++ = *r++;
	if (pvis (send_plr) > 0 && see_player (recv_plr, send_plr))
	  *p++ = ')';
	break;
      case 't':		/* Message Text */
	if (t == NULL)
	  return NULL;
	for (r = t; *r != 0;)
	  *p++ = *r++;
	break;
      case 0:
	--q;
	break;
      default:
	*p++ = *q;
      }
      ++q;
    }
  }
  if (p[-1] == '\n')
    --p;
  *p = 0;
  return b;
}

void
com_handler (char *format, char *linename, int lvl, int flg)
{
  char xx[MAX_COM_LEN], txt[MAX_COM_LEN];
  int plr, aplr;

  getreinput (txt);

  if (EMPTY (txt)) {
    bprintf ("Well, what do you want to say?\n");
    return;
  }
  if (plev (mynum) < lvl) {
    bprintf ("Such advanced conversation is beyond you.\n");
    return;
  }
  if (ststflg (mynum, flg)) {
    bprintf ("You must be listening to the %s channel to talk on it.\n", linename);
    return;
  }
  for (plr = 0; plr < max_players; plr++) {
    if (is_in_game (plr) && is_aliased (plr)) {
      aplr = players[plr].aliasto;
      if (!ststflg (aplr, flg))
	sendf (aplr, "%s\n", make_com_text (xx, format, txt, mynum, plr));
    } else if (is_in_game (plr) && plev (plr) >= lvl) {
      if (!ststflg (plr, flg))
	sendf (plr, "%s\n", make_com_text (xx, format, txt, mynum, plr));
    }
  }
}

void
nolinecom (int lvl, int flg, char txt[20])
{
  if (plev (mynum) < lvl) {
    erreval ();
    return;
  }
  if (!ststflg (mynum, flg)) {
    ssetflg (mynum, flg);
    send_msg (DEST_ALL, 0, max (lvl, pvis (mynum)), LVL_MAX, mynum, NOBODY,
	    "&+C[&+W%s &*has left the %s &*line&+C]\n", pname (mynum), txt);
    bprintf ("You are no longer listening to the %s &*line.\n", txt);
  } else {
    sclrflg (mynum, flg);
    send_msg (DEST_ALL, 0, max (lvl, pvis (mynum)), LVL_MAX, mynum, NOBODY,
	  "&+C[&+W%s &*has joined the %s &*line&+C]\n", pname (mynum), txt);
    bprintf ("You are once again listening to the %s &*line.\n", txt);
  }
}

static char *
shout_test (int player, int sender, char *text)
{
  static char buff[MAX_COM_LEN];

  if (player == sender ||
      (plev (player) >= LVL_WIZARD && ststflg (player, SFL_NOSHOUT)) ||
      (plev (player) < LVL_WIZARD && ststflg (player, SFL_DEAF)) ||
      (ltstflg (ploc (player), LFL_SOUNDPROOF) &&
       ploc (sender) != ploc (player)))
    return NULL;

  if (plev (player) >= LVL_WIZARD || plev (sender) >= LVL_WIZARD
      || ploc (player) == ploc (sender)) {
    sprintf (buff, "&+B%s%s%s &+wshouts &+W'&+w%s&+W'\n",
	     pvis (sender) > 0 && see_player (player, sender) ? "(" : "",
	     see_name (player, sender),
	     pvis (sender) > 0 && see_player (player, sender) ? ")" : "",
	     text);
  } else {
    sprintf (buff, "&+wA voice shouts &+W'&+w%s&+W'\n", text);
  }

  return buff;
}

void
shoutcom (void)
{
  char blob[MAX_COM_LEN];

  if (plev (mynum) < LVL_WIZARD && ststflg (mynum, SFL_NOSHOUT)) {
    bprintf ("I'm sorry, you can't shout anymore.\n");
    return;
  } else {
    getreinput (blob);
    if (EMPTY (blob)) {
      bprintf ("What do you want to shout?\n");
      return;
    } else {
      send_g_msg (DEST_ALL, shout_test, mynum, blob);
      if (ststflg (mynum, SFL_HEARBACK))
	bprintf ("You shout: %s\n", blob);
    }
  }
}

void
saycom (void)
{
  char foo[MAX_COM_LEN];
  char lang[30];

  getreinput (foo);
  if (EMPTY (txt1)) {
    bprintf ("What do you want to say?\n");
    return;
  } else {
    sprintf (lang, "(in &+C%s&+w) ", Nflags[plang (mynum)]);

    lsend_msg (ploc (mynum), MODE_LANG | ML (plang (mynum)), MODE_NODEAF,
	       LVL_MIN, LVL_MAX, mynum, NOBODY, "&+G\001p%s\003 &+wsays %s"
	       "&+W'&+w%s&+W'\n", pname (mynum), is_eng (mynum) ? "" : lang,
	       foo);

    lsend_msg (ploc (mynum), MODE_NLANG | ML (plang (mynum)), MODE_NODEAF,
	       LVL_MIN, LVL_MAX, mynum, NOBODY, "&+G\001p%s\003 &*says "
	       "something in a foreign tongue.\n", pname (mynum));

    if (ststflg (mynum, SFL_HEARBACK))
      bprintf ("You say %s: %s\n", is_eng (mynum) ? "" : lang, foo);
  }
}

void
saytocom (void)
{
  char lang[30];
  int plr;

  if ((plr = pl1) == -1) {
    bprintf ("Say what to who?\n");
    return;
  }
  if (ploc (plr) != ploc (mynum)) {
    bprintf ("They aren't here.\n");
    return;
  }
  if (plr == mynum) {
    bprintf ("You mumble to yourself.\n");
    return;
  }
  if (EMPTY (txt2)) {
    bprintf ("What do you want to say?\n");
    return;
  }
  sprintf (lang, "(in &+C%s&+w) ", Nflags[plang (mynum)]);

  lsend_msg (ploc (mynum), MODE_LANG | ML (plang (mynum)), MODE_NODEAF,
	     LVL_MIN, LVL_MAX, mynum, plr, "&+G\001p%s\003 &+wsays to "
	     "&+C\001p%s\003 %s&+W'&+w%s&+W'\n", pname (mynum), pname (plr),
	     is_eng (mynum) ? "" : lang, txt2);

  lsend_msg (ploc (mynum), MODE_NLANG | ML (plang (mynum)), MODE_NODEAF,
	     LVL_MIN, LVL_MAX, mynum, plr, "&+G\001p%s\003 &*says "
	     "something to &+C\001p%s\003 &*in a foreign tongue.\n",
	     pname (mynum), pname (plr));

  lsend_msg (plr, MODE_LANG | ML (plang (mynum)), MODE_NODEAF, LVL_MIN,
	     LVL_MAX, NOBODY, NOBODY, "&+G\001p%s\003 &*says to you %s"
	 "&+W'&*%s&+W'\n", pname (mynum), is_eng (mynum) ? "" : lang, txt2);
  lsend_msg (plr, MODE_NLANG | ML (plang (mynum)), MODE_NODEAF, LVL_MIN,
	     LVL_MAX, NOBODY, NOBODY, "&+G\001p%s\003 &*says something to "
	     "you in a foreign tongue.\n", pname (mynum));

  if (ststflg (mynum, SFL_HEARBACK))
    bprintf ("You say to &+G%s &+w%s: %s\n", pname (plr),
	     is_eng (mynum) ? "" : lang, txt2);
}

void
lsaycom (void)
{
  Boolean is_english;
  char foo[MAX_COM_LEN];
  char lang[30];
  int x;

  if (brkword () == -1) {
    bprintf ("Speak in what language?\n");
    return;
  }
  if ((x = tlookup (wordbuf, Nflags)) < 0) {
    bprintf ("That language doesn't exist!\n");
    return;
  }
  if (!ntstflg (mynum, x)) {
    bprintf ("You don't know how to speak that language!\n");
    return;
  }
  getreinput (foo);

  if (EMPTY (txt2)) {
    bprintf ("What do you want to say?\n");
    return;
  } else {
    sprintf (lang, "(in &+C%s&+w) ", Nflags[x]);
    is_english = (x == NFL_ENGLISH);

    lsend_msg (ploc (mynum), MODE_LANG | ML (x), MODE_NODEAF,
	       LVL_MIN, LVL_MAX, mynum, NOBODY, "&+G\001p%s\003 &+wsays %s"
	       "&+W'&+w%s&+W'\n", pname (mynum), is_english ? "" : lang,
	       foo);

    lsend_msg (ploc (mynum), MODE_NLANG | ML (x), MODE_NODEAF,
	       LVL_MIN, LVL_MAX, mynum, NOBODY, "&+G\001p%s\003 &*says "
	       "something in a foreign tongue.\n", pname (mynum));

    if (ststflg (mynum, SFL_HEARBACK))
      bprintf ("You say: %s\n", foo);
  }
}

void
tellcom (void)
{
  int b;

  if (EMPTY (item1)) {
    bprintf ("Tell who?\n");
    return;
  }
  if ((b = pl1) == -1) {
    bprintf ("No one with that name is playing.\n");
    return;
  }
  if (b == mynum) {
    bprintf ("You talk to yourself.\n");
    return;
  }
  if (EMPTY (txt2)) {
    bprintf ("What do you want to tell them?\n");
    return;
  }
  if (ststflg (mynum, SFL_NOORACLE) && EQ (pname (b), "Oracle") &&
      strstr (txt2, "tell")) {
    bprintf ("You are NoOracle, and cannot use Oracle Tell.\n");
    return;
  }
  if (check_forget (b, mynum)) {
    bprintf ("&+W%s &*has forgotten you and will not receive your messages.\n",
	     pname (b));
    return;
  }
  if (check_forget (mynum, b)) {
    bprintf ("You have forgotten &+W%s&*, leave them in peace.\n", pname (b));
    return;
  }
  if (check_busy (b))
    return;

  if (check_coding (b))
    bprintf ("%s is marked as &+Ccoding &*and might not respond right away.\n",
	     pname (b));
  if (check_away (b)) {
    bprintf ("%s is marked as &+Caway &*and might not respond right away.\n",
	     pname (b));
    bprintf ("Reason: %s\n", players[b].awaymsg);
  }
  if (check_coding (mynum))
    bprintf ("Don't forget that you are marked as &+Ccoding&*.\n");

  if (check_away (mynum))
    bprintf ("Don't forget that you are marked as &+Caway&*.\n");

  if (ltstflg (ploc (b), LFL_SOUNDPROOF) && ploc (mynum) != ploc (b)) {
    bprintf ("%s seems to be in a soundproof room.\n", pname (b));
    return;
  }
  if (ltstflg (ploc (mynum), LFL_SOUNDPROOF) && ploc (mynum) != ploc (b))
    bprintf ("Remember, that you are in a soundproof room and %s can't talk "
	     "back to you\n", pname (b));

  if (b == (max_players + MOB_CATACOMB_SERAPH)) {
    if (strchr (txt2, '?')) {
      switch (my_random () % 4) {
      case 0:
	sprintf (txt2, "Charity");
	break;
      case 1:
	sprintf (txt2, "Faith");
	break;
      case 2:
	sprintf (txt2, "Wisdom");
	break;
      case 3:
	sprintf (txt2, "Courage");
	break;
      }
    } else
      sprintf (txt2, "A blessing be upon your house.");
  }
  sendf (b, "&+C%s &+wtells you &+W'&+w%s&+W'\n", see_name (b, mynum), txt2);

  if (!see_player (b, mynum) && (players[b].replyplr != mynum)) {
    sendf (b, "Use reply to return a tell%s.\n", players[b].replyplr != -1 ?
	   " (Warning: Different Invisible Player)" : "");
    players[b].replyplr = mynum;
  }
  if (ststflg (mynum, SFL_HEARBACK))
    bprintf ("You tell &+W%s&*: %s\n", pname (b), txt2);
}

static char *
anon_test (int plr, int snd, char *txt)
{
  static char buff[MAX_COM_LEN];

  if (ststflg (plr, SFL_NOANON))
    return NULL;

  if (ptstflg (plr, PFL_SEEANON))
    sprintf (buff, "&+w[&+WAnon &+B(&*\001p%s\003&+B)&+w]&+W: &+C'&+w%s&+C'\n",
	     pname (snd), txt);
  else
    sprintf (buff, "&+w[&+WAnon&+w]&+W: &+C'&+w%s&+C'\n", txt);

  return buff;
}

void
anoncom (void)
{
  if (ststflg (mynum, SFL_NOANON)) {
    bprintf ("You must be listening to the anon channel to talk on it.\n");
    return;
  }
  if (EMPTY (txt1)) {
    bprintf ("What would you like to say?\n");
    return;
  }
  send_g_msg (DEST_ALL, anon_test, mynum, txt1);
}

void
wishcom (void)
{
  char x[MAX_COM_LEN];

  if (EMPTY (item1)) {
    bprintf ("Wish for what?\n");
    return;
  }
  getreinput (x);

  if (ststflg (mynum, SFL_NOWISH)) {
    bprintf ("%s\n", plev (mynum) < LVL_WIZARD ?
	     "You find it hard to wish.." :
	     "You can't wish if you don't want to listen to them!");
    return;
  }
  send_msg (DEST_ALL, MODE_NSFLAG | MS (SFL_NOWISH), LVL_WIZARD, LVL_MAX,
	    NOBODY, NOBODY, "&+B[&+wWish from &+W\001p%s\003&+B]\n"
	    "&+B[&+w%s&+B]\n", pname (mynum), x);

  sillycom ("\001s%s\002%s begs and grovels to the powers that be.\n\003");

  if (ststflg (mynum, SFL_HEARBACK) && plev (mynum) < LVL_WIZARD)
    bprintf ("You wish: %s\n", x);
  else
    bprintf ("Ok\n");
}

void
chatcom (void)
{
  int i;

  if (EMPTY (txt1)) {
    bprintf ("&+C[&+wGlobal Chat Channel : %d&+C]\n", pchannel (mynum));
    bprintf ("&+COn Channel: ");
  }
  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && pchannel (i) == pchannel (mynum)) {

      if (EMPTY (txt1))
	bprintf ("&+w%-15s", see_name (mynum, i));

      if (!EMPTY (txt1))
	sendf (i, "&+C-> &+w%s%s%s &+C<- &+B: &+w%s\n",
	       pvis (mynum) > 0 && see_player (i, mynum) ? "(" : "",
	       see_name (i, mynum),
	       pvis (mynum) > 0 && see_player (i, mynum) ? ")" : "",
	       txt1);
    }
  }

  if (EMPTY (txt1))
    bprintf ("\n");
}

void
channelcom (void)
{
  Boolean f, is_me, is_mobile;
  int a, new_channel, i;
  PERSONA p;

  if ((brkword () == -1) && ptstflg (mynum, PFL_CHATMOD)) {
    bprintf ("&+CName                Channel\n");
    bprintf ("&+b---------------------------\n");

    for (i = 0; i < max_players; ++i) {
      if (is_in_game (i))
	bprintf ("&+W%-20s &+C%6d\n", see_name (mynum, i), pchannel (i));
    }

    bprintf ("&+b---------------------------\n");
    return;
  }
  if ((a = find_player (wordbuf, &p, &f)) == -1) {
    bprintf ("Who?\n");
    return;
  }
  is_me = !f && a == mynum;
  is_mobile = !f && a >= max_players;

  if (is_mobile) {
    bprintf ("You cannot change the chat channel of a mobile!\n");
    return;
  }
  is_me = !f && a == mynum;
  is_mobile = !f && a >= max_players;

  if (is_mobile) {
    bprintf ("You cannot change the chat channel of a mobile!\n");
    return;
  }
  if (brkword () == -1) {
    if (is_me) {
      bprintf ("&*Your current chat channel is &+W%d\n", pchannel (mynum));
    } else {
      if (ptstflg (mynum, PFL_CHATMOD))
	bprintf ("&+W%s &*is currently using channel &+C%d\n", p.p_name,
		 p.p_channel);
      else
	bprintf ("You cannot view another players chat channel selection!\n");
    }
    return;
  }
  if (!is_me && !ptstflg (mynum, PFL_CHATMOD)) {
    bprintf ("You cannot change another players chat channel selection!\n");
    return;
  }
  new_channel = max (0, atoi (wordbuf));

  if (is_me && (pchannel (mynum) == new_channel)) {
    bprintf ("Why did you pick the channel that you are already on?\n");
    return;
  }
  if (new_channel < 0 || new_channel > 100000) {
    bprintf ("The channel must be between 0 and 100000.\n");
    return;
  }
  if (!is_me)
    bprintf ("Changing &+W%s's &*channel to &+C%d&*.\n", p.p_name, new_channel);
  else
    bprintf ("Changing your channel to &+C%d&*.\n", new_channel);

  if (f) {
    p.p_channel = new_channel;
    putuaf (&p);
    return;
  }
  for (i = 0; i < max_players; ++i) {
    if (is_in_game (i) && see_player (i, a) && i != a) {
      if (pchannel (i) == pchannel (a))
	sendf (i, "&+C-> &+W%s &*has left the channel &+C<-\n", pname (a));
      if (pchannel (i) == new_channel)
	sendf (i, "&+C-> &+W%s &*has joined the channel &+C<-\n", pname (a));
    }
  }

  setpchannel (a, new_channel);
}

void
forgetcom (void)
{
  int b, i;

  if (brkword () == -1) {
    bprintf ("You are currently forgetting:\n");
    for (i = 0; i < 10; ++i) {
      if (cur_player->forget[i] != -1) {
	if (!is_in_game (cur_player->forget[i]))
	  cur_player->forget[i] = -1;
	else
	  bprintf ("%s\n", pname (cur_player->forget[i]));
      }
    }
    return;
  }
  if ((b = fpbns (wordbuf)) != -1 && seeplayer (b)) {
    if (b >= max_players) {
      bprintf ("Forget a mobile? Why?\n");
      return;
    }
    if (ptstflg (b, PFL_NOFORGET)) {
      bprintf ("You cannot forget about that person.\n");
      return;
    }
    if (check_forget (mynum, b)) {
      bprintf ("Remembering %s.\n", pname (b));
      for (i = 0; i < 10; ++i)
	if (cur_player->forget[i] == b)
	  cur_player->forget[i] = -1;
      return;
    }
    for (i = 0; i < 10; ++i) {
      if (cur_player->forget[i] == -1) {
	bprintf ("Forgetting %s.\n", pname (b));
	cur_player->forget[i] = b;
	return;
      }
    }
    bprintf ("Sorry, you cannot forget anybody else.\n");
    return;
  }
  bprintf ("That player is not online.\n");
}

/* Languages; 1995, Illusion. Concept by Moses.
 * Change current language. If you wish to add more languages, please
 * read the 'doc/lang.doc' file before doing so.
 */
void
langcom (void)
{
  NFLAGS *n = &(nflags (mynum));
  int x;

  if (brkword () == -1) {
    bprintf ("Current Language: %s\n", Nflags[plang (mynum)]);
    bprintf ("Known Languages:\n");
    show_bits ((int *) n, sizeof (NFLAGS) / sizeof (int), Nflags);

    return;
  }
  if ((x = tlookup (wordbuf, Nflags)) < 0) {
    bprintf ("Unknown Language - %s\n", wordbuf);
    bprintf ("Known Languages:\n");
    show_bits ((int *) n, sizeof (NFLAGS) / sizeof (int), Nflags);

    return;
  }
  if (ntstflg (mynum, x)) {
    bprintf ("Language changed to %s.\n", Nflags[x]);
    setplang (mynum, x);
  } else {
    bprintf ("Unknown Language - %s\n", wordbuf);
    bprintf ("Known Languages:\n");
    show_bits ((int *) n, sizeof (NFLAGS) / sizeof (int), Nflags);
  }

}

void
conversecom (void)
{
  int b;

  if (EMPTY (item1)) {
    if (pconv (mynum) == -1)
      bprintf ("Converse with who?\n");
    else
      bprintf ("You are conversing with &+W%s&*.\n", pname (pconv (mynum)));
    return;
  }
  if ((b = pl1) == -1) {
    bprintf ("Who?\n");
    return;
  }
  if (b == mynum) {
    bprintf ("You want to converse with yourself? You aren't Sybil.\n");
    return;
  }
  if (b >= max_players) {
    bprintf ("I'm sure that %s has alot to converse about.\n", pname (b));
    return;
  }
  setpconv (mynum, b);
  bprintf ("Entering Converse Mode                  !<command> = MUD Command  ** = Quit\n");
  bprintf ("You are conversing with: %s\n\n", pname (b));
}

void
replycom (void)
{
  if (cur_player->replyplr == -1) {
    bprintf ("You have nobody to reply to.\n");
    return;
  }
  if (!is_in_game (cur_player->replyplr)) {
    bprintf ("Person you were replying to has left the game.\n");
    cur_player->replyplr = -1;
    return;
  }
  if (EMPTY (txt1)) {
    bprintf ("What do you wish to reply?\n");
    return;
  }
  sendf (cur_player->replyplr, "&+C%s &+wreplies &+W'&+w%s&+W'\n",
	 see_name (cur_player->replyplr, mynum), txt1);

  if (ststflg (mynum, SFL_HEARBACK))
    bprintf ("You reply: %s\n", txt1);
}
