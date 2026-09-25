#ifndef _CPARTICLEDATAFACTORY
#define _CPARTICLEDATAFACTORY

#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CGenDescription;
class CInputStream;
class CSimplePool;

class CParticleDataFactory {
public:
  static CUVElement* GetTextureElement(CInputStream& in, CSimplePool* resPool);
  static CColorElement* GetColorElement(CInputStream& in);
  static CModVectorElement* GetModVectorElement(CInputStream& in);
  static CEmitterElement* GetEmitterElement(CInputStream& in);
  static CVectorElement* GetVectorElement(CInputStream& in);
  static CRealElement* GetRealElement(CInputStream& in);
  static CIntElement* GetIntElement(CInputStream& in);

  static float GetReal(CInputStream& in);
  static int GetInt(CInputStream& in);
  static bool GetBool(CInputStream& in);
  static int GetBitflag(CInputStream& in);
  static FourCC GetClassID(CInputStream& in);
  static rstl::optional_object< TToken< CGenDescription > >
  GetChildGeneratorDesc(CInputStream& in, CSimplePool* pool,
                        const rstl::vector< CAssetId >& resources);
};

#endif // _CPARTICLEDATAFACTORY
