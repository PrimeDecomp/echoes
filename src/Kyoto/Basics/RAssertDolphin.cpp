#include "Kyoto/Basics/RAssertDolphin.hpp"

#include "dolphin/ai.h"
#include "dolphin/dvd.h"
#include "dolphin/os/OSMemory.h"
#include "dolphin/os/OSReset.h"
#include "dolphin/pad.h"
#include "dolphin/vi.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern "C" void fn_803816B4(int);
extern "C" void fn_80362EC4(void*);
extern "C" void fn_80377118(int);
extern u32* fn_8033ED24(int);

static char* rs_debugger_buffer;
static int rs_debugger_size;
static bool SkipFatal = false;
static const GXColor bg = {128, 0, 0, 0};
static const GXColor fg = {255, 255, 255, 0};
static const uchar ExitButtons[4] = {PAD_BUTTON_RIGHT, PAD_BUTTON_LEFT, PAD_BUTTON_DOWN,
                                     PAD_BUTTON_UP};

static void hack() {
  static const char* tmp1 = "%s\0\n";
  static const char* tmp2 = "0x%08x: 0x%08x 0x%08x";
  static const char* tmp3 = " 0x%08x: %s";
}

void ErrorHandler(OSError code, OSContext* context, int dsisr, uint dar) {
  char buffer[1024];
  OSContext newContext;
  uint loopExitCriteria;
  PADStatus pads[4];
  uchar local_60[4];
  u32* tmp;
  uint i;
  u32* gpr;


  if (code == 15 && ((dsisr & 0x10) != 0 || ((uint)dar > 0x1800000))) {
    return;
  }

  OSProtectRange(3, 0, 0, 3);
  memset((void*)0x81200000, 0, (u32)0x280);
  DCFlushRange((void*)0x81200000, 0x280);
  AIRegisterDMACallback(NULL);
  AIInitDMA(0x81200000, 0x280);
  AIStartDMA();
  DVDCancelAll(nullptr);
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
    if (OSTicksToSeconds(currentTime - lastTime) >= 10) {
      OSResetSystem(1, 0, 0);
    }

    PADRead(pads);
    while (pads[1].err == PAD_ERR_NO_CONTROLLER && OSTicksToSeconds(OSGetTime() - lastTime) < 10) {
      PADReset(PAD_CHAN1_BIT);
      PADRead(pads);
    }

    if (pads[1].err == PAD_ERR_NONE) {
      *(uint*)local_60 = *(uint*)ExitButtons;
      if ((pads[1].button & 0xc10) == 0xc10) {
        lastTime = OSGetTime();

        if ((pads[1].button & 0xf) != 0) {
          if ((pads[1].button & local_60[loopExitCriteria]) != 0) {
            loopExitCriteria += 1;

          } else if ((loopExitCriteria != 0) &&
                     ((pads[1].button & local_60[loopExitCriteria - 1]) == 0)) {
            loopExitCriteria = 0;
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
  OSReport("%s\n", BuildTime);
  OSReport("------------------------- Context 0x%08x -------------------------\n", context);

  for (i = 0; i < 16; ++i) {
    OSReport("r%-2d  = 0x%08x (%14d)  r%-2d  = 0x%08x (%14d)\n", i, context->gpr[i],
             context->gpr[i], i + 0x10, context->gpr[i + 0x10], context->gpr[i + 0x10]);
  }

  OSReport("LR   = 0x%08x                   CR   = 0x%08x\n", context->lr, context->cr);
  OSReport("SRR0 = 0x%08x                   SRR1 = 0x%08x\n", context->srr0, context->srr1);
  OSReport("DSISR= 0x%08x                   DAR  = 0x%08x\n", dsisr, dar);

  if (rs_debugger_buffer != nullptr) {
    strcpy(buffer, rs_debugger_buffer);
  } else {
    buffer[0] = 0;
  }
  uint len = strlen(buffer);
  if (len != 0) {
    len += sprintf(buffer + len, " - Production\n");
  } else {
    len += sprintf(buffer, "Exception %d - Production\n", code);
  }
  len += sprintf(buffer + len, "%s\n", BuildTime);
  len += sprintf(buffer + len, "IP: 0x%8.8x  Mem: 0x%8.8x", context->srr0, dar);
  if (code == 0xf) {
    len += sprintf(buffer + len, " - %d", dsisr);
  }
  len += sprintf(buffer + len, "\n\n");

  OSReport("\nAddress:      Back Chain    LR Save\n");

  gpr = (u32*)context->gpr[1];
  for (i = 0; gpr != NULL && gpr != (void*)0xFFFFFFFF && i++ < rs_debugger_size + 16;) {
    if (gpr > (void*)0x80000400 && gpr < (void*)0x81800000) {
      if (i >= rs_debugger_size) {
        OSReport("0x%08x:   0x%08x    0x%08x\n", gpr, *gpr, gpr[1]);
        u32 iVar2 = gpr[1];
        u32* tmp = fn_8033ED24(iVar2);
        if (tmp == NULL) {
          len += sprintf(buffer + len, "0x%08x: 0x%08x 0x%08x\n", gpr, *gpr, gpr[1]);
        }

        if (tmp != NULL) {
          OSReport("RFO:0x%08x: %s\n", iVar2 - tmp[1], *tmp);
          len += sprintf(buffer + len, "RFO:0x%08x: %s\n", iVar2 - tmp[1], *tmp);
        }
      }
    } else {
      OSReport("0x%08x:   BAD PTR\n", gpr);
      sprintf(buffer + len, "0x%08x: BAD PTR\n", gpr);
      break;
    }
    gpr = (u32*)*gpr;
  }

  OSReport("\nInstruction at 0x%x (read from SRR0) attempted to access invalid address 0x%x "
           "(read from DAR)\n",
           context->srr0, dar);

  if (!SkipFatal) {
    OSFatal(fg, bg, buffer);
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
  rs_debugger_size = 0;
  if (strncmp(format, "SKIP", 4) == 0) {
    const char* s = format + 4;
    format += 5;
    rs_debugger_size = *s - 0x30;
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
