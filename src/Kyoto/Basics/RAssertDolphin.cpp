#include "Kyoto/Basics/RAssertDolphin.hpp"

#include "dolphin/__start.h"
#include "dolphin/ai.h"
#include "dolphin/os/OSMemory.h"
#include "dolphin/pad.h"
#include "dolphin/vi.h"


#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern "C" void fn_803816B4(int);
extern "C" void fn_80362EC4(void*);
extern "C" void fn_80377118(int);

static const char* buildTime = "Build v1.088 10/29/2002 2:21:25";
static char* rs_debugger_buffer;
static int lbl_80419820;
static bool lbl_80419818;

#define COMPARE_TIME(current, last) (current / last) > 1.0

void ErrorHandler(OSError code, OSContext* context, int dsisr, uint dar) {

  uint loopExitCriteria;
  OSContext newContext;
  PADStatus pads[4];
  uint local_60;

  if (code != OS_ERROR_PROTECTION || ((dsisr & 0x10) == 0 && (dar < 0x1800001))) {
    OSProtectRange(3, 0, 0, 3);
    memset((void*)0x81200000, 0, 0x280);
    DCFlushRange((void*)0x81200000, 0x280);
    fn_803816B4(0);
    AIInitDMA(0x81200000, 0x280);
    AIStartDMA();
    fn_80362EC4(nullptr);
    loopExitCriteria = 0;
    PADControlMotor(0, 0);
    VISetBlack(1);
    VIFlush();
    OSClearContext(&newContext);
    OSSetCurrentContext(&newContext);
    OSEnableInterrupts();

    OSTime lastTime = OSGetTime();
    OSTime currentTime;

    while (loopExitCriteria < 4) {

      currentTime = OSGetTime();
      if (COMPARE_TIME(currentTime, lastTime)) {
        OSResetSystem(1, 0, 0);
      }

      PADRead(pads);
      while (pads[1].err == -1) {
        currentTime = OSGetTime();
        if (COMPARE_TIME(currentTime, lastTime)) {
          break;
        }
        fn_80377118(0x40000000);
        PADRead(pads);
      }

      if (pads[1].err == 0) {
        local_60 = 0x2010408;
        if ((pads[1].button & 0xc10) == 0xc10) {
          lastTime = OSGetTime();
          if ((pads[1].button & 0xf) != 0) {
            if ((pads[1].button & loopExitCriteria) ==
                0) { // TODO: wrong logic with loopExitCriteria
              if ((loopExitCriteria != 0) &&
                  pads[1].button == 0) { // TODO: wrong logic with pads[1].button
                loopExitCriteria = 0;
              }
            } else {
              loopExitCriteria += 1;
            }
          }
        } else {
          loopExitCriteria = 0;
        }
      }
    }
    VISetBlack(0);
    VIFlush();
    OSReport("Unhandled exception %d - Production\n", code);
    OSReport("%s\n", buildTime);
    OSReport("------------------------- Context 0x%08x -------------------------\n", context);

    int i = 0;
    do {
      OSReport("r%-2d  = 0x%08x (%14d)  r%-2d  = 0x%08x (%14d)\n", i, context->gpr[i],
               context->gpr[i], i + 0x10, context->gpr[i + 0x10], context->gpr[i + 0x10]);
      i += 1;
    } while (i < 0x10);

    OSReport("LR   = 0x%08x                   CR   = 0x%08x\n", context->lr, context->cr);
    OSReport("SRR0 = 0x%08x                   SRR1 = 0x%08x\n", context->srr0, context->srr1);
    OSReport("DSISR= 0x%08x                   DAR  = 0x%08x\n", dsisr, dar);

    char buffer[1024];

    if (rs_debugger_buffer == nullptr) {
      buffer[0] = 0;
    } else {
      strcpy(buffer, rs_debugger_buffer);
    }

    int bufferSize = strlen(buffer);
    if (bufferSize == 0) {
      i = sprintf(buffer, "Exception %d - Production\n", code);
    } else {
      i = sprintf(buffer + bufferSize, " - Production\n");
    }
    int j = sprintf(buffer + bufferSize + i, "%s\n", buildTime);
    int bufferIndex = bufferSize + i + j;
    bufferIndex += sprintf(buffer + bufferIndex, "IP: 0x%8.8x  Mem: 0x%8.8x", context->srr0, dar);
    if (code == 0xf) {
      bufferIndex += sprintf(buffer + bufferIndex, " - %d", dsisr);
    }
    bufferIndex += sprintf(buffer + bufferIndex, "\n\n");

    OSReport("\nAddress:      Back Chain    LR Save\n");

    uint stackPtr = context->gpr[1];
    bool check;
    i = 0;
    while (true) {
      if (((stackPtr == NULL) || (stackPtr == 0xffffffff)) || (check = 0xf < i, i += 1, check)) {
        break;
      }
      if ((stackPtr < 0x80000401) || (0x817fffff < stackPtr)) {
        OSReport("0x%08x:   BAD PTR\n", stackPtr);
        sprintf(buffer + j, "0x%08x: BAD PTR\n", stackPtr);
        break;
      }
      uint* asPtr = (uint*)stackPtr;

      OSReport("0x%08x:   0x%08x    0x%08x\n", stackPtr, *asPtr, asPtr[1]);
      bufferIndex += sprintf(buffer + j, "0x%08x: 0x%08x 0x%08x\n", stackPtr, *asPtr, asPtr[1]);
      stackPtr = *asPtr;
    }
    OSReport("\nInstruction at 0x%x (read from SRR0) attempted to access invalid address 0x%x "
             "(read from DAR)\n",
             context->srr0, dar);

    if (!lbl_80419818) {
      GXColor fg; // = &DAT_80000000;
      GXColor bg; // = 0xffffff00;
      OSFatal(fg, bg, buffer);
    }
  }
}

void SetErrorHandlers() {
  OSSetErrorHandler(OS_ERROR_DSI, (OSErrorHandler)ErrorHandler);
  OSSetErrorHandler(OS_ERROR_ISI, (OSErrorHandler)ErrorHandler);
  OSSetErrorHandler(OS_ERROR_ALIGNMENT, (OSErrorHandler)ErrorHandler);
  OSSetErrorHandler(OS_ERROR_PROTECTION, (OSErrorHandler)ErrorHandler);
}

void rs_debugger_printf(const char* format, ...) {
  va_list arg;

  OSProtectRange(3, 0, 0, 3);
  lbl_80419820 = 0;
  if (strncmp(format, "SKIP", 4) == 0) {
    const char* s = format + 4;
    format += 5;
    lbl_80419820 = *s - 0x30;
  }

  char buffer[0x400];

  va_start(arg, format);
  vsnprintf(buffer, 0x400, format, arg);
  va_end(arg);
  rs_debugger_buffer = buffer;

  OSContext context;
  OSSaveContext(&context);
  ErrorHandler(0xff, &context, 0, 0xd1dd0d1e);
}
