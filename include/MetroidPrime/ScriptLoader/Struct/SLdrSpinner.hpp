#ifndef _SLDRSPINNER
#define _SLDRSPINNER

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrSpinner {
    SLdrSpinner();
    ~SLdrSpinner();

    SLdrEditorProperties editorProperties;
    float forwardSpeed;
    float backwardSpeed;
    float unknown_0x449dd059;
    float unknown_0xfc849759;
    bool shotSpinner;
    bool allowWrap;
    bool noBackward;
    bool splineControl;
    int loopSound;
    int startSound;
    int stopSound;
};

void LoadTypedefSLdrSpinner(SLdrSpinner&, CInputStream&);

#endif // _SLDRSPINNER
