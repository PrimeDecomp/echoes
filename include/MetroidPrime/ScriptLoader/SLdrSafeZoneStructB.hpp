#ifndef _SLDRSAFEZONESTRUCTB
#define _SLDRSAFEZONESTRUCTB

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrSafeZoneStructB {
    SLdrSafeZoneStructB();
    ~SLdrSafeZoneStructB();

    int turn_on_sound;
    float unknown_0xd4839a3f;
    int active_loop_sound;
    int turn_off_sound;
    int player_enter_sound;
    int player_exit_sound;
    CAssetId dark_visor_spot_texture;
    float dark_visor_spot_max_size;
    CAssetId shell_environment_map;
    float shell1_animated_horiz_rate;
    float shell1_animated_vert_rate;
    float shell1_scale_horiz;
    float shell1_scale_vert;
    CAssetId shell1_texture;
    float shell2_animated_horiz_rate;
    float shell2_animated_vert_rate;
    float shell2_scale_horiz;
    float shell2_scale_vert;
    CAssetId shell2_texture;
    CColor shell_color;
    CColor unknown_0xe68b1fa8;
};

void LoadTypedefSLdrSafeZoneStructB(SLdrSafeZoneStructB&, CInputStream&);

#endif // _SLDRSAFEZONESTRUCTB
