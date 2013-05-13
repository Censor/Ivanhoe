
#include "RobboLito.h"

#ifdef LINUX_LARGE_PAGES
#include <signal.h>
static MUTEX_TYPE DELETION_LOCK[1];
void SIGNAL_HANDLE (int signo)
{
  struct sigaction sig_action[1];
  LOCK (DELETION_LOCK);
  SHARED_DELETE (); /* accrual */
  sig_action->sa_handler = SIG_DFL; /* HACK */
  sigaction (signo, sig_action, NULL);;
  UNLOCK (DELETION_LOCK);
  raise (signo);
}

void LINUX_handle_signals ()
{
  struct sigaction sig_action[1];
  sig_action->sa_handler = SIGNAL_HANDLE;
  sigaction (SIGABRT, sig_action, NULL);
  sigaction (SIGHUP, sig_action, NULL);
  sigaction (SIGINT, sig_action, NULL);
  sigaction (SIGSEGV, sig_action, NULL);
  sigaction (SIGTERM, sig_action, NULL);
  LOCK_INIT (DELETION_LOCK);
}
#endif
