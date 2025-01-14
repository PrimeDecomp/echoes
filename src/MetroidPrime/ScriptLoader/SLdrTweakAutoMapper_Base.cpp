#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper_Base.hpp"

SLdrTweakAutoMapper_Base::SLdrTweakAutoMapper_Base() {
  unknown_0xcbe595d8 = true;
  unknown_0x8ecb53a6 = true;
  scaleMoveSpeedWithCameraDistance = true;
  unknown_0x6bea9324 = 175.0f;
  unknown_0x065dd754 = 50.0f;
  unknown_0x57a46c09 = 700.0f;
  unknown_0xb54255b5 = -89.900002f;
  unknown_0x0c64cec4 = 0.0f;
  unknown_0x335ebc7e = 75.0f;
  mapScreenAreaOpacity = 0.69999999f;
  unknown_0x533c5684 = CColor::Black();
  unknown_0xeb383668 = 120.0f;
  unknown_0x27151ede = -45.0f;
  unknown_0x434172c3 = 45.0f;
  unknown_0x68097036 = 0.60000002f;
  unknown_0x03adcf46 = CColor::Black();
  unknown_0xd3fae283 = CColor::Black();
  unknown_0x65d2cf45 = CColor::Black();
  unknown_0xb5752c08 = CColor::Black();
  unknown_0x035d01ce = CColor::Black();
  unknown_0x805d5fa3 = CColor::Black();
  unknown_0x36757265 = CColor::Black();
  unknown_0xe550fcbd = CColor::Black();
  unknown_0x5378d17b = CColor::Black();
  unknown_0x0ebf3cbc = CColor::Black();
  unknown_0xb897117a = CColor::Black();
  unknown_0x3e670f6a = CColor::Black();
  unknown_0x884f22ac = CColor::Black();
  unknown_0x7bdb0edf = 0.40000001f;
  unknown_0x12221909 = 0.40000001f;
  unknown_0x38dbbc09 = 0.5f;
  unknown_0x30610062 = 0.5f;
  unknown_0xb6acea88 = 0.5f;
  unknown_0x73de4110 = 0.5f;
  mapScreenZoomSpeed = 3.0f;
  mapScreenCircleSpeed = 3.0f;
  mapScreenMoveSpeed = 3.0f;
  unknown_0xab82e268 = CColor::Black();
  unknown_0x1daacfae = CColor::Black();
  unknown_0xdad161a1 = CColor::Black();
  unknown_0x6cf94c67 = CColor::Black();
  unknown_0x47967404 = 0.60000002f;
  unknown_0x0ece1950 = 0.30000001f;
  unknown_0x9ac1bdde = 0.40000001f;
  unknown_0x97a19386 = 0.30000001f;
  unknown_0xcb9e3a54 = 0.25f;
  unknown_0x2511a49b = 0.25f;
  unknown_0x16c9f38e = 0.2f;
  unknown_0xbc7e2e4d = 0.2f;
  unknown_0x15564d32 = 3.5699999f;
  unknown_0xf5479260 = 3.5699999f;
  unknown_0x271b644e = 2.0f;
  unknown_0x52dc08c1 = 24.0f;
  unknown_0x9980db64 = 348.0f;
  unknown_0x23f59057 = 152.0f;
  unknown_0xad3d5a3f = 114.0f;
  unknown_0x3315d22b = 0.85000002f;
  unknown_0x9e4007b6 = 1.85f;
  unknown_0x7a8d3d46 = 1.36f;
  unknown_0x2b97d64c = false;
  unknown_0xbdc57ce0 = 800.0f;
  unknown_0x7d59c854 = 400.0f;
  unknown_0x3c4ef7d2 = 2000.0f;
  unknown_0x2b483e9f = 0.5f;
  unknown_0x706f52fe = 5.0f;
  unknown_0x62f9ebf6 = 1.0f;
  unknown_0xa9a53853 = 0.63499999f;
  unknown_0x722b1bc0 = -0.050000001f;
  playerModelColor = CColor::Black();
  unknown_0x5a87c156 = CColor::Black();
  playerSurfaceColor = CColor::Black();
  playerOutlineColor = CColor::Black();
  textColor = CColor::Black();
  textOutlineColor = CColor::Black();
  unknown_0x1a4b8068 = CColor::Black();
  frameColor = CColor::Black();
  titleColor = CColor::Black();
  legendBackgroundColor = CColor::Black();
  legendGradientColor = CColor::Black();
}

SLdrTweakAutoMapper_Base::~SLdrTweakAutoMapper_Base() {}

void LoadTypedefSLdrTweakAutoMapper_Base(SLdrTweakAutoMapper_Base& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xcbe595d8:
      sldrThis.unknown_0xcbe595d8 = input.ReadBool();
      break;
    case 0x8ecb53a6:
      sldrThis.unknown_0x8ecb53a6 = input.ReadBool();
      break;
    case 0x30b13740:
      sldrThis.scaleMoveSpeedWithCameraDistance = input.ReadBool();
      break;
    case 0x6bea9324:
      sldrThis.unknown_0x6bea9324 = input.ReadFloat();
      break;
    case 0x065dd754:
      sldrThis.unknown_0x065dd754 = input.ReadFloat();
      break;
    case 0x57a46c09:
      sldrThis.unknown_0x57a46c09 = input.ReadFloat();
      break;
    case 0xb54255b5:
      sldrThis.unknown_0xb54255b5 = input.ReadFloat();
      break;
    case 0x0c64cec4:
      sldrThis.unknown_0x0c64cec4 = input.ReadFloat();
      break;
    case 0x335ebc7e:
      sldrThis.unknown_0x335ebc7e = input.ReadFloat();
      break;
    case 0x45be3f6b:
      sldrThis.mapScreenAreaOpacity = input.ReadFloat();
      break;
    case 0x533c5684:
      sldrThis.unknown_0x533c5684 = CColor(input);
      break;
    case 0xeb383668:
      sldrThis.unknown_0xeb383668 = input.ReadFloat();
      break;
    case 0x27151ede:
      sldrThis.unknown_0x27151ede = input.ReadFloat();
      break;
    case 0x434172c3:
      sldrThis.unknown_0x434172c3 = input.ReadFloat();
      break;
    case 0x68097036:
      sldrThis.unknown_0x68097036 = input.ReadFloat();
      break;
    case 0x03adcf46:
      sldrThis.unknown_0x03adcf46 = CColor(input);
      break;
    case 0xd3fae283:
      sldrThis.unknown_0xd3fae283 = CColor(input);
      break;
    case 0x65d2cf45:
      sldrThis.unknown_0x65d2cf45 = CColor(input);
      break;
    case 0xb5752c08:
      sldrThis.unknown_0xb5752c08 = CColor(input);
      break;
    case 0x035d01ce:
      sldrThis.unknown_0x035d01ce = CColor(input);
      break;
    case 0x805d5fa3:
      sldrThis.unknown_0x805d5fa3 = CColor(input);
      break;
    case 0x36757265:
      sldrThis.unknown_0x36757265 = CColor(input);
      break;
    case 0xe550fcbd:
      sldrThis.unknown_0xe550fcbd = CColor(input);
      break;
    case 0x5378d17b:
      sldrThis.unknown_0x5378d17b = CColor(input);
      break;
    case 0x0ebf3cbc:
      sldrThis.unknown_0x0ebf3cbc = CColor(input);
      break;
    case 0xb897117a:
      sldrThis.unknown_0xb897117a = CColor(input);
      break;
    case 0x3e670f6a:
      sldrThis.unknown_0x3e670f6a = CColor(input);
      break;
    case 0x884f22ac:
      sldrThis.unknown_0x884f22ac = CColor(input);
      break;
    case 0x7bdb0edf:
      sldrThis.unknown_0x7bdb0edf = input.ReadFloat();
      break;
    case 0x12221909:
      sldrThis.unknown_0x12221909 = input.ReadFloat();
      break;
    case 0x38dbbc09:
      sldrThis.unknown_0x38dbbc09 = input.ReadFloat();
      break;
    case 0x30610062:
      sldrThis.unknown_0x30610062 = input.ReadFloat();
      break;
    case 0xb6acea88:
      sldrThis.unknown_0xb6acea88 = input.ReadFloat();
      break;
    case 0x73de4110:
      sldrThis.unknown_0x73de4110 = input.ReadFloat();
      break;
    case 0x19069725:
      sldrThis.mapScreenZoomSpeed = input.ReadFloat();
      break;
    case 0x5ba0de1e:
      sldrThis.mapScreenCircleSpeed = input.ReadFloat();
      break;
    case 0x310b37a1:
      sldrThis.mapScreenMoveSpeed = input.ReadFloat();
      break;
    case 0xab82e268:
      sldrThis.unknown_0xab82e268 = CColor(input);
      break;
    case 0x1daacfae:
      sldrThis.unknown_0x1daacfae = CColor(input);
      break;
    case 0xdad161a1:
      sldrThis.unknown_0xdad161a1 = CColor(input);
      break;
    case 0x6cf94c67:
      sldrThis.unknown_0x6cf94c67 = CColor(input);
      break;
    case 0x47967404:
      sldrThis.unknown_0x47967404 = input.ReadFloat();
      break;
    case 0x0ece1950:
      sldrThis.unknown_0x0ece1950 = input.ReadFloat();
      break;
    case 0x9ac1bdde:
      sldrThis.unknown_0x9ac1bdde = input.ReadFloat();
      break;
    case 0x97a19386:
      sldrThis.unknown_0x97a19386 = input.ReadFloat();
      break;
    case 0xcb9e3a54:
      sldrThis.unknown_0xcb9e3a54 = input.ReadFloat();
      break;
    case 0x2511a49b:
      sldrThis.unknown_0x2511a49b = input.ReadFloat();
      break;
    case 0x16c9f38e:
      sldrThis.unknown_0x16c9f38e = input.ReadFloat();
      break;
    case 0xbc7e2e4d:
      sldrThis.unknown_0xbc7e2e4d = input.ReadFloat();
      break;
    case 0x15564d32:
      sldrThis.unknown_0x15564d32 = input.ReadFloat();
      break;
    case 0xf5479260:
      sldrThis.unknown_0xf5479260 = input.ReadFloat();
      break;
    case 0x271b644e:
      sldrThis.unknown_0x271b644e = input.ReadFloat();
      break;
    case 0x52dc08c1:
      sldrThis.unknown_0x52dc08c1 = input.ReadFloat();
      break;
    case 0x9980db64:
      sldrThis.unknown_0x9980db64 = input.ReadFloat();
      break;
    case 0x23f59057:
      sldrThis.unknown_0x23f59057 = input.ReadFloat();
      break;
    case 0xad3d5a3f:
      sldrThis.unknown_0xad3d5a3f = input.ReadFloat();
      break;
    case 0x3315d22b:
      sldrThis.unknown_0x3315d22b = input.ReadFloat();
      break;
    case 0x9e4007b6:
      sldrThis.unknown_0x9e4007b6 = input.ReadFloat();
      break;
    case 0x7a8d3d46:
      sldrThis.unknown_0x7a8d3d46 = input.ReadFloat();
      break;
    case 0x2b97d64c:
      sldrThis.unknown_0x2b97d64c = input.ReadBool();
      break;
    case 0xbdc57ce0:
      sldrThis.unknown_0xbdc57ce0 = input.ReadFloat();
      break;
    case 0x7d59c854:
      sldrThis.unknown_0x7d59c854 = input.ReadFloat();
      break;
    case 0x3c4ef7d2:
      sldrThis.unknown_0x3c4ef7d2 = input.ReadFloat();
      break;
    case 0x2b483e9f:
      sldrThis.unknown_0x2b483e9f = input.ReadFloat();
      break;
    case 0x706f52fe:
      sldrThis.unknown_0x706f52fe = input.ReadFloat();
      break;
    case 0x62f9ebf6:
      sldrThis.unknown_0x62f9ebf6 = input.ReadFloat();
      break;
    case 0xa9a53853:
      sldrThis.unknown_0xa9a53853 = input.ReadFloat();
      break;
    case 0x722b1bc0:
      sldrThis.unknown_0x722b1bc0 = input.ReadFloat();
      break;
    case 0x4c3fc933:
      sldrThis.playerModelColor = CColor(input);
      break;
    case 0x5a87c156:
      sldrThis.unknown_0x5a87c156 = CColor(input);
      break;
    case 0x9c0c5318:
      sldrThis.playerSurfaceColor = CColor(input);
      break;
    case 0x2a247ede:
      sldrThis.playerOutlineColor = CColor(input);
      break;
    case 0x44303a9c:
      sldrThis.textColor = CColor(input);
      break;
    case 0xf2e13506:
      sldrThis.textOutlineColor = CColor(input);
      break;
    case 0x1a4b8068:
      sldrThis.unknown_0x1a4b8068 = CColor(input);
      break;
    case 0xa485372c:
      sldrThis.frameColor = CColor(input);
      break;
    case 0x536647d5:
      sldrThis.titleColor = CColor(input);
      break;
    case 0xa6b633fa:
      sldrThis.legendBackgroundColor = CColor(input);
      break;
    case 0x01cea7f9:
      sldrThis.legendGradientColor = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
