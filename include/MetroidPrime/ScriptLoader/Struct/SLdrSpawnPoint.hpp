#ifndef _SLDRSPAWNPOINT
#define _SLDRSPAWNPOINT

#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/SLdrSpawnPointStruct.hpp"

struct SLdrSpawnPoint {
    SLdrSpawnPoint();
    ~SLdrSpawnPoint();

    SLdrEditorProperties editorProperties;
    bool firstSpawn;
    bool morphed;
    SLdrSpawnPointStruct amount;
    SLdrSpawnPointStruct capacity;
};

void LoadTypedefSLdrSpawnPoint(SLdrSpawnPoint&, CInputStream&);

#endif // _SLDRSPAWNPOINT
