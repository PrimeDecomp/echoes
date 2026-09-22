#ifndef _DOLPHIN_THPDEC
#define _DOLPHIN_THPDEC
#include "types.h"
#ifdef __cplusplus
extern "C" {
#endif
BOOL THPInit(void);
s32 THPVideoDecode(void* file, void* y, void* u, void* v, void* work);
u32 THPAudioDecode(s16* audio, u8* file, s32 flag);
#ifdef __cplusplus
}
#endif
#endif
