#include "dolphin/DVDPriv.h"
#include "dolphin/ai.h"
#include "dolphin/dvd.h"
#include "dolphin/os.h"
#include "dolphin/os/OSBootInfo.h"
#include "string.h"

typedef int (*appGetNextCallback)(void*, u32*, u32*);
typedef void (*appInitCallback)(void (*)(char*));
typedef void* (*appGetEntryCallback)();
typedef void (*AppLoaderCallback)(appInitCallback*, appGetNextCallback*, appGetEntryCallback*);

OSExecParams* __OSExecParams : (OS_BASE_CACHED | 0x30F0);
s32 __OSAppLoaderOffset : (OS_BASE_CACHED | 0x30F4);

extern volatile u32 BOOT_REGION_START : 0x812FDFF0;
extern volatile u32 BOOT_REGION_END : 0x812FDFEC;
extern volatile u8 g_unk_800030E2 : 0x800030E2;



BOOL DVDReadAbsAsyncPrio(DVDCommandBlock* block, void* addr, s32 length, s32 offset,
                         DVDCBCallback callback, s32 prio);
BOOL DVDCheckDisk(void);
BOOL DVDSetAutoInvalidation(BOOL autoInval);
void DVDResume(void);
BOOL __DVDPrepareResetAsync(DVDCBCallback callback);
void __OSDoHotReset(u32 resetCode);
void OSGetSaveRegion(void** start, void** end);
int sprintf(char* s, const char* format, ...);

void __OSSetExecParams(const OSExecParams* params, OSExecParams* addr);

extern BOOL __OSIsGcam;

static BOOL Prepared;

static int PackArgs(void* addr, s32 argc, char** argv) {
  s32 numArgs;
  char* bootInfo2;
  char* ptr;
  char** list;
  u32 i;

  bootInfo2 = (char*)addr;
  memset(bootInfo2, 0, 0x2000);

  if (argc == 0) {
    *(u32*)(bootInfo2 + 8) = 0;
  } else {
    numArgs = argc;
    ptr = bootInfo2 + 0x2000;
    while (--argc >= 0) {
      ptr -= strlen(argv[argc]) + 1;
      strcpy(ptr, argv[argc]);
      argv[argc] = (char*)(ptr - bootInfo2);
    }

    ptr = bootInfo2 + ((ptr - bootInfo2) & ~0x3);
    ptr -= ((numArgs + 1) * 4);
    list = (char**)ptr;

    for (i = 0; i < numArgs + 1; i++) {
      list[i] = argv[i];
    }

    ptr -= 4;
    *(u32*)ptr = numArgs;

    *(u32*)(bootInfo2 + 8) = (ptr - bootInfo2);
  }

  return 1;
}

/* clang-format off */
static asm void Run(register void* entryPoint) {
  nofralloc

  mflr r0
  stw r0, 4(r1)
  stwu r1, -0x18(r1)
  stw r31, 0x14(r1)
  mr r31, entryPoint
  bl ICFlashInvalidate
  sync
  isync
  mtlr r31
  blr

  lwz r0, 0x1c(r1)
  lwz r31, 0x14(r1)
  addi r1, r1, 0x18
  mtlr r0
  blr
}
/* clang-format on */

static void StartDol(const OSExecParams* params, void* entry) {
  OSExecParams* paramsWork = OSAllocFromArenaLo(sizeof(OSExecParams), 1);

  __OSSetExecParams(params, paramsWork);
  __PIRegs[9] = 7;

  OSDisableInterrupts();
  Run(entry);
}

static void ReadDisc(void* addr, s32 length, s32 offset) {
  DVDCommandBlock block;

  DVDReadAbsAsyncPrio(&block, addr, length, offset, NULL, 0);

  while (DVDGetCommandBlockStatus(&block)) {
    if (!DVDCheckDisk()) {
      __OSDoHotReset(0);
    }
  }
}

static void Callback(s32 result, DVDCommandBlock* block) { Prepared = TRUE; }

static int IsStreamEnabled() {
  if (DVDGetCurrentDiskID()->streaming) {
    return TRUE;
  }

  return FALSE;
}

void __OSGetExecParams(OSExecParams* params) {
  if (0x80000000 <= (u32)__OSExecParams) {
    memcpy(params, __OSExecParams, sizeof(OSExecParams));
  } else {
    params->valid = FALSE;
  }
}

void __OSSetExecParams(const OSExecParams* params, OSExecParams* addr) {
  memcpy(addr, params, sizeof(OSExecParams));
  __OSExecParams = addr;
}

static void StopStreaming() {
  DVDCommandBlock block;

  if (!__OSIsGcam && IsStreamEnabled()) {
    AISetStreamVolLeft(0);
    AISetStreamVolRight(0);
    DVDCancelStreamAsync(&block, NULL);

    while (DVDGetCommandBlockStatus(&block)) {
      if (!DVDCheckDisk()) {
        __OSDoHotReset(0);
      }
    }

    AISetStreamPlayState(0);
  }
}

static int GetApploaderPosition(void) {
  static s32 apploaderPosition;

  u32* tgcHeader;
  s32 apploaderOffsetInTGC;

  if (apploaderPosition != 0) {
    return apploaderPosition;
  }

  if (__OSAppLoaderOffset != 0) {
    tgcHeader = OSAllocFromArenaLo(0x40, 32);
    ReadDisc(tgcHeader, 0x40, __OSAppLoaderOffset);
    apploaderOffsetInTGC = tgcHeader[14];

    apploaderPosition = __OSAppLoaderOffset + apploaderOffsetInTGC;
  } else {
    apploaderPosition = 0x2440;
  }
  return apploaderPosition;
}

typedef struct {
  char date[16];
  u32 entry;
  u32 size;
  u32 rebootSize;
  u32 reserved2;
} AppLoaderStruct;

static AppLoaderStruct* LoadApploader() {
  AppLoaderStruct* header;

  header = (AppLoaderStruct*)OSAllocFromArenaLo(sizeof(AppLoaderStruct), 32);
  ReadDisc(header, sizeof(AppLoaderStruct), GetApploaderPosition());

  ReadDisc((void*)0x81200000, OSRoundUp32B(header->size), GetApploaderPosition() + 0x20);
  ICInvalidateRange((void*)0x81200000, OSRoundUp32B(header->size));
  return header;
}

static void* LoadDol(const OSExecParams* params, AppLoaderCallback getInterface) {
  appInitCallback appInit;
  appGetNextCallback appGetNext;
  appGetEntryCallback appGetEntry;
  void* addr;
  u32 length;
  u32 offset;
  OSExecParams* paramsWork;

  getInterface(&appInit, &appGetNext, &appGetEntry);
  paramsWork = (OSExecParams*)OSAllocFromArenaLo(sizeof(OSExecParams), 1);
  __OSSetExecParams(params, paramsWork);
  appInit((void (*)(char*))OSReport);
  OSSetArenaLo(paramsWork);

  while (appGetNext(&addr, &length, &offset) != 0) {
    ReadDisc(addr, length, offset);
  }

  return appGetEntry();
}

static BOOL IsNewApploader(AppLoaderStruct* header) {
  return strncmp(header->date, "2004/02/01", 10) > 0 ? TRUE : FALSE;
}

void __OSBootDolSimple(u32 doloffset, u32 restartCode, void* regionStart, void* regionEnd,
                       BOOL argsUseDefault, s32 argc, char** argv) {
  OSExecParams* params;
  void* dolEntry;
  AppLoaderStruct* header;

  OSDisableInterrupts();
  params = (OSExecParams*)OSAllocFromArenaLo(sizeof(OSExecParams), 1);
  params->valid = TRUE;
  params->restartCode = restartCode;
  params->regionStart = regionStart;
  params->regionEnd = regionEnd;
  params->argsUseDefault = argsUseDefault;

  if (!argsUseDefault) {
    params->argsAddr = OSAllocFromArenaLo(0x2000, 1);
    PackArgs(params->argsAddr, argc, argv);
  }

  DVDInit();
  DVDSetAutoInvalidation(TRUE);
  DVDResume();

  Prepared = FALSE;

  __DVDPrepareResetAsync(Callback);
  __OSMaskInterrupts(0xFFFFFFE0);
  __OSUnmaskInterrupts(0x400);
  OSEnableInterrupts();

  while (Prepared != TRUE) {
    if (!DVDCheckDisk()) {
      __OSDoHotReset(0);
    }
  }

  StopStreaming();

  header = LoadApploader();
  if (IsNewApploader(header)) {
    if (doloffset == 0xFFFFFFFF) {
      doloffset = (GetApploaderPosition() + 0x20) + header->size;
    }

    params->bootDol = doloffset;
    dolEntry = LoadDol(params, (AppLoaderCallback)header->entry);
    StartDol(params, dolEntry);
  } else {
    BOOT_REGION_START = (u32)regionStart;
    BOOT_REGION_END = (u32)regionEnd;
    g_unk_800030E2 = 1;

    ReadDisc((void*)0x81300000, OSRoundUp32B(header->rebootSize),
             (GetApploaderPosition() + 0x20) + header->size);
    ICInvalidateRange((void*)0x81300000, OSRoundUp32B(header->rebootSize));
    OSDisableInterrupts();
    ICFlashInvalidate();
    Run((void*)0x81300000);
  }
}

void __OSBootDol(u32 doloffset, u32 restartCode, const char** argv) {
  char doloffInString[20];
  s32 argvlen;
  char** argvToPass;
  s32 i;
  void* saveStart;
  void* saveEnd;

  OSGetSaveRegion(&saveStart, &saveEnd);
  sprintf(doloffInString, "%d", doloffset);
  argvlen = 0;

  if (argv != 0) {
    while (argv[argvlen] != 0) {
      argvlen++;
    }
  }

  argvlen++;
  argvToPass = OSAllocFromArenaLo((argvlen + 1) * 4, 1);
  *argvToPass = doloffInString;

  for (i = 1; i < argvlen; i++) {
    argvToPass[i] = (char*)argv[i - 1];
  }

  __OSBootDolSimple(-1, restartCode, saveStart, saveEnd, FALSE, argvlen, argvToPass);
}
