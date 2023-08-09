#ifndef _SLDRSTREAMEDMOVIE
#define _SLDRSTREAMEDMOVIE

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "rstl/string.hpp"

struct SLdrStreamedMovie {
    SLdrStreamedMovie();
    ~SLdrStreamedMovie();

    SLdrEditorProperties editor_properties;
    rstl::string movie_file;
    bool loop;
    bool video_filter_enabled;
    int unknown;
    int volume;
    int volume_type;
    float cache_length;
    float fade_out_time;
};

void LoadTypedefSLdrStreamedMovie(SLdrStreamedMovie&, CInputStream&);

#endif // _SLDRSTREAMEDMOVIE
