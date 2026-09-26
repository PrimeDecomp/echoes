#ifndef _IWORLD
#define _IWORLD

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/string.hpp"

class CMapWorld;
class IGameArea;

class IWorld {
public:
  virtual ~IWorld();
  virtual CAssetId IGetWorldAssetId() const = 0;
  virtual CAssetId IGetStringTableAssetId() const = 0;
  virtual CAssetId IGetDarkStringTableAssetId() const = 0; // Guessed name
  virtual CAssetId IGetSaveWorldAssetId() const = 0;
  virtual const CMapWorld* IGetMapWorld() const = 0;
  virtual CMapWorld* IMapWorld() = 0;
  virtual const IGameArea* IGetAreaAlways(TAreaId id) const = 0;
  virtual TAreaId IGetCurrentAreaId() const = 0;
  virtual TAreaId IGetAreaId(CAssetId id) const = 0;
  virtual bool ICheckWorldComplete() = 0;
  virtual rstl::string IGetDefaultAudioTrack() const = 0;
  virtual int IGetAreaCount() const = 0;
  virtual uint IGetTempleKeyWorldIndex() const = 0; // Guessed name
  virtual bool ICancelLoad() = 0;                   // Guessed name
};
CHECK_SIZEOF(IWorld, 0x4)

#endif // _IWORLD
