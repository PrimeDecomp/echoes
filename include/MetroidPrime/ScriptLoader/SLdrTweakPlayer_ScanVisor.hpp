#ifndef _SLDRTWEAKPLAYER_SCANVISOR
#define _SLDRTWEAKPLAYER_SCANVISOR

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayer_ScanVisor {
    SLdrTweakPlayer_ScanVisor();
    ~SLdrTweakPlayer_ScanVisor();

    float scan_distance;
    bool scan_retention;
    bool scan_freezes_game;
    bool scan_line_of_sight;
    float scan_max_target_distance;
    float scan_max_lock_distance;
    float scan_camera_speed;
};

void LoadTypedefSLdrTweakPlayer_ScanVisor(SLdrTweakPlayer_ScanVisor&, CInputStream&);

#endif // _SLDRTWEAKPLAYER_SCANVISOR
