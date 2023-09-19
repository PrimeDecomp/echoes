#ifndef _REL_SETUP
#define _REL_SETUP

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void _prolog();
void _epilog();
void _unresolved();

void ModuleConstructors(void);
void ModuleDestructors(void);

// User functions
void RELMain(void);
void RELExit(void);

#ifdef __MWERKS__
#define REL_EXPORT __declspec(export)
#else
#define REL_EXPORT
#endif

#ifdef __cplusplus
}
#endif

#endif // _REL_SETUP
