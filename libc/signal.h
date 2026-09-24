#ifndef _SIGNAL_H
#define _SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*__signal_func_ptr)(int);

#define SIG_DFL ((__signal_func_ptr)0)
#define SIG_IGN ((__signal_func_ptr)1)

#define SIGABRT 1
#define SIGFPE 2
#define SIGILL 3
#define SIGINT 4
#define SIGSEGV 5
#define SIGTERM 6

int raise(int sig);

#ifdef __cplusplus
}
#endif

#endif
