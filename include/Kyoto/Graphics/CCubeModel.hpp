#ifndef _CCUBEMODEL
#define _CCUBEMODEL

#include "types.h"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/vector.hpp"

class CStopwatch;
class CTexture;
class CTransform4f;
class CVector3f;

class CCubeModel {
public:
  static void SetRenderModelBlack(bool v);
  static void DisableShadowMaps();
  static void EnableShadowMaps(const CTexture*, const CTransform4f&, unsigned char, unsigned char);
  static void SetNewPlayerPositionAndTime(const CVector3f& pos, const CStopwatch& stopwatch);
  static void SetDrawingOccluders(bool);

  void SetUsingPackedLightmaps(bool v) const;
  static bool IsUsingPackedLightmaps() { return sUsingPackedLightmaps; }

  const rstl::vector< TCachedToken< CTexture > >& GetTextures() const { return *x1c_textures; }
  const CAABox& GetBoundingBox() const { return x20_bounds; }

private:
  uchar x0_instance[0x1c];
  rstl::vector< TCachedToken< CTexture > >* x1c_textures;
  CAABox x20_bounds;

  static bool sUsingPackedLightmaps;
};

#endif // _CCUBEMODEL
