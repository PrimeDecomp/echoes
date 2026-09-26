#ifndef _CMAPPABLEOBJECT
#define _CMAPPABLEOBJECT

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

class CMapWorldInfo;

class CMappableObject {
public:
  enum EMappableObjectType {
    kMOT_Teleporter = 22 // Guessed name
  };

  enum EVisMode {
    kVM_Never = 0,
    kVM_Always = 1,
    kVM_MapStationOrVisit = 2,
    kVM_DoorVisit = 3,
    kVM_Visit = 4
  };

  void PostConstruct();
  bool GetIsVisibleToAutoMapper(bool worldVis, const CMapWorldInfo& info) const;
  CTransform4f AdjustTransformForType() const;

  EMappableObjectType GetType() const { return mType; }
  TEditorId GetObjId() const { return mObjId; }
  const CTransform4f& GetTransform() const { return mTransform; }

private:
  EMappableObjectType mType;
  EVisMode mVisibilityMode;
  TEditorId mObjId;
  uint xc_;
  CTransform4f mTransform;
  uchar x40_[0x10];
};
CHECK_SIZEOF(CMappableObject, 0x50)

#endif // _CMAPPABLEOBJECT
