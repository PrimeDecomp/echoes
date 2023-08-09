#ifndef _SLDRGUISLIDER
#define _SLDRGUISLIDER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiWidgetProperties.hpp"

struct SLdrGuiSlider {
    SLdrGuiSlider();
    ~SLdrGuiSlider();

    SLdrEditorProperties editor_properties;
    SLdrGuiWidgetProperties gui_widget_properties;
    float min_value;
    float max_value;
    float increment;
    float slide_speed;
    int slide_sound;
    int slide_sound_volume;
};

void LoadTypedefSLdrGuiSlider(SLdrGuiSlider&, CInputStream&);

#endif // _SLDRGUISLIDER
