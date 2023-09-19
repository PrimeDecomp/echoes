#include "REL/REL_Setup.h"
#include "dolphin/os.h"

#ifdef __MWERKS__
#define REL_EXTERN __declspec(section ".init") extern
#else
#define REL_EXTERN extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*VoidFunc)(void);
REL_EXTERN VoidFunc _ctors[];
REL_EXTERN VoidFunc _dtors[];

#ifdef __cplusplus
}
#endif

void ModuleConstructors(void) {
  VoidFunc* func;
  for (func = _ctors; *func != nullptr; ++func) {
    (*func)();
  }
}

void ModuleDestructors(void) {
  VoidFunc* func;
  for (func = _dtors; *func != nullptr; ++func) {
    (*func)();
  }
}

REL_EXPORT void _prolog(void) {
  ModuleConstructors();
  RELMain();
}

REL_EXPORT void _epilog(void) {
  ModuleDestructors();
  RELExit();
}

REL_EXPORT void _unresolved(void) {
  u32 i, s;

  OSReport("\nError: Unlinked function called in module %s.\n", __FILE__);
  OSReport("Address:      Back Chain    LR Save\n");

  for (i = 0, s = OSGetStackPointer(); s != 0 && s != 0xFFFFFFFF && i++ < 16;
       s = *(const u32*)(s)) {
    const u32* p = (const u32*)(s);
    OSReport("0x%08x:   0x%08x    0x%08x\n", p, p[0], p[1]);
  }

  OSReport("\n");
}
