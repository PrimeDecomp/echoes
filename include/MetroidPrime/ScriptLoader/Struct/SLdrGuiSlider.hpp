#ifndef _SLDRGUISLIDER
#define _SLDRGUISLIDER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGuiWidgetProperties.hpp"

struct SLdrGuiSlider {
    SLdrGuiSlider();
    ~SLdrGuiSlider();

    SLdrEditorProperties editorProperties;
    SLdrGuiWidgetProperties guiWidgetProperties;
    float minValue;
    float maxValue;
    float increment;
    float slideSpeed;
    int slideSound;
    int slideSoundVolume;
};

void LoadTypedefSLdrGuiSlider(SLdrGuiSlider&, CInputStream&);

#endif // _SLDRGUISLIDER
