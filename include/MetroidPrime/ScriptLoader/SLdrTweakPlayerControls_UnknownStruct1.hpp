#ifndef _SLDRTWEAKPLAYERCONTROLS_UNKNOWNSTRUCT1
#define _SLDRTWEAKPLAYERCONTROLS_UNKNOWNSTRUCT1

#include "Kyoto/Streams/CInputStream.hpp"

struct SLdrTweakPlayerControls_UnknownStruct1 {
    SLdrTweakPlayerControls_UnknownStruct1();
    ~SLdrTweakPlayerControls_UnknownStruct1();

    int forward;
    int backward;
    int turnLeft;
    int turnRight;
    int strafeLeft;
    int strafeRight;
    int lookLeft;
    int lookRight;
    int lookUp;
    int lookDown;
    int jump;
    int jump2;
    int fireBeam;
    int fireBeam2;
    int autoFireBeam;
    int chargeBeam;
    int chargeBeam2;
    int useItem;
    int aimUp;
    int aimDown;
    int cycleBeamUp;
    int cycleBeamDown;
    int cycleItem;
    int selectPowerBeam;
    int selectIceBeam;
    int selectWaveBeam;
    int selectPlasmaBeam;
    int gunToggleHolster;
    int orbitClose;
    int orbitFar;
    int orbitObject;
    int orbitSelect;
    int orbitConfirm;
    int orbitLeft;
    int orbitRight;
    int orbitUp;
    int orbitDown;
    int holdLook1;
    int holdLook2;
    int lookZoomIn;
    int lookZoomOut;
    int holdAim;
    int mapCircleUp;
    int mapCircleDown;
    int mapCircleLeft;
    int mapCircleRight;
    int mapMoveForward;
    int mapMoveBack;
    int mapMoveLeft;
    int mapMoveRight;
    int mapZoomIn;
    int mapZoomOut;
    int spiderBall;
    int chaseCamera;
    int xRayVisor;
    int thermoVisor;
    int enviroVisor;
    int noVisor;
    int visorMenu;
    int cycleVisorUp;
    int cycleVisorDown;
    int darkVisorToggle;
    int crosshairs;
    int unknown_0x29293fb1;
    int useShield;
    int scanItem;
    int inventoryScreen;
    int mapScreen;
    int optionsScreen;
    int logScreen;
    int unknown_0xbf218f4f;
    int unknown_0x05ef2422;
    int boostBall;
    int morphIntoBall;
    int morphFromBall;
};

void LoadTypedefSLdrTweakPlayerControls_UnknownStruct1(SLdrTweakPlayerControls_UnknownStruct1&, CInputStream&);

#endif // _SLDRTWEAKPLAYERCONTROLS_UNKNOWNSTRUCT1
