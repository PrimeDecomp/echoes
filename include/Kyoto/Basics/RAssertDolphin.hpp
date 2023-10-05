#ifndef _RASSERTDOLPHIN
#define _RASSERTDOLPHIN

#include "types.h"

#include "dolphin/os.h"

#define BUILD_INFO_TAG "!#$MetroidBuildInfo!#$"
// TODO: Add handling for build info
#define BUILD_TIME_DUMMY "Build v1.028 10/18/2004 10:44:32\0AD"
extern const char* BuildTime;

#define BUILD_INFO BUILD_INFO_TAG BUILD_TIME_DUMMY
#define BUILD_INFO_TAG_SIZE sizeof(BUILD_INFO_TAG) - 1

void ErrorHandler(OSError code, OSContext* context, int, uint);
void SetErrorHandlers();
void rs_debugger_printf(const char* format, ...);

#endif // _RASSERTDOLPHIN
