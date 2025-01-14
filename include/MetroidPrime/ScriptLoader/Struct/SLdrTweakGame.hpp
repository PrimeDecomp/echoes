#ifndef _SLDRTWEAKGAME
#define _SLDRTWEAKGAME

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame_CoinLimitChoices.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame_FragLimitChoices.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame_TimeLimitChoices.hpp"
#include "rstl/string.hpp"

struct SLdrTweakGame {
    SLdrTweakGame();
    ~SLdrTweakGame();

    rstl::string instanceName;
    rstl::string pakFile;
    rstl::string asset;
    float fieldofView;
    float fieldofView2Player;
    bool disableDebugMenu;
    bool unknown_0x7262d27b;
    bool developmentMode;
    float unknown_0xa3dcf42a;
    float unknown_0xb35c72be;
    float unknown_0x4a02103c;
    float unknown_0xe1fca71b;
    float unknown_0xfbce966a;
    float unknown_0x09c6ca10;
    float hardModeDamageMultiplier;
    float hardModeWeaponMultiplier;
    float unknown_0x5ab5812c;
    float unknown_0x53401390;
    int totalPercentage;
    SLdrTweakGame_FragLimitChoices unknown_0x1d627808;
    SLdrTweakGame_TimeLimitChoices unknown_0xb2e8828d;
    SLdrTweakGame_CoinLimitChoices unknown_0x06af87bd;
    SLdrTweakGame_TimeLimitChoices unknown_0x1533ea4e;
    SLdrSpline unknown_0x40818220;
};

void LoadTypedefSLdrTweakGame(SLdrTweakGame&, CInputStream&);

#endif // _SLDRTWEAKGAME
