
#include "LINKCOUNT.H"
#include "kernel.h"
#include "bprintf.h"
#include "ver.h"

void
versioncom (void)
{
  bprintf ("&+C%s &+B(&*#%d&+B) [&*%s&+B]\n");
  bprintf ("&+CBuild Date: &*%s %s\n", __DATE__, __TIME__);
  bprintf ("&+C1994-1996&*, &+cIllusion &+B(&*shill@nyx.net&+B)\n");

#if defined (IS_ALPHA) || defined (IS_BETA)
  bprintf ("\nThis Code is Not a Public Release, Do Not Distribute\n");
#endif

#ifdef IS_BETA
  bprintf ("\nThis is a registered iDiRT Beta Testing site. To become a "
	   "Beta Test site,\nplease e-mail Illusion with your reason for "
	   "wanting to become one.\n");
#endif
}

int
linknumber (void)
{
  return LINKCOUNT;
}
