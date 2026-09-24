#ifndef _DOLPHIN_HW_REGS
#define _DOLPHIN_HW_REGS

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __MWERKS__
vu16 __VIRegs[59] : 0xCC002000;
vu32 __PIRegs[12] : 0xCC003000;
vu16 __MEMRegs[64] : 0xCC004000;
vu16 __DSPRegs[32] : 0xCC005000;
vu32 __DIRegs[16] : 0xCC006000;
vu32 __SIRegs[64] : 0xCC006400;
vu32 __EXIRegs[16] : 0xCC006800;
vu32 __AIRegs[8] : 0xCC006C00;

#else
#define __VIRegs ((vu16*)0xCC002000)
#define __PIRegs ((vu32*)0xCC003000)
#define __MEMRegs ((vu16*)0xCC004000)
#define __DSPRegs ((vu16*)0xCC005000)
#define __DIRegs ((vu32*)0xCC006000)
#define __SIRegs ((vu32*)0xCC006400)
#define __EXIRegs ((vu32*)0xCC006800)
#define __AIRegs ((vu32*)0xCC006C00)
#endif

#ifdef __cplusplus
}
#endif


// VI register indices
#define VI_VERT_TIMING        (0)
#define VI_DISP_CONFIG        (1)
#define VI_HORIZ_TIMING_0L    (2)
#define VI_HORIZ_TIMING_0U    (3)
#define VI_HORIZ_TIMING_1L    (4)
#define VI_HORIZ_TIMING_1U    (5)
#define VI_VERT_TIMING_ODD    (6)
#define VI_VERT_TIMING_ODD_U  (7)
#define VI_VERT_TIMING_EVEN   (8)
#define VI_VERT_TIMING_EVEN_U (9)

#define VI_BBI_ODD    (10) // burst blanking interval
#define VI_BBI_ODD_U  (11) // burst blanking interval
#define VI_BBI_EVEN   (12) // burst blanking interval
#define VI_BBI_EVEN_U (13) // burst blanking interval

#define VI_TOP_FIELD_BASE_LEFT   (14) // top in 2d, top of left pic in 3d
#define VI_TOP_FIELD_BASE_LEFT_U (15) // top in 2d, top of left pic in 3d

#define VI_TOP_FIELD_BASE_RIGHT   (16) // top of right pic in 3d
#define VI_TOP_FIELD_BASE_RIGHT_U (17) // top of right pic in 3d

#define VI_BTTM_FIELD_BASE_LEFT   (18) // bottom in 2d, bottom of left pic in 3d
#define VI_BTTM_FIELD_BASE_LEFT_U (19) // bottom in 2d, bottom of left pic in 3d

#define VI_BTTM_FIELD_BASE_RIGHT   (20) // bottom of right pic in 3d
#define VI_BTTM_FIELD_BASE_RIGHT_U (21) // bottom of right pic in 3d

#define VI_VERT_COUNT  (22) // vertical display position
#define VI_HORIZ_COUNT (23) // horizontal display position

#define VI_DISP_INT_0  (24) // display interrupt 0L
#define VI_DISP_INT_0U (25) // display interrupt 0U
#define VI_DISP_INT_1  (26) // display interrupt 1L
#define VI_DISP_INT_1U (27) // display interrupt 1U
#define VI_DISP_INT_2  (28) // display interrupt 2L
#define VI_DISP_INT_2U (29) // display interrupt 2U
#define VI_DISP_INT_3  (30) // display interrupt 3L
#define VI_DISP_INT_3U (31) // display interrupt 3U

#define VI_HSW (36) // horizontal scaling width
#define VI_HSR (37) // horizontal scaling register

#define VI_FCT_0  (38) // filter coefficient table 0L
#define VI_FCT_0U (39) // filter coefficient table 0U
#define VI_FCT_1  (40) // filter coefficient table 1L
#define VI_FCT_1U (41) // filter coefficient table 1U
#define VI_FCT_2  (42) // filter coefficient table 2L
#define VI_FCT_2U (43) // filter coefficient table 2U
#define VI_FCT_3  (44) // filter coefficient table 3L
#define VI_FCT_3U (45) // filter coefficient table 3U
#define VI_FCT_4  (46) // filter coefficient table 4L
#define VI_FCT_4U (47) // filter coefficient table 4U
#define VI_FCT_5  (48) // filter coefficient table 5L
#define VI_FCT_5U (49) // filter coefficient table 5U
#define VI_FCT_6  (50) // filter coefficient table 6L
#define VI_FCT_6U (51) // filter coefficient table 6U

#define VI_CLOCK_SEL (54) // clock select
#define VI_DTV_STAT  (55) // DTV status

#define VI_WIDTH (56)

#endif // _DOLPHIN_HW_REGS
