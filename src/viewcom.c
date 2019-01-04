
/* Viewcom
 * 1995 by Illusion
 * -----------------
 * Views the input handlers of on-line players. If the input handler
 * is the command parse, the last command the user entered will be
 * displayed.
 */

#include "kernel.h"
#include "viewcom.h"
#include "commands.h"
#include "wizard.h"
#include "mud.h"
#include "uaf.h"
#include "writer.h"
#include "mailer.h"
#include "change.h"
#include "frob.h"
#include "bprintf.h"
#include "timing.h"
#include "pflags.h"
#include "mobile.h"

void
viewcom (void)
{
  int i;
  char idle[64];

  if (!ptstflg (mynum, PFL_VIEWCOM)) {
    bprintf ("Pardon?\n");
    return;
  }
  bprintf ("&+CUser               Idle  Handler\n");
  bprintf ("&+B-------------------------------------------------------------------------------\n");
  for (i = 0; i < max_players; ++i) {
    if (see_player (mynum, i) && is_conn (i)) {

      if (ptstflg (mynum, PFL_SEEIDLE))
	sprintf (idle, sec_to_hhmmss (global_clock - prlast_cmd (i)));
      else
	sprintf (idle, sec_to_hhmmss (global_clock - plast_cmd (i)));

      bprintf ("&+w%-14.14s %8.8s  ", pname (i), idle);

      if (phandler (i) == get_command) {
	if (wlevel (plev (mynum)) >= wlevel (plev (i)))
	  bprintf ("get_command (Last Command: %-.26s)\n", plastcom (i));
	else
	  bprintf ("get_command (Last Command: Unavailable)\n");
      } else if (phandler (i) == help2)
	bprintf ("help2 (Help: Page 2)\n");
      else if (phandler (i) == help3)
	bprintf ("help3 (Help: Page 3)\n");
      else if (phandler (i) == help4)
	bprintf ("help4 (Help: Page 4)\n");
      else if (phandler (i) == help5)
	bprintf ("help5 (Help: Page 5)\n");
      else if (phandler (i) == help6)
	bprintf ("help6 (Help: Page 6)\n");

      else if (phandler (i) == mail_menu)
	bprintf ("mail_menu (Mail: Selection Menu)\n");
      else if (phandler (i) == mail_input)
	bprintf ("mail_input (Mail: Writing Message)\n");
      else if (phandler (i) == get_subject)
	bprintf ("get_subject (Mail: Entering Subject Name)\n");

      else if (phandler (i) == get_pname1)
	bprintf ("get_pname1 (Login: Getting Name)\n");
      else if (phandler (i) == get_pname2)
	bprintf ("get_pname2 (Login: Checking Name)\n");
      else if (phandler (i) == get_new_pass1)
	bprintf ("get_new_pass1 (Login: New Player: Getting Password)\n");
      else if (phandler (i) == get_new_pass2)
	bprintf ("get_new_pass2 (Login: New Player: Confirm Password)\n");
      else if (phandler (i) == get_passwd1)
	bprintf ("get_passwd1 (Login: Getting Password)\n");
      else if (phandler (i) == get_gender)
	bprintf ("get_gender (Login: New Player: Getting Gender)\n");
      else if (phandler (i) == kick_out_yn)
	bprintf ("kick_out_yn (Login: Confirm To Kill Other Session)\n");
      else if (phandler (i) == do_issue)
	bprintf ("do_issue (Login: Display Issue)\n");
      else if (phandler (i) == enter_vis)
	bprintf ("enter_vis (Login: Changing Vis Level)\n");
      else if (phandler (i) == do_motd)
	bprintf ("do_motd (Login: Display MOTD)\n");

      else if (phandler (i) == ask_old_passwd)
	bprintf ("ask_old_passwd (Passwd Change: Ask Old Password)\n");
      else if (phandler (i) == ask_new_passwd)
	bprintf ("ask_new_passwd (Passwd Change: Ask New Password)\n");
      else if (phandler (i) == ask_confirm_passwd)
	bprintf ("ask_confirm_passwd (Passwd Change: Confirm New Password)\n");

      else if (phandler (i) == unveilcom)
	bprintf ("unveilcom (Unveil Command)\n");
      else if (phandler (i) == becom)
	bprintf ("becom (Become Command)\n");
      else if (phandler (i) == frobcom)
	bprintf ("frobcom (Frob Command)\n");

      else if (phandler (i) == pager)
	bprintf ("pager (Using file pager)\n");

      else if (phandler (i) == klockcom)
	bprintf ("klockcom (Keyboard Locker)\n");

      else if (phandler (i) == write_handler) {
	WrHead *w = (WrHead *) players[i].writer;

	if (w->exit_handler == room_desc_handler)
	  bprintf ("room_desc_handler (Desc: Room)\n");
	if (w->exit_handler == mob_desc_handler)
	  bprintf ("mob_desc_handler (Desc: Mobile)\n");
	if (w->exit_handler == obj_desc_handler)
	  bprintf ("obj_desc_handler (Desc: Object)\n");
	if (w->exit_handler == player_desc_handler)
	  bprintf ("player_desc_handler (Desc: Player)\n");
      } else
	bprintf ("Unknown Handler\n");
    }
  }
  bprintf ("&+B-------------------------------------------------------------------------------\n");
}
