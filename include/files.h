#ifndef _FILES_H
#define _FILES_H

#define DATA_DIR        "../data/"      /* Directory for data files	*/
#define DESC_DIR	"DESC"		/* Character descriptions	*/
#define MAIL_DIR	"MAIL"		/* Directory to keep mail	*/
#define INFO_DIR	"INFO"		/* Information files (*.i)	*/
#define HELP_DIR	"HELP"		/* Directory for help files	*/
#define BULL_DIR	"BULL"		/* Directory for bulletin files	*/
#define WIZ_ZONES       "WIZ_ZONES"     /* Wizard's personal creations	*/
#define	LOG_DIR		"LOGS"		/* Player logging directory	*/
#define POLICY_DIR	"POLICY"	/* Directory for policies	*/
#define	TEMP_DIR	"TMP"		/* Temporary data directory	*/

#define WELCOME		"title"		/* Title screen			*/
#define GOODBYE		"goodbye"	/* Exit screen (quit game)	*/
#define BAN_CHARS	"banned_chars"	/* List of banned characters	*/
#define ILLEGAL_FILE    "illegal"       /* Illegal character names	*/
#define BAN_ACCTS	"banned_accts"	/* List of banned accounts	*/
#define BAN_HOSTS	"banned_hosts"	/* List of banned hosts		*/
#define BAN_USRHOSTS	"banned_check"	/* Hosts can't do user at login	*/
#define	BAN_LOGIN	"banned_login"	/* Hosts can't connect		*/
#define SILENT_LOGIN	"silent_login"	/* Hosts w/out socket messages	*/
#define BOOTSTRAP	"bootstrap"	/* Points to text files		*/
#define CREDITS		"credits"	/* Author credits		*/
#define EXTERNS		"actions"	/* External actions		*/
#define OBJECTS		"objects"	/* Object names and desc	*/
#define GWIZ		"wiz.txt"	/* "Welcome to Wizard" message	*/
#define FULLHELP	"help"		/* Detailed command description	*/
#define HELP1		"help.mortal"	/* Commands for Mortals		*/
#define HELP2		"help.wizard"	/* Commands for Wizards		*/
#define HELP3		"help.prophet"	/* Commands for Prophets	*/
#define HELP4		"help.awiz"	/* Commands for ArchWizards	*/
#define HELP5		"help.advisor"	/* Commands for Advisors	*/
#define HELP6		"help.avatar"	/* Commands for Avatars		*/
#define HELP7		"help.upper"	/* Commands for Gods		*/
#define COUPLES		"couples"	/* Couples list			*/
#define INFO		"info"		/* Brief information on game	*/
#define POLICY		"policy"	/* List of policies		*/
#define BULLETIN1	"bull.mortal"	/* Bulletins for Mortals	*/
#define BULLETIN2	"bull.wizard"	/* Bulletins for Wizards	*/
#define BULLETIN3	"bull.awiz"	/* Bulletins for AWiz/Advisor	*/
#define BULLETIN4	"bull.upper"	/* Bulletins for Avatar+	*/
#define SETIN_FILE	"setins"	/* Player's travel messages	*/
#define LOG_FILE	"syslog"        /* The system's log-file	*/
#define MOTD		"motd"		/* Message of the day		*/
#define ISSUE		"issue"		/* Message of the day		*/
#define NOLOGIN		"nologin"	/* If exists, no logins		*/
#define UAF_RAND	"uaf_rand"      /* User database		*/
#define PRIVED_FILE     "prived"        /* Priveliged players		*/
#define MONITOR_FILE    "monitor"       /* Players to monitor		*/
#define PID_FILE        "pid"           /* PID of MUD Daemon running	*/
#define	HOSTS		"hosts"		/* Aliased hosts for MUD	*/
#define	SIGNAL1		"sigmsg1"	/* SIGUSR1 crash message	*/
#define	SIGNAL2		"sigmsg2"	/* SIGUSR2 crash message	*/
#define	FROBCHT		"frobcht"	/* Frobbing chart for Frob	*/

#define ID_COUNTER      WIZ_ZONES"/ID_COUNTER"

#endif
