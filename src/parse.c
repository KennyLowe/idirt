

#include "kernel.h"
#include "condact.h"
#include "verbs.h"
#include "sflags.h"
#include "sendsys.h"
#include "locations.h"
#include "objects.h"
#include "parse.h"
#include "mobile.h"
#include "move.h"
#include "commands.h"
#include "climate.h"
#include "wizard.h"
#include "change.h"
#include "clone.h"
#include "pflags.h"
#include "objsys.h"
#include "bprintf.h"
#include "uaf.h"
#include "actions.h"
#include "spell.h"
#include "flags.h"
#include "viewcom.h"
#include "log.h"
#include "exit.h"
#include "wizlist.h"
#include "timing.h"
#include "frob.h"
#include "acct.h"
#include "zones.h"
#include "magic.h"
#include "reboot.h"
#include "mailer.h"
#include "fight.h"
#include "rooms.h"
#include "comm.h"
#include "main.h"
#include "ver.h"
#include "god.h"

#define NOISECHAR(c)	((c) == ' ' || (c) == '.' || (c) == ',' || (c) == '%')

static void parse_1 (char *itemb, int *pl, int *ob, int *pre);
static char *markpos (void);

static char *preptable[] =
{NULL, "at", "on", "with", "to", "in", "into",
 "from", "out", TABLE_END};

static int
xgamecom (char *str, Boolean savecom)
{
  int a;

  if (str == NULL || *str == 0) {
    return 0;
  } else if (EQ (str, "!")) {
    return gamecom (cur_player->prev_com, False);
  } else {
    if (savecom)
      (void) strcpy (cur_player->prev_com, str);

    a = -1;

    /* Converse Mode */
    if (pconv (real_mynum) != -1 && real_mynum < max_players) {
      if (!is_in_game (pconv (mynum))) {
	bprintf ("The person with you in converse mode has quit.\n");
	bprintf ("Exiting Converse Mode\n\n");
	setpconv (mynum, -1);
	return 0;
      }
      if (str[0] == '*' && str[1] == '*') {
	bprintf ("Exiting Converse Mode\n\n");
	setpconv (mynum, -1);
	return 0;
      } else if (str[0] == '!') {
	strcpy (strbuf, str + 1);
      } else {
	sprintf (strbuf, "tell %s %s", pname (pconv (mynum)), str);
      }
      strcpy (str, strbuf);
    }
    /* Translate Macros */
    if (str[0] == '\"' || str[0] == '\'') {	/* Say          */
      strcpy (strbuf, "say ");
      strcat (strbuf, str + 1);
    } else if (str[0] == ':' || str[0] == ';') {	/* Emote        */
      strcpy (strbuf, "emote ");
      strcat (strbuf, str + 1);
    } else if (str[0] == '1') {	/* Wizard Line  */
      strcpy (strbuf, "wiz ");
      strcat (strbuf, str + 1);
    } else if (str[0] == '2') {	/* Prophet Line */
      strcpy (strbuf, "prophet ");
      strcat (strbuf, str + 1);
    } else if (str[0] == '3') {	/* AWiz Line    */
      strcpy (strbuf, "awiz ");
      strcat (strbuf, str + 1);
    } else if (str[0] == '4') {	/* Advisor Line */
      strcpy (strbuf, "advisor ");
      strcat (strbuf, str + 1);
    } else if (str[0] == '5') {	/* God Line     */
      strcpy (strbuf, "god ");
      strcat (strbuf, str + 1);
    } else if (str[0] == '6') {	/* Upper Line   */
      strcpy (strbuf, "upper ");
      strcat (strbuf, str + 1);
    } else if (str[0] == '[') {	/* Chat Line    */
      strcpy (strbuf, "gchat ");
      strcat (strbuf, str + 1);
    } else {			/* No Match     */
      strcpy (strbuf, str);
    }

    stp = 0;

    if (brkword () == -1) {	/* if line contains nothing but  */
      erreval ();		/* pn's or articles (a, an, the) */
      return -1;
    }
    if ((a = chkverb ()) != -1) {
      doaction (a);
      return 0;
    }
    if (fextern (wordbuf) >= 0) {	/* handle external commands */
      stp = 0;
      return 0;
    }
    stp = 0;
    (void) brkword ();
    if ((a = fpbn (wordbuf)) >= 0) {	/* translate NAME to TELL NAME */
      stp = 0;
      a = VERB_TELL;
      doaction (a);
      return 0;
    }
    bprintf ("Pardon?\n");
    return -1;
  }
}

int
gamecom (char *str, Boolean savecom)
{
  int x;

  x = xgamecom (str, savecom);
  special_events (mynum);
  return x;
}

char *
getreinput (char *b)
{
  while (strbuf[stp] == ' ')
    stp++;
  return b != NULL ? strcpy (b, &strbuf[stp]) : &strbuf[stp];
}

int
my_brkword (void)
{
  static int neword = 0;
  int worp;

  if (stp == 0)
    neword = 0;
  if (neword) {
    neword = 0;
    return 0;
  }
  for (;;) {
    while (NOISECHAR (strbuf[stp]))
      stp++;
    for (worp = 0; strbuf[stp] != 0 && !NOISECHAR (strbuf[stp]);)
      wordbuf[worp++] = strbuf[stp++];
    wordbuf[worp] = 0;
    if (EQ (wordbuf, "the") || EQ (wordbuf, "a") || EQ (wordbuf, "an")
	|| EQ (wordbuf, "of") || EQ (wordbuf, "with"))
      continue;

    if (EQ (wordbuf, "it")) {
      if (cur_player->wd_it == NULL) {
	return -1;
      }
      strcpy (wordbuf, cur_player->wd_it);
    }
    if (EQ (wordbuf, "him")) {
      strcpy (wordbuf, cur_player->wd_him);
    }
    if (EQ (wordbuf, "her")) {
      strcpy (wordbuf, cur_player->wd_her);
    }
    if (EQ (wordbuf, "them")) {
      strcpy (wordbuf, cur_player->wd_them);
    }
    if (EQ (wordbuf, "me")) {
      strcpy (wordbuf, pname (mynum));
    }
    break;
  }

  return (worp ? 0 : -1);
}

int
brkword (void)
{
  static int neword = 0;
  int worp;

  if (stp == 0)
    neword = 0;
  if (neword) {
    neword = 0;
    return 0;
  }
  for (;;) {
    while (NOISECHAR (strbuf[stp]))
      stp++;
    for (worp = 0; strbuf[stp] != 0 && !NOISECHAR (strbuf[stp]);)
      wordbuf[worp++] = strbuf[stp++];
    wordbuf[worp] = 0;
    lowercase (wordbuf);
    if (EQ (wordbuf, "the") || EQ (wordbuf, "a") || EQ (wordbuf, "an")
	|| EQ (wordbuf, "of") || EQ (wordbuf, "with"))
      continue;

    if (EQ (wordbuf, "it")) {
      if (cur_player->wd_it == NULL) {
	return -1;
      }
      strcpy (wordbuf, cur_player->wd_it);
    }
    if (EQ (wordbuf, "him")) {
      strcpy (wordbuf, cur_player->wd_him);
    }
    if (EQ (wordbuf, "her")) {
      strcpy (wordbuf, cur_player->wd_her);
    }
    if (EQ (wordbuf, "them")) {
      strcpy (wordbuf, cur_player->wd_them);
    }
    if (EQ (wordbuf, "me")) {
      strcpy (wordbuf, pname (mynum));
    }
    break;
  }

  return (worp ? 0 : -1);
}



int
chkverb ()
{
  return chklist (wordbuf, verbtxt, verbnum);
}

int
chklist (char *word, char *lista[], int listb[])
{
  int a, b, c;
  int d = -1;

  b = c = 0;
  for (a = 0; lista[a]; a++)
    if ((b = Match (word, lista[a])) > c) {
      c = b;
      d = listb[a];
    }
  return (c < 5 ? -1 : d);
}

int
Match (char *x, char *y)
{
  if (strncasecmp (x, y, strlen (x)) == 0)
    return 10000;
  return 0;
}

void
doaction (int vb)
{
  if (do_tables (vb) == 2)
    return;
  if (vb > 1 && vb < 8) {
    if (cur_player->i_follow >= 0) {
      bprintf ("You stopped following.\n");
      cur_player->i_follow = -1;
    }
    dodirn (vb);
    return;
  }
  if (cur_player->inmailer) {
    switch (vb) {
    case VERB_FROB:
    case VERB_ALIAS:
    case VERB_MAIL:
    case VERB_BECOME:
    case VERB_QUIT:
      bprintf ("You can't do that in the mailer.\n");
      return;
    }
  }
  if (cur_player->isforce) {

    switch (vb) {
    case VERB_QUIT:
    case VERB_FROB:
    case VERB_BECOME:
    case VERB_BUG:
    case VERB_TYPO:
    case VERB_FORCE:
    case VERB_MAIL:
    case VERB_KLOCK:
      bprintf ("You can't be forced to do that.\n");
      return;
    }

  }
  switch (vb) {
  case VERB_GO:
    if (cur_player->i_follow >= 0) {
      bprintf ("You stopped following %s.\n", pname (cur_player->i_follow));
      cur_player->i_follow = -1;
    }
    dogocom ();
    break;

  case VERB_DEBUG:
    /*debugcom ();*/
    break;

  case VERB_ICESTORM:
    spellcom (VERB_ICESTORM);
    break;
  case VERB_NORUN:
    noruncom ();
    break;
  case VERB_REPLY:
    replycom ();
    break;
  case VERB_USOCK:
    usesocketcom ();
    break;
  case VERB_JUDGE:
    judgecom ();
    break;
  case VERB_LEVFORCE:
    levforcecom ();
    break;
  case VERB_RELOADACT:
    reboot_actions ();
    break;
  case VERB_SAYTO:
    saytocom ();
    break;
  case VERB_ROSE:
    rosecom ();
    break;
  case VERB_WIPE:
    wipecom ();
    break;
  case VERB_FLUSH:
    flushcom ();
    break;
  case VERB_USE:
    usecom ();
    break;
  case VERB_UNLOCK:
    unlockcom ();
    break;
  case VERB_LOCK:
    lockcom ();
    break;
  case VERB_CLOSE:
    closecom ();
    break;
  case VERB_OPEN:
    opencom ();
    break;
  case VERB_PROBATION:
    probationcom ();
    break;
  case VERB_LINK:
    linkcom ();
    break;
  case VERB_RING:
    ringcom ();
    break;
  case VERB_SHOUT:
    shoutcom ();
    break;
  case VERB_SAY:
    saycom ();
    break;
  case VERB_TELL:
    tellcom ();
    break;
  case VERB_LOOK:
    lookcom ();
    break;
  case VERB_SCORE:
    scorecom ();
    break;
  case VERB_INVENTORY:
    inventory ();
    break;
  case VERB_TIE:
    tiecom ();
    break;
  case VERB_UNTIE:
    untiecom ();
    break;
  case VERB_DIG:
    digcom ();
    break;
  case VERB_GIVE:
    givecom ();
    break;

  case VERB_VALUE:
    valuecom ();
    break;
  case VERB_COMPARE:
    comparecom ();
    break;
  case VERB_WEARALL:
    wearall ();
    break;
  case VERB_BLUR:
    spellcom (VERB_BLUR);
    break;
  case VERB_BHANDS:
    spellcom (VERB_BHANDS);
    break;
  case VERB_ARMOR:
    spellcom (VERB_ARMOR);
    break;
  case VERB_DAMAGE:
    spellcom (VERB_DAMAGE);
    break;
  case VERB_SPELLS:
    spellscom ();
    break;
  case VERB_PUNTALL:
    puntallcom ();
    break;
  case VERB_FREAQ:
    freaqcom ();
    break;
  case VERB_CONVERSE:
    conversecom ();
    break;
  case VERB_AID:
    spellcom (VERB_AID);
    break;
  case VERB_VTOUCH:
    spellcom (VERB_VTOUCH);
    break;
  case VERB_EFLAGS:
    eflagscom ();
    break;
  case VERB_BANHOST:
    bancom (BAN_HOSTS, BANHOST);
    break;
  case VERB_BANUSER:
    bancom (BAN_CHARS, BANUSER);
    break;
  case VERB_BANLOGIN:
    bancom (BAN_LOGIN, BANLOGIN);
    break;
  case VERB_BANCHECK:
    bancom (BAN_USRHOSTS, BANCHECK);
    break;
  case VERB_WEATHER:
    weathercom ();
    break;
  case VERB_CTIME:
    climatetime ();
    break;
  case VERB_SETTIME:
    settimecom ();
    break;
  case VERB_LSAY:
    lsaycom ();
    break;
  case VERB_LANG:
    langcom ();
    break;
  case VERB_NFLAGS:
    nflagscom ();
    break;
  case VERB_MAXSTATE:
    maxstatecom ();
    break;
  case VERB_LOG:
    writelog ();
    break;
  case VERB_ATVIS:
    atviscom ();
    break;
  case VERB_SIC:
    siccom ();
    break;
  case VERB_POLICY:
    policycom ();
    break;
  case VERB_PLOC:
    ploccom ();
    break;
  case VERB_CFR:
    cfrcom ();
    break;
  case VERB_BURN:
    burncom ();
    break;
  case VERB_SILENT:
    silentcom ();
    break;
  case VERB_WLOAD:
    wloadcom ();
    break;
  case VERB_VIEWCOM:
    viewcom ();
    break;
  case VERB_KLOCK:
    klockcom (NULL);
    break;
  case VERB_OPTIONS:
    optionscom ();
    break;
  case VERB_PAGER:
    setpager ();
    break;
  case VERB_FAKEQUIT:
    fakequitcom ();
    break;
  case VERB_SETIN:
    set_msg (cur_player->setin, False, False);
    break;
  case VERB_SETOUT:
    set_msg (cur_player->setout, True, False);
    break;
  case VERB_SETMIN:
    set_msg (cur_player->setmin, False, False);
    break;
  case VERB_SETMOUT:
    set_msg (cur_player->setmout, False, False);
    break;
  case VERB_SETVIN:
    set_msg (cur_player->setvin, False, False);
    break;
  case VERB_SETVOUT:
    set_msg (cur_player->setvout, False, False);
    break;
  case VERB_SETQIN:
    set_msg (cur_player->setqin, False, False);
    break;
  case VERB_SETQOUT:
    set_msg (cur_player->setqout, False, False);
    break;
  case VERB_SETSIT:
    set_msg (cur_player->setsit, False, False);
    break;
  case VERB_SETSTND:
    set_msg (cur_player->setstand, False, False);
    break;
  case VERB_SETSUM:
    set_msg (cur_player->setsum, False, False);
    break;
  case VERB_SETSIN:
    set_msg (cur_player->setsumin, False, True);
    break;
  case VERB_SETSOUT:
    set_msg (cur_player->setsumout, False, True);
    break;
  case VERB_NOPUNT:
    nopuntcom ();
    break;
  case VERB_FIND:
    findcom ();
    break;
  case VERB_FOLLIST:
    follist ();
    break;
  case VERB_FORCEALL:
    forceallcom ();
    break;
  case VERB_SEEEXT:
    toggleseeext ();
    break;
  case VERB_SIGNAL:
    signalcom ();
    break;
  case VERB_REBOOT:
    rebootcom ();
    break;
  case VERB_UPDATE:
    updatecom ();
    break;
  case VERB_BULL:
    mbullcom ();
    break;
  case VERB_WBULL:
    wbullcom ();
    break;
  case VERB_ABULL:
    abullcom ();
    break;
  case VERB_UBULL:
    ubullcom ();
    break;
  case VERB_CODING:
    togglecoding ();
    break;
  case VERB_SOCKET:
    socketcom ();
    break;
  case VERB_SEESOCKET:
    toggleseesocket ();
    break;
  case VERB_LIT:
    if (plev (mynum) < LVL_WIZARD)
      spellcom (VERB_LIT);
    else
      litcom ();
    break;
  case VERB_UPTIME:
    uptimecom ();
    break;
  case VERB_HEALALL:
    healallcom ();
    break;
  case VERB_SAVEALL:
    saveallcom ();
    break;
  case VERB_SHALL:
    if (plev (mynum) > LVL_WIZARD && ptstflg (mynum, PFL_HEAL)) {
      healallcom ();
      saveallcom ();
    } else {
      erreval ();
    }
    break;
  case VERB_FINGER:
    fingercom ();
    break;
  case VERB_GCHAT:
    chatcom ();
    break;
  case VERB_CHANNEL:
    channelcom ();
    break;
  case VERB_FORGET:
    forgetcom ();
    break;
  case VERB_FLIST:
    forgetlist ();
    break;
  case VERB_WHON:
    whocom (True);
    break;
  case VERB_PZAP:
    pzapcom ();
    break;
  case VERB_BEEP:
    beepcom ();
    break;
  case VERB_SEEIDLE:
    seeidlecom ();
    break;
  case VERB_CLEAR:
    bprintf ("@C");
    break;
  case VERB_LEVECHO:
    levechocom ();
    break;
  case VERB_PUNT:
    puntcom ();
    break;
  case VERB_IDLE:
    idlecom ();
    break;
  case VERB_NOSLAIN:
    noslaincom ();
    break;
  case VERB_NOWISH:
    nowishcom ();
    break;
  case VERB_BRESET:
    bresetcom ();
    break;
  case VERB_TXTRAW:
    textrawcom ();
    break;
  case VERB_SYSTEM:
    systemcom ();
    break;
  case VERB_RAW:
    rawcom ();
    break;
  case VERB_MSTAT:
    mstatcom ();
    break;
  case VERB_ACCT:
    acctcom ();
    break;
  case VERB_OBJECTS:
    objectscom ();
    break;
  case VERB_QDONE:
    qdonecom ();
    break;
  case VERB_NOPUFF:
    togglecom (SFL_NOPUFF, "NoPuff Enabled", "NoPuff Disabled");
    break;
  case VERB_NOBEEP:
    togglecom (SFL_NOBEEP, "NoBeep Enabled", "NoBeep Disabled");
    break;
  case VERB_NOFLAG:
    togglecom (SFL_NOFLAG, "NoFlags Enabled", "NoFlags Disabled");
    break;
  case VERB_NOWET:
    togglecom (SFL_NOWET, "NoWeather Enabled", "NoWeather Disabled");
    break;
  case VERB_NOBLINK:
    togglecom (SFL_NOBLINK, "Blinking Disabled", "Blinking Enabled");
    break;
  case VERB_NEWSTYLE:
    togglecom (SFL_NEWSTYLE, "NewStyle Enabled", "NewStyle Disabled");
    break;
  case VERB_AUTOEXIT:
    togglecom (SFL_AUTOEXIT, "AutoExit Enabled", "AutoExit Disabled");
    break;
  case VERB_HEARBACK:
    togglecom (SFL_HEARBACK, "Hearback Enabled", "Hearback Disabled");
    break;
  case VERB_NOINV:
    togglecom (SFL_NOINV, "NoInventory Enabled", "NoInventory Disabled");
    break;
  case VERB_NOFIGHT:
    togglecom (SFL_NOFIGHT, "NoFight Enabled", "NoFight Disabled");
    break;
  case VERB_AWAY:
    togglecom (SFL_AWAY, "Marking you as away from keyboard.",
	       "Marking you as returned to keyboard.");
    break;
  case VERB_BUSY:
    togglecom (SFL_BUSY, "Marking you as busy to lower levels.",
	       "Marking you as no longer busy.");
    break;
  case VERB_BRIEF:
    togglecom (SFL_BRIEF, "Brief Mode Enabled", "Brief Mode Disabled");
    break;
  case VERB_GOSSIP:
    com_handler ("&*[&+MGossip&*] &+M%n: &*%t", "&+MGossip", LVL_MIN, SFL_NOGOSSIP);
    break;
  case VERB_ANON:
    anoncom ();
    break;
  case VERB_TTY:
    ttycom ();
    break;
  case VERB_NOGOSSIP:
    nolinecom (LVL_MIN, SFL_NOGOSSIP, "&+MGossip");
    break;
  case VERB_NOANON:
    nolinecom (LVL_MIN, SFL_NOANON, "&+WAnon");
    break;
  case VERB_NOWIZ:
    nolinecom (LVL_WIZARD, SFL_NOWIZ, "&+YWizard");
    break;
  case VERB_NOPROPHET:
    nolinecom (LVL_PROPHET, SFL_NOPROPHET, "&+WProphet");
    break;
  case VERB_NOAWIZ:
    nolinecom (LVL_ARCHWIZARD, SFL_NOAWIZ, "&+CArchWizard");
    break;
  case VERB_NOADV:
    nolinecom (LVL_ADVISOR, SFL_NOADV, "&+RAdvisor");
    break;
  case VERB_NOGOD:
    nolinecom (LVL_AVATAR, SFL_NOGOD, "&+BGod");
    break;
  case VERB_NOUPPER:
    nolinecom (LVL_GOD, SFL_NOUPPER, "&+GUpper");
    break;
  case VERB_WIZ:
    com_handler ("&*[&+Y1&*] &+Y%n: &*%t", "&+YWizard", LVL_WIZARD, SFL_NOWIZ);
    break;
  case VERB_PROPHET:
    com_handler ("&*[&+W2&*] &+W%n: &*%t", "&+WProphet", LVL_PROPHET, SFL_NOPROPHET);
    break;
  case VERB_AWIZ:
    com_handler ("&*[&+C3&*] &+C%n: &*%t", "&+CAWiz", LVL_ARCHWIZARD, SFL_NOAWIZ);
    break;
  case VERB_ADVISOR:
    com_handler ("&*[&+R4&*] &+R%n: &*%t", "&+RAdvisor", LVL_ADVISOR, SFL_NOADV);
    break;
  case VERB_GOD:
    com_handler ("&*[&+B5&*] &+B%n: &*%t", "&+BGod", LVL_AVATAR, SFL_NOGOD);
    break;
  case VERB_UPPER:
    com_handler ("&*[&+G6&*] &+G%n: &*%t", "&+GUpper", LVL_GOD, SFL_NOUPPER);
    break;
  case VERB_SFLAGS:
    sflagscom ();
    break;

/********************************************************************/

  case VERB_FLOWERS:
    flowercom ();
    break;
  case VERB_DESCRIPTION:
    change_desc ();
    break;
  case VERB_CLONE:
    clonecom (True);
    break;
  case VERB_DESTRUCT:
    destructcom (NULL);
    break;
  case VERB_LOAD:
    loadcom ();
    break;
  case VERB_STORE:
    storecom ();
    break;
  case VERB_TICKLE:
    ticklecom ();
    break;
  case VERB_PET:
    petcom ();
    break;
  case VERB_QUIT:
    quit_game ();
    break;
  case VERB_SIT:
    sitcom ();
    break;
  case VERB_STAND:
    standcom (mynum);
    break;
  case VERB_GET:
    getcom ();
    break;
  case VERB_DROP:
    dropobj ();
    break;
  case VERB_WHO:
    whocom (False);
    break;
  case VERB_MWHO:
    mwhocom ();
    break;
  case VERB_RESET:
    resetcom (RES_TEST);
    break;
  case VERB_ZAP:
    zapcom ();
    break;
  case VERB_EAT:
    eatcom ();
    break;
  case VERB_SAVE:
    saveother ();
    break;
  case VERB_SAVESET:
    saveset ();
    break;
  case VERB_GLOBAL:
    globalcom ();
    break;
  case VERB_STEAL:
    stealcom ();
    break;
  case VERB_REVIEW:
    reviewcom ();
    break;
  case VERB_LEVELS:
    levelscom ();
    break;
  case VERB_COUPLES:
    file_pager (COUPLES);
    break;
  case VERB_WIZLIST:
    wizlistcom ();
    break;
  case VERB_MAIL:
    mailcom ();
    break;
  case VERB_PROMPT:
    promptcom ();
    break;
  case VERB_TOUT:
    toutcom ();
    break;
  case VERB_INFO:
    infocom ();
    break;
  case VERB_QUESTS:
    questcom ();
    break;
  case VERB_TOURNAMENT:
    tournamentcom ();
    break;
  case VERB_SYSLOG:
    syslogcom ();
    break;
  case VERB_DELETE:
    deletecom ();
    break;
  case VERB_OPENGAME:
    opengamecom ();
    break;
  case VERB_HELP:
    helpcom ();
    break;
  case VERB_STATS:
    showplayer ();
    break;
  case VERB_EXAMINE:
    examcom ();
    break;
  case VERB_EXORCISE:
    exorcom ();
    break;
  case VERB_SUMMON:
    sumcom ();
    break;
  case VERB_WIELD:
    wieldcom ();
    break;
  case VERB_KILL:
    killcom ();
    break;
  case VERB_POSE:
    posecom ();
    break;
  case VERB_SET:
    setcom ();
    break;
  case VERB_PRAY:
    praycom ();
    break;
  case VERB_TIPTOE:
    gotocom (True);
    break;
  case VERB_GOTO:
    gotocom (False);
    break;
  case VERB_WEAR:
    wearcom ();
    break;
  case VERB_REMOVE:
    removecom ();
    break;
  case VERB_PUT:
    putcom ();
    break;
  case VERB_WAVE:
    wavecom ();
    break;
  case VERB_FORCE:
    forcecom ();
    break;
  case VERB_LIGHT:
    lightcom ();
    break;
  case VERB_EXTINGUISH:
    extinguishcom ();
    break;
  case VERB_CRIPPLE:
    cripplecom ();
    break;
  case VERB_CURE:
    curecom ();
    break;
  case VERB_HEAL:
    healcom ();
    break;
  case VERB_MUTE:
    dumbcom ();
    break;
  case VERB_CHANGE:
    changecom ();
    break;
  case VERB_MISSILE:
    spellcom (VERB_MISSILE);
    break;
  case VERB_SHOCK:
    spellcom (VERB_SHOCK);
    break;
  case VERB_FIREBALL:
    spellcom (VERB_FIREBALL);
    break;
  case VERB_FROST:
    spellcom (VERB_FROST);
    break;
  case VERB_BLOW:
    blowcom ();
    break;
  case VERB_EXITS:
    exitcom ();
    break;
  case VERB_PUSH:
    pushcom ();
    break;
  case VERB_IN:
    incom (True);
    break;
  case VERB_AT:
    incom (False);
    break;
  case VERB_INVISIBLE:
    inviscom ();
    break;
  case VERB_VISIBLE:
    viscom ();
    break;
  case VERB_DEAFEN:
    deafcom ();
    break;
  case VERB_RESURRECT:
    resurcom ();
    break;
  case VERB_TITLE:
    change_title ();
    break;
  case VERB_SETSTART:
    setstart ();
    break;
  case VERB_LOCATIONS:
    locationscom ();
    break;
  case VERB_ZONES:
    zonescom ();
    break;
  case VERB_USERS:
    usercom ();
    break;
  case VERB_BECOME:
    becom (NULL);
    break;
  case VERB_SNOOP:
    snoopcom ();
    break;
  case VERB_ROLL:
    rollcom ();
    break;
  case VERB_CREDITS:
    bprintf ("\001f%s\003", CREDITS);
    break;
  case VERB_JUMP:
    jumpcom ();
    break;
  case VERB_WHERE:
    wherecom ();
    break;
  case VERB_FLEE:
    fleecom ();
    break;
  case VERB_BUG:
    bugcom ();
    break;
  case VERB_TYPO:
    typocom ();
    break;
  case VERB_ACTIONS:
    lisextern ();
    break;
  case VERB_PN:
    pncom ();
    break;
  case VERB_BLIND:
    blindcom ();
    break;
  case VERB_MASK:
    maskcom ();
    break;
  case VERB_PFLAGS:
    pflagscom ();
    break;
  case VERB_MFLAGS:
    mflagscom ();
    break;
  case VERB_FROB:
    frobcom (NULL);
    break;
  case VERB_SHUTDOWN:
    shutdowncom (False);
    break;
  case VERB_CRASH:
    shutdowncom (True);
    break;
  case VERB_EMOTE:
    emotecom ();
    break;
  case VERB_EMOTETO:
    emotetocom ();
    break;
  case VERB_EMPTY:
    emptycom ();
    break;
  case VERB_TIME:
    timecom ();
    break;
  case VERB_TREASURES:
    treasurecom ();
    break;
  case VERB_WAR:
    warcom ();
    break;
  case VERB_PEACE:
    peacecom ();
    break;
  case VERB_QUIET:
    togglecom (SFL_QUIET, "Quiet Mode Enabled", "Quiet Mode Disabled");
    break;
  case VERB_NOSHOUT:
    noshoutcom ();
    break;
  case VERB_COLOR:
    if (ststflg (mynum, SFL_COLOR)) {
      bprintf ("Back to black and white.\n");
      sclrflg (mynum, SFL_COLOR);
    } else {
      bprintf ("Welcome to the wonderful world of &+RC&+GO&+YL&+BO&+MR&+C!\n");
      ssetflg (mynum, SFL_COLOR);
    }
    break;
  case VERB_ALOOF:
    togglecom (SFL_ALOOF, "Aloof Enabled", "Aloof Disabled");
    break;
  case VERB_SHOW:
    showitem ();
    break;
  case VERB_WIZLOCK:
    wizlock ();
    break;
  case VERB_FOLLOW:
    followcom ();
    break;
  case VERB_LOSE:
    losecom ();
    break;
  case VERB_ECHO:
    echocom ();
    break;
  case VERB_ECHOALL:
    echoallcom ();
    break;
  case VERB_ECHOTO:
    echotocom ();
    break;
  case VERB_WISH:
    wishcom ();
    break;
  case VERB_TRACE:
    tracecom ();
    break;
  case VERB_START:
    startcom ();
    break;
  case VERB_STOP:
    stopcom ();
    break;
  case VERB_MOBILES:
    mobilecom ();
    break;
  case VERB_UNVEIL:
    unveilcom (NULL);
    break;
  case VERB_BANG:
    if (plev (mynum) < LVL_SORCERER)
      erreval ();
    else
      broad ("\001dA huge crash of thunder echoes through the land.\n\003");
    break;
  case VERB_TRANSLOCATE:
    sumcom ();
    break;
  case VERB_ALIAS:
    aliascom ();
    break;
  case VERB_LFLAGS:
    lflagscom ();
    break;
  case VERB_PRIVS:
    pflagscom ();
    break;
  case VERB_VERSION:
    versioncom ();
    break;
  case VERB_MEDITATE:
    meditatecom ();
    break;
  default:
    mudlog ("PARSER: No match for verb = %d", vb);
    bprintf ("You can't do that now.\n");
    break;
  }
}


void
quit_game (void)
{
  char xx[128];

  if (cur_player->aliased) {
    unalias (real_mynum);
    return;
  }
  if (pfighting (mynum) >= 0) {
    bprintf ("Not in the middle of a fight!\n");
    return;
  }
  if (cur_player->polymorphed >= 0) {
    bprintf ("A mysterious force won't let you quit.\n");
    return;
  }
  bprintf ("Ok");

  send_msg (ploc (mynum), 0, pvis (mynum), LVL_MAX, mynum, NOBODY,
  "%s\n", build_setin (xx, cur_player->setqout, pname (mynum), NULL, NULL));

  send_msg (DEST_ALL, MODE_QUIET, max (pvis (mynum), LVL_WIZARD),
	    LVL_MAX, mynum, NOBODY, "&+B[&+CQuitting Game: &+W%s&+B]\n", pname (mynum));

  crapup ("\001f" GOODBYE "\003", CRAP_SAVE);
}

void
erreval (void)
{
  bprintf ("You can't do that now.\n");
}

Boolean
parse_2 (int vb)
{
  int savestp = stp;
  int ob, pl, pre;

  *item1 = *item2 = '\0';

  if (vb == -1)
    return False;
  ob1 = pl1 = ob2 = pl2 = -1;
  prep = 0;
  txt1 = markpos ();
  txt2 = NULL;
  parse_1 (item1, &pl, &ob, &pre);
  if (pre == 0) {
    ob1 = ob;
    pl1 = pl;
    txt2 = markpos ();
    parse_1 (item2, &pl, &ob, &pre);
  }
  if (pre != 0) {
    prep = pre;
    parse_1 (item2, &pl, &ob, &pre);
  }
#ifdef DEBUG_PARSER
  if (pre != 0) {
    bprintf ("Huh?\n");
    return False;
  } else {
    pl2 = pl;
    ob2 = ob;
  }
#else
  pl2 = pl;
  ob2 = ob;
#endif
  stp = savestp;
  return True;
}

static void
parse_1 (char *itemb, int *pl, int *ob, int *pre)
{
  int o, p;

  *pl = *ob = -1;
  *pre = 0;
  if (brkword () != -1) {
    strcpy (itemb, wordbuf);
    if ((p = findprep (itemb)) != -1) {
      *pre = p;
    } else {
      /* It's not a preposition. */
      if ((p = *pl = fpbn (itemb)) != -1) {
	if (psex (p)) {
	  cur_player->wd_them = strcpy (cur_player->wd_her, pname (p));
	} else {
	  cur_player->wd_them = strcpy (cur_player->wd_him, pname (p));
	}
      }
      if ((o = *ob = fobnc (itemb)) != -1 || (o = *ob = fobna (itemb)) != -1) {
	cur_player->wd_it = oname (o);
      }
    }
  }
}

static char *
markpos (void)
{
  register int c;

  while ((c = strbuf[stp]) == ',' || isspace (c))
    ++stp;
  return strbuf + stp;
}

int
findprep (char *t)
{
  return xlookup (t, preptable);
}

int
prmmod (int p)
{
  switch (p) {
  case FL_PL1:
    return pl1;
  case FL_PL2:
    return pl2;
  case FL_OB1:
    return ob1;
  case FL_OB2:
    return ob2;
  case FL_CURCH:
    return ploc (mynum);
  case FL_PREP:
    return prep;
  case FL_LEV:
    return plev (mynum);
  case FL_STR:
    return pstr (mynum);
  case FL_SEX:
    return psex (mynum);
  case FL_SCORE:
    return pscore (mynum);
  case FL_MYNUM:
    return mynum;
  }

  if (IS_PL (p))
    return p - PL_CODE + max_players;
  return p;
}

/* This function drives the function tables. */
int
do_tables (int a)
{
  if (!parse_2 (a))
    return 2;
  else
    return 1;
}
