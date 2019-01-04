#ifndef _MAILER_H
#define _MAILER_H

/** Macros **************************************************************
 *									*
 ************************************************************************/
#define	msg_from(C)		(players[C].Mailer.message.from)
#define	msg_subject(C)		(players[C].Mailer.message.subject)
#define	msg_date(C)		(players[C].Mailer.message.date)
#define	msg_text(C)		(players[C].Mailer.message.msgtxt)

#define	msg_idx(C, P)		(players[C].Mailer.msgidx[P])
#define	msg_idx_offset(C, P)	(players[C].Mailer.msgidx[P].offset)
#define	msg_idx_delete(C, P)	(players[C].Mailer.msgidx[P].delete)

/** Defines *************************************************************
 *									*
 ************************************************************************/
#define	MAILVERSION	"2.02"
#define	MAIL_HELP	"Mail.Help"

#define	EOM_MARKER	'\005'	/* Can only be ONE character in length	*/
#define	READ		0
#define	WRITE		1

#define INPUT_INFO	"\
&+B------------------------------------------------------------------------------\n\
&+CCommands: \
&+B[&+W=s &*or &+W** &+c: &+wSave&+B] \
&+B[&+W=a &*or &+W*abort &+c: &+wAbort&+B] \
&+B[&+W!command &+c: &+wMUD Command&+B]\n\
&+B------------------------------------------------------------------------------\n"

#define	FWD_NOTE	"(-------- Forwarded Message --------)"
#define	NEW_MAIL_MSG	"&+B[&+CNew Mail Has Arrived In Your Mailbox&+B]"
#define	SENT_MESSAGE	"&+B[&+CMessage Has Been Sent&+B]"
#define	SENT_FORWARD	"&+B[&+CMessage Has Been Forwarded&+B]"
#define	MENU_PROMPT	"&+B[&*Mail&+B]&+C: &*"
#define	SUBJECT_PROMPT	"&+CSubject: &*"
#define	INPUT_PROMPT	"&+W:&+C> &*"

/** Prototypes **********************************************************
 *									*
 ************************************************************************/
void	mailcom (void);
void	initialize_mailer (void);
void	check_mail (char *name);
Boolean	open_mailbox (char *name, short mode);
void	mail_headers (void);
void	mail_menu (char *Choice);
char	*make_mailtime (time_t tm_t);
void	read_msgidx (void);
void	quit_mailer (void);
void	read_message (int number);
void	send_mail (char *send_to);
void	get_subject (char *subject);
void	mail_input (char *input);
Boolean	append_file (char *dest, char *src);
void	send_forward (char *param);
void	send_reply (int num);
void	reindex_mail (int plr);

#endif
