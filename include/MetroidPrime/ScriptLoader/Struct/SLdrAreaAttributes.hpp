#ifndef _SLDRAREAATTRIBUTES
#define _SLDRAREAATTRIBUTES

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrEditorProperties.hpp"

struct SLdrAreaAttributes {
  SLdrAreaAttributes()
  : editorProperties()
  , needSky(false)
  , darkWorld(false)
  , environmentEffects(0)
  , environmentGroupSound(-1)
  , density(1.0f)
  , normalLighting(1.0f)
  , overrideSky(kInvalidAssetId)
  , phazonDamage(0) {
    editorProperties.unknown = 3;
  }

  ~SLdrAreaAttributes();

  SLdrEditorProperties editorProperties;
  bool needSky;
  bool darkWorld;
  int environmentEffects;
  int environmentGroupSound;
  float density;
  float normalLighting;
  CAssetId overrideSky;
  int phazonDamage;
};

void LoadTypedefSLdrAreaAttributes(SLdrAreaAttributes&, CInputStream&);

#endif // _SLDRAREAATTRIBUTES
