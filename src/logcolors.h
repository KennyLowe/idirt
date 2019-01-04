#ifndef _LOGCOLORS_H
#define _LOGCOLORS_H

char *LogTable[] = {
	"ERROR",	"SIGNAL",	"PERROR",	"PARSER",
	"-----",	"DEBUG",	"SYSTEM",	"BOOTUP",
	"UPDATE",	"REBOOT",	"UPGRADE",	"RESET",
	"QUEST",	"UNVEIL",	"ENTRY",	"SOCKET",
	"EXIT",		"CHANGE",	"CLONE",	"DESTRUCT",
	"LINK",		"MAXSTATE",	"BUG",		"TYPO",
	"PFLAG",	"MASK",		"MFLAG",	"LFLAG",
	"SFLAG",	"NFLAG",	"EFLAG",	"FROB",
	"FREAQ",	"TOUT",		"PZAP",		"BANHOST",
	"BANLOGIN",	"BANUSER",	"BANCHECK",	"ZAP",
	"DELETE",	"PROBATION",	"BURN",		"BRESET",
	"ZOPEN",	"ZCLOSE",	"BOOTSTRAP",	"LOG",
	"CHEAT",	"MONITOR",	TABLE_END
};

char *LogColors[] = {
	"&=Wr",		"&=Wr",		"&=Wr",		"&=Wr",
	"&=Wr",		"&=lr",		"&=cl",		"&=cl",
	"&=Cl",		"&=Cl",		"&=Cl",		"&=ml",
	"&=yl",		"&=Gb",		"&=Bl",		"&=Bl",
	"&=Bl",		"&=gl",		"&=gl",		"&=gl",
	"&=gl",		"&=gl",		"&=lr",		"&=lr",
	"&=Ml",		"&=Ml",		"&=Ml",		"&=Ml",
	"&=Ml",		"&=Ml",		"&=Ml",		"&=Ml",
	"&=Gc",		"&=Yl",		"&=Yl",		"&=Wl",
	"&=Wl",		"&=Wl",		"&=Wl",		"&=Rl",
	"&=Rl",		"&=Rl",		"&=Rl",		"&=Yl",
	"&=wb",		"&=wb",		"&=cl",		"&=lw",
	"&=Rb",		"&=Yb"
};
	
#endif
