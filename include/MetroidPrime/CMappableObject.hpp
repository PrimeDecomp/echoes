#ifndef _CMAPPABLEOBJECT
#define _CMAPPABLEOBJECT

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

class CMapWorldInfo;

class CMappableObject {
public:
  enum EMappableObjectType {
    // Guessed names; values are the map door-color selectors.
    kMOT_BlueDoor = 0,
    kMOT_MissileDoor = 1,
    kMOT_DarkBeamDoor = 2,
    kMOT_AnnihilatorBeamDoor = 3,
    kMOT_LightBeamDoor = 4,
    kMOT_SuperMissileDoor = 5,
    kMOT_SeekerDoor = 6,
    kMOT_PowerBombDoor = 7,
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
  static void ReadAutomapperTweaks();
  bool GetIsVisibleToAutoMapper(bool worldVis, const CMapWorldInfo& info) const;
  CTransform4f AdjustTransformForType() const;
  void Draw(int curAreaId, const CMapWorldInfo& info, float alpha, bool needsVtxLoad) const;
  void DrawDoorSurface(int curAreaId, const CMapWorldInfo& info, float alpha, int surfaceIdx,
                       bool needsVtxLoad) const;
  CVector3f BuildSurfaceCenterPoint(int surfaceIdx) const;
  static bool IsDoorType(EMappableObjectType type) { return type >= 0 && type < 8; }

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
