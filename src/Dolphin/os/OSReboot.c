#include "dolphin/dvd.h"
#include "dolphin/os.h"
#include "dolphin/os/OSBootInfo.h"

extern OSExecParams __OSRebootParams;
void __OSBootDol(u32 doloffset, u32 restartCode, const char** argv);

static void* SaveStart;
static void* SaveEnd;

void __OSReboot(u32 resetCode, u32 bootDol) {
  OSContext exceptionContext;
  char* argvToPass;

  OSDisableInterrupts();
  OSSetArenaLo((void*)0x81280000);
  OSSetArenaHi((void*)0x812f0000);
  OSClearContext(&exceptionContext);
  OSSetCurrentContext(&exceptionContext);
  argvToPass = NULL;
  __OSBootDol(bootDol, resetCode | 0x80000000, &argvToPass);
}

void OSSetSaveRegion(void* start, void* end) {
  SaveStart = start;
  SaveEnd = end;
}

void OSGetSaveRegion(void** start, void** end) {
  *start = SaveStart;
  *end = SaveEnd;
}

void OSGetSavedRegion(void** start, void** end) {
  *start = __OSRebootParams.regionStart;
  *end = __OSRebootParams.regionEnd;
}
