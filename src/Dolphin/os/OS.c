
#include "dolphin/os.h"
#include "dolphin/DVDPriv.h"
#include "dolphin/db.h"
#include "dolphin/os/OSBootInfo.h"

extern OSTime __OSGetSystemTime();
static const char* __OSVersion =
    "<< Dolphin SDK - OS\trelease build: May 21 2004 09:28:09 (0x2301) >>";
extern char _db_stack_end[];

#define OS_BI2_DEBUG_ADDRESS 0x800000F4
#define DEBUGFLAG_ADDR 0x800030E8
#define OS_DEBUG_ADDRESS_2 0x800030E9
#define OS_CURRENTCONTEXT_PADDR 0x00C0

extern char* __OSResetSWInterruptHandler[];

vu16 __OSDeviceCode : (OS_BASE_CACHED | 0x30E6);
static DVDDriveInfo DriveInfo ATTRIBUTE_ALIGN(32);
static DVDCommandBlock DriveBlock;

static OSBootInfo* BootInfo;
static u32* BI2DebugFlag;
static u32 BI2DebugFlagHolder;
__declspec(weak) BOOL __OSIsGcam = FALSE;
static f64 ZeroF;
static f32 ZeroPS[2];
static BOOL AreWeInitialized = FALSE;
static __OSExceptionHandler* OSExceptionTable;
OSTime __OSStartTime;
BOOL __OSInIPL;
OSExecParams __OSRebootParams;

extern u8 __ArenaHi[];
extern u8 __ArenaLo[];
extern u32 __DVDLongFileNameFlag;
extern u32 __PADSpec;

#define OS_EXCEPTIONTABLE_ADDR 0x3000
#define OS_DBJUMPPOINT_ADDR 0x60
// memory locations for important stuff
#define OS_CACHED_REGION_PREFIX 0x8000
#define OS_BI2_DEBUG_ADDRESS 0x800000F4
#define OS_BI2_DEBUGFLAG_OFFSET 0xC
#define PAD3_BUTTON_ADDR 0x800030E4
#define OS_DVD_DEVICECODE 0x800030E6
#define DEBUGFLAG_ADDR 0x800030E8
#define OS_DEBUG_ADDRESS_2 0x800030E9
#define DB_EXCEPTIONRET_OFFSET 0xC
#define DB_EXCEPTIONDEST_OFFSET 0x8
#define MSR_RI_BIT 0x1E

void OSDefaultExceptionHandler(__OSException exception, OSContext* context);
extern BOOL __DBIsExceptionMarked(__OSException);
static void OSExceptionInit(void);
void __OSGetExecParams(OSExecParams* params);
void EnableMetroTRKInterrupts(void);
extern char _stack_addr[];

static void DisableWriteGatherPipe(void) {
  u32 hid2;

  hid2 = PPCMfhid2();
  hid2 &= ~0x40000000;
  PPCMthid2(hid2);
}

/* clang-format off */
asm void __OSFPRInit(void)
{
    nofralloc

    mfmsr   r3
    ori     r3, r3, 0x2000
    mtmsr   r3

    mfspr   r3, 0x398
    rlwinm. r3, r3, 3, 31, 31
    beq     SkipPairedSingles

    lis     r3, ZeroPS@ha
    addi    r3, r3, ZeroPS@l
    psq_l   fp0, 0(r3), 0, 0
    ps_mr   fp1, fp0
    ps_mr   fp2, fp0
    ps_mr   fp3, fp0
    ps_mr   fp4, fp0
    ps_mr   fp5, fp0
    ps_mr   fp6, fp0
    ps_mr   fp7, fp0
    ps_mr   fp8, fp0
    ps_mr   fp9, fp0
    ps_mr   fp10, fp0
    ps_mr   fp11, fp0
    ps_mr   fp12, fp0
    ps_mr   fp13, fp0
    ps_mr   fp14, fp0
    ps_mr   fp15, fp0
    ps_mr   fp16, fp0
    ps_mr   fp17, fp0
    ps_mr   fp18, fp0
    ps_mr   fp19, fp0
    ps_mr   fp20, fp0
    ps_mr   fp21, fp0
    ps_mr   fp22, fp0
    ps_mr   fp23, fp0
    ps_mr   fp24, fp0
    ps_mr   fp25, fp0
    ps_mr   fp26, fp0
    ps_mr   fp27, fp0
    ps_mr   fp28, fp0
    ps_mr   fp29, fp0
    ps_mr   fp30, fp0
    ps_mr   fp31, fp0

SkipPairedSingles:
    lfd     fp0, ZeroF
    fmr     fp1, fp0
    fmr     fp2, fp0
    fmr     fp3, fp0
    fmr     fp4, fp0
    fmr     fp5, fp0
    fmr     fp6, fp0
    fmr     fp7, fp0
    fmr     fp8, fp0
    fmr     fp9, fp0
    fmr     fp10, fp0
    fmr     fp11, fp0
    fmr     fp12, fp0
    fmr     fp13, fp0
    fmr     fp14, fp0
    fmr     fp15, fp0
    fmr     fp16, fp0
    fmr     fp17, fp0
    fmr     fp18, fp0
    fmr     fp19, fp0
    fmr     fp20, fp0
    fmr     fp21, fp0
    fmr     fp22, fp0
    fmr     fp23, fp0
    fmr     fp24, fp0
    fmr     fp25, fp0
    fmr     fp26, fp0
    fmr     fp27, fp0
    fmr     fp28, fp0
    fmr     fp29, fp0
    fmr     fp30, fp0
    fmr     fp31, fp0

    mtfsf   0xFF, fp0

    blr
}
/* clang-format on */

u32 OSGetConsoleType() {
  if (BootInfo == NULL || BootInfo->consoleType == 0) {
    return OS_CONSOLE_ARTHUR;
  }
  return BootInfo->consoleType;
}

static void ClearArena(void) {
  if (!((OSGetResetCode() & 0x80000000) ? TRUE : FALSE)) {
    memset(OSGetArenaLo(), 0, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
    return;
  }

  if (*(u32*)&__OSRebootParams.regionStart == 0) {
    memset(OSGetArenaLo(), 0, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
    return;
  }

  if ((u32)OSGetArenaLo() < *(u32*)&__OSRebootParams.regionStart) {
    if ((u32)OSGetArenaHi() <= *(u32*)&__OSRebootParams.regionStart) {
      memset(OSGetArenaLo(), 0, (u32)OSGetArenaHi() - (u32)OSGetArenaLo());
      return;
    }

    memset(OSGetArenaLo(), 0, *(u32*)&__OSRebootParams.regionStart - (u32)OSGetArenaLo());

    if ((u32)OSGetArenaHi() > (u32)__OSRebootParams.regionEnd) {
      memset(__OSRebootParams.regionEnd, 0,
             (u32)OSGetArenaHi() - (u32)__OSRebootParams.regionEnd);
    }
  }
}

static void InquiryCallback(s32 result, DVDCommandBlock* block) {
  switch (block->state) {
  case 0:
    __OSDeviceCode = (u16)(0x8000 | DriveInfo.deviceCode);
    break;
  default:
    __OSDeviceCode = 1;
    break;
  }
}

void OSInit(void) {
  u32 consoleType;
  void* bi2StartAddr;

  if (AreWeInitialized == FALSE) {
    AreWeInitialized = TRUE;

    __OSStartTime = __OSGetSystemTime();
    OSDisableInterrupts();

    __OSGetExecParams(&__OSRebootParams);
    PPCMtmmcr0(0);
    PPCMtmmcr1(0);
    PPCMtpmc1(0);
    PPCMtpmc2(0);
    PPCMtpmc3(0);
    PPCMtpmc4(0);
    PPCDisableSpeculation();
    PPCSetFpNonIEEEMode();

    BootInfo = (OSBootInfo*)OSPhysicalToCached(0);
    BI2DebugFlag = 0;
    __DVDLongFileNameFlag = 0;

    bi2StartAddr = (void*)(*(u32*)OSPhysicalToCached(0xF4));
    if (bi2StartAddr) {
      BI2DebugFlag = (void*)((char*)bi2StartAddr + 0xC);
      __PADSpec = ((u32*)bi2StartAddr)[9];
      *(u8*)OSPhysicalToCached(0x30E8) = *BI2DebugFlag;
      *(u8*)OSPhysicalToCached(0x30E9) = __PADSpec;
    } else if (BootInfo->arenaHi) {
      BI2DebugFlagHolder = *(u8*)OSPhysicalToCached(0x30E8);
      BI2DebugFlag = &BI2DebugFlagHolder;
      __PADSpec = *(u8*)OSPhysicalToCached(0x30E9);
    }

    __DVDLongFileNameFlag = 1;

    OSSetArenaLo((!BootInfo->arenaLo) ? &__ArenaLo : BootInfo->arenaLo);
    if ((!BootInfo->arenaLo) && (BI2DebugFlag) && (*(u32*)BI2DebugFlag < 2)) {
      OSSetArenaLo((void*)(((u32)(char*)&_stack_addr + 0x1F) & 0xFFFFFFE0));
    }
    OSSetArenaHi((!BootInfo->arenaHi) ? &__ArenaHi : BootInfo->arenaHi);

    OSExceptionInit();
    __OSInitSystemCall();
    OSInitAlarm();
    __OSModuleInit();
    __OSInterruptInit();
    __OSSetInterruptHandler(__OS_INTERRUPT_PI_RSW, (void*)__OSResetSWInterruptHandler);
    __OSContextInit();
    __OSCacheInit();
    EXIInit();
    SIInit();
    __OSInitSram();
    __OSThreadInit();
    __OSInitAudioSystem();

    DisableWriteGatherPipe();

    if (!__OSInIPL) {
      __OSInitMemoryProtection();
    }

    OSReport("\nDolphin OS\n");
    OSReport("Kernel built : %s %s\n", "May 21 2004", "09:28:09");
    OSReport("Console Type : ");

    consoleType = OSGetConsoleType();
    switch (consoleType & 0xF0000000) {
    case OS_CONSOLE_RETAIL:
      OSReport("Retail %d\n", consoleType);
      break;
    case OS_CONSOLE_DEVELOPMENT:
    case OS_CONSOLE_TDEV:
      switch (consoleType & 0x0FFFFFFF) {
      case OS_CONSOLE_EMULATOR:
        OSReport("Mac Emulator\n");
        break;
      case OS_CONSOLE_PC_EMULATOR:
        OSReport("PC Emulator\n");
        break;
      case OS_CONSOLE_ARTHUR:
        OSReport("EPPC Arthur\n");
        break;
      case OS_CONSOLE_MINNOW:
        OSReport("EPPC Minnow\n");
        break;
      default:
        OSReport("Development HW%d (%08x)\n", (consoleType & 0xFFFFFFF) - 3, consoleType);
        break;
      }
      break;
    default:
      OSReport("%08x\n", consoleType);
      break;
    }

    OSReport("Memory %d MB\n", (u32)BootInfo->memorySize >> 0x14U);
    OSReport("Arena : 0x%x - 0x%x\n", OSGetArenaLo(), OSGetArenaHi());
    OSRegisterVersion(__OSVersion);

    if (BI2DebugFlag && ((*BI2DebugFlag) >= 2)) {
      EnableMetroTRKInterrupts();
    }

    ClearArena();
    OSEnableInterrupts();

    if (!__OSInIPL) {
      DVDInit();

      if (__OSIsGcam) {
        __OSDeviceCode = 0x9000;
        return;
      }

      DCInvalidateRange(&DriveInfo, sizeof(DriveInfo));
      DVDInquiryAsync(&DriveBlock, &DriveInfo, InquiryCallback);
    }
  }
}

static u32 __OSExceptionLocations[] = {
    0x00000100, 0x00000200, 0x00000300, 0x00000400, 0x00000500, 0x00000600, 0x00000700, 0x00000800,
    0x00000900, 0x00000C00, 0x00000D00, 0x00000F00, 0x00001300, 0x00001400, 0x00001700,
};

// dummy entry points to the OS Exception vector
void __OSEVStart(void);
void __OSEVEnd(void);
void __OSEVSetNumber(void);
void __OSExceptionVector(void);

void __DBVECTOR(void);
void __OSDBINTSTART(void);
void __OSDBINTEND(void);
void __OSDBJUMPSTART(void);
void __OSDBJUMPEND(void);

#define NOP 0x60000000

__OSExceptionHandler __OSSetExceptionHandler(__OSException exception, __OSExceptionHandler handler);

/*
 * --INFO--
 * Address:	800EB654
 * Size:	000280
 */
static void OSExceptionInit(void) {
  __OSException exception;
  void* destAddr;

  // These two vars help us change the exception number embedded
  // in the exception handler code.
  u32* opCodeAddr;
  u32 oldOpCode;

  // Address range of the actual code to be copied.
  u8* handlerStart;
  u32 handlerSize;

  // Install the first level exception vector.
  opCodeAddr = (u32*)__OSEVSetNumber;
  oldOpCode = *opCodeAddr;
  handlerStart = (u8*)__OSEVStart;
  handlerSize = (u32)((u8*)__OSEVEnd - (u8*)__OSEVStart);

  // Install the DB integrator, only if we are the first OSInit to be run
  destAddr = (void*)OSPhysicalToCached(OS_DBJUMPPOINT_ADDR);
  if (*(u32*)destAddr == 0) // Lomem should be zero cleared only once by BS2
  {
    DBPrintf("Installing OSDBIntegrator\n");
    memcpy(destAddr, (void*)__OSDBINTSTART, (u32)__OSDBINTEND - (u32)__OSDBINTSTART);
    DCFlushRangeNoSync(destAddr, (u32)__OSDBINTEND - (u32)__OSDBINTSTART);
    __sync();
    ICInvalidateRange(destAddr, (u32)__OSDBINTEND - (u32)__OSDBINTSTART);
  }

  // Copy the right vector into the table
  for (exception = 0; exception < __OS_EXCEPTION_MAX; exception++) {
    if (BI2DebugFlag && (*BI2DebugFlag >= 2) && __DBIsExceptionMarked(exception)) {
      // this DBPrintf is suspicious.
      DBPrintf(">>> OSINIT: exception %d commandeered by TRK\n", exception);
      continue;
    }

    // Modify the copy of code in text before transferring
    // to the exception table.
    *opCodeAddr = oldOpCode | exception;

    // Modify opcodes at __DBVECTOR if necessary
    if (__DBIsExceptionMarked(exception)) {
      DBPrintf(">>> OSINIT: exception %d vectored to debugger\n", exception);
      memcpy((void*)__DBVECTOR, (void*)__OSDBJUMPSTART, (u32)__OSDBJUMPEND - (u32)__OSDBJUMPSTART);
    } else {
      // make sure the opcodes are still nop
      u32* ops = (u32*)__DBVECTOR;
      int cb;

      for (cb = 0; cb < (u32)__OSDBJUMPEND - (u32)__OSDBJUMPSTART; cb += sizeof(u32)) {
        *ops++ = NOP;
      }
    }

    // Install the modified handler.
    destAddr = (void*)OSPhysicalToCached(__OSExceptionLocations[(u32)exception]);
    memcpy(destAddr, handlerStart, handlerSize);
    DCFlushRangeNoSync(destAddr, handlerSize);
    __sync();
    ICInvalidateRange(destAddr, handlerSize);
  }

  // initialize pointer to exception table
  OSExceptionTable = OSPhysicalToCached(OS_EXCEPTIONTABLE_ADDR);

  // install default exception handlers
  for (exception = 0; exception < __OS_EXCEPTION_MAX; exception++) {
    __OSSetExceptionHandler(exception, OSDefaultExceptionHandler);
  }

  // restore the old opcode, so that we can re-start an application without
  // downloading the text segments
  *opCodeAddr = oldOpCode;

  DBPrintf("Exceptions initialized...\n");
}

static asm void __OSDBIntegrator(void) {
  /* clang-format off */
    nofralloc
entry __OSDBINTSTART
    li      r5, OS_DBINTERFACE_ADDR
    mflr    r3
    stw     r3, DB_EXCEPTIONRET_OFFSET(r5)
    lwz     r3, DB_EXCEPTIONDEST_OFFSET(r5)
    oris    r3, r3, OS_CACHED_REGION_PREFIX
    mtlr    r3
    li      r3, 0x30 // MSR_IR | MSR_DR     // turn on memory addressing
    mtmsr   r3
    blr
entry __OSDBINTEND
  /* clang-format on */
}

static asm void __OSDBJump(void){
    /* clang-format off */

    nofralloc
entry __OSDBJUMPSTART
    bla     OS_DBJUMPPOINT_ADDR
entry __OSDBJUMPEND
    /* clang-format on */

} 

__OSExceptionHandler
    __OSSetExceptionHandler(__OSException exception, __OSExceptionHandler handler) {
  __OSExceptionHandler oldHandler;
  oldHandler = OSExceptionTable[exception];
  OSExceptionTable[exception] = handler;
  return oldHandler;
}

__OSExceptionHandler __OSGetExceptionHandler(__OSException exception) {
  return OSExceptionTable[exception];
}

static asm void OSExceptionVector(void) {
  /* clang-format off */
    nofralloc

entry __OSEVStart
    // Save r4 into SPRG0
    mtsprg  0, r4

    // Load current context physical address into r4
    lwz     r4, OS_CURRENTCONTEXT_PADDR

    // Save r3 - r5 into the current context
    stw     r3, OS_CONTEXT_R3(r4)
    mfsprg  r3, 0
    stw     r3, OS_CONTEXT_R4(r4)
    stw     r5, OS_CONTEXT_R5(r4)

    lhz     r3, OS_CONTEXT_STATE(r4)
    ori     r3, r3, OS_CONTEXT_STATE_EXC
    sth     r3, OS_CONTEXT_STATE(r4)

    // Save misc registers
    mfcr    r3
    stw     r3, OS_CONTEXT_CR(r4)
    mflr    r3
    stw     r3, OS_CONTEXT_LR(r4)
    mfctr   r3
    stw     r3, OS_CONTEXT_CTR(r4)
    mfxer   r3
    stw     r3, OS_CONTEXT_XER(r4)
    mfsrr0  r3
    stw     r3, OS_CONTEXT_SRR0(r4)
    mfsrr1  r3
    stw     r3, OS_CONTEXT_SRR1(r4)
    mr      r5, r3

entry __DBVECTOR
    nop

    // Set SRR1[IR|DR] to turn on address
    // translation at the next RFI
    mfmsr   r3
    ori     r3, r3, 0x30
    mtsrr1  r3

    // This lets us change the exception number based on the
    // exception we're installing.
entry __OSEVSetNumber
    addi    r3, 0, 0x0000

    // Load current context virtual address into r4
    lwz     r4, 0xD4

    // Check non-recoverable interrupt
    rlwinm. r5, r5, 0, MSR_RI_BIT, MSR_RI_BIT
    bne     recoverable
    addis   r5, 0,  OSDefaultExceptionHandler@ha
    addi    r5, r5, OSDefaultExceptionHandler@l
    mtsrr0  r5
    rfi
    // NOT REACHED HERE

recoverable:
    // Locate exception handler.
    rlwinm  r5, r3, 2, 22, 29               // r5 contains exception*4
    lwz     r5, OS_EXCEPTIONTABLE_ADDR(r5)
    mtsrr0  r5

    // Final state
    // r3 - exception number
    // r4 - pointer to context
    // r5 - garbage
    // srr0 - exception handler
    // srr1 - address translation enalbed, not yet recoverable

    rfi
    // NOT REACHED HERE
    // The handler will restore state

entry __OSEVEnd
    nop
  /* clang-format on */
}

void __OSUnhandledException(__OSException exception, OSContext* context, u32 dsisr, u32 dar);
asm void OSDefaultExceptionHandler(register __OSException exception, register OSContext* context) {
  /* clang-format off */
    nofralloc
    OS_EXCEPTION_SAVE_GPRS(context)
    mfdsisr r5
    mfdar   r6

    stwu    r1,-8(r1)
    b       __OSUnhandledException
    /* clang-foramt on */
}

void __OSPSInit(void)
{
	PPCMthid2(PPCMfhid2() | 0xA0000000);
	ICFlashInvalidate();
	__sync();
	// clang-format off
    asm
    {
        li      r3, 0
        mtspr   GQR0, r3
        mtspr   GQR1, r3
        mtspr   GQR2, r3
        mtspr   GQR3, r3
        mtspr   GQR4, r3
        mtspr   GQR5, r3
        mtspr   GQR6, r3
        mtspr   GQR7, r3
    }
  // clang-format on
}

#define DI_CONFIG_IDX 0x9
#define DI_CONFIG_CONFIG_MASK 0xFF
u32 __OSGetDIConfig(void) { return (__DIRegs[DI_CONFIG_IDX] & DI_CONFIG_CONFIG_MASK); }

void OSRegisterVersion(const char* id) { OSReport("%s\n", id); }
