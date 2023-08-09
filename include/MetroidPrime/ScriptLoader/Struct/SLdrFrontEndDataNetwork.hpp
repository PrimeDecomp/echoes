#ifndef _SLDRFRONTENDDATANETWORK
#define _SLDRFRONTENDDATANETWORK

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrFrontEndDataNetwork {
    SLdrFrontEndDataNetwork();
    ~SLdrFrontEndDataNetwork();

    SLdrEditorProperties editor_properties;
    bool is_root;
    bool unknown_0x77f59f4a;
    bool unknown_0x29c0cb7f;
    bool can_be_selected;
    bool is_proxy;
    bool is_locked;
    bool unknown_0x8b8fa0fe;
    bool unknown_0xd0f2d612;
    float connection_radius;
    CAssetId hot_dot_texture;
    CAssetId txtr_0x547fffc3;
    CAssetId txtr_0xcdaaba00;
    CColor selected_color;
    CColor unselected_min_color;
    CColor unselected_max_color;
    CColor disabled_color;
    int rotation_sound;
    spline transition_shrink_spline;
    float transition_shrink_time;
    spline transition_move_spline;
    float transition_move_time;
    spline transition_expand_spline;
    float transition_expand_time;
    spline transition_move_in_spline;
    float transition_move_in_time;
    int rotation_sound_volume;
};

void LoadTypedefSLdrFrontEndDataNetwork(SLdrFrontEndDataNetwork&, CInputStream&);

#endif // _SLDRFRONTENDDATANETWORK
