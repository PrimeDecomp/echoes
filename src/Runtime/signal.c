#include <critical_regions.h>
#include <signal.h>
#include <stdlib.h>

static __signal_func_ptr signal_funcs[6];

int raise(int sig) {
  __signal_func_ptr signal_func;

  if (sig < 1 || sig > 6) {
    return -1;
  }

  __begin_critical_region(signal_funcs_access);

  signal_func = signal_funcs[sig - 1];

  if (signal_func != SIG_IGN) {
    signal_funcs[sig - 1] = SIG_DFL;
  }

  __end_critical_region(signal_funcs_access);

  if (signal_func == SIG_IGN || (signal_func == SIG_DFL && sig == SIGABRT)) {
    return 0;
  }

  if (signal_func == SIG_DFL) {
    exit(0);
  }

  (*signal_func)(sig);
  return 0;
}
