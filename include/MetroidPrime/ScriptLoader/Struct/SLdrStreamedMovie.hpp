#ifndef _SLDRSTREAMEDMOVIE
#define _SLDRSTREAMEDMOVIE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrStreamedMovie {
    SLdrStreamedMovie();
    ~SLdrStreamedMovie();

    SLdrEditorProperties editorProperties;
    rstl::string movieFile;
    bool loop;
    bool videoFilterEnabled;
    int unknown;
    int volume;
    int volumeType;
    float cacheLength;
    float fadeOutTime;
};

void LoadTypedefSLdrStreamedMovie(SLdrStreamedMovie&, CInputStream&);

#endif // _SLDRSTREAMEDMOVIE
