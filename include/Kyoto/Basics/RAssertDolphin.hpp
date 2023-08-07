#ifndef _RASSERTDOLPHIN
#define _RASSERTDOLPHIN

#include "types.h"

#include "dolphin/os.h"

void ErrorHandler(OSError code, OSContext* context, int, uint);
void SetErrorHandlers();
void rs_debugger_printf(const char* format, ...);

#endif // _RASSERTDOLPHIN
