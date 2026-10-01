#ifndef _CMAPPABLEOBJECT
#define _CMAPPABLEOBJECT

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/pair.hpp"

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
    kMOT_Elevator = 16,
    kMOT_SaveStation = 17,
    kMOT_MissileStation = 20,
    kMOT_Portal = 21,
    kMOT_Teleporter = 22, // Guessed name
    kMOT_TranslatorDoor = 23,
    kMOT_UpArrow = 24,
    kMOT_DownArrow = 25
  };

  enum EVisMode {
    kVM_Never = 0,
    kVM_Always = 1,
    kVM_MapStationOrVisit = 2,
    kVM_DoorVisit = 3,
    kVM_Visit = 4
  };

  void PostConstruct(const void* buf);
  static void ReadAutomapperTweaks(); // Guessed name
  bool GetIsVisibleToAutoMapper(bool worldVis, const CMapWorldInfo& info) const;
  CTransform4f AdjustTransformForType() const;
  void Draw(int curAreaId, const CMapWorldInfo& info, float alpha, bool needsVtxLoad) const;
  void DrawDoorSurface(int curAreaId, const CMapWorldInfo& info, float alpha, int surfaceIdx,
                       bool needsVtxLoad) const;
  CVector3f BuildSurfaceCenterPoint(int surfaceIdx) const;
  static bool IsDoorType(EMappableObjectType type) { return type >= 0 && type <= 7; }
  void DrawDoor(int curAreaId, const CMapWorldInfo& info, float alpha) const; // Guessed name
  rstl::pair< CColor, CColor > GetDoorColors(int curAreaId, const CMapWorldInfo& info,
                                             float alpha) const;

  EMappableObjectType GetType() const { return mType; }
  TEditorId GetObjId() const { return mObjId; }
  const CTransform4f& GetTransform() const { return mTransform; }

  static CVector3f skDoorVerts[8];

private:
  // Guessed name
  void DrawDoorSurface(const CColor& surfaceColor, const CColor& outlineColor, int surfaceIdx,
                       bool needsVtxLoad) const;

  EMappableObjectType mType;
  EVisMode mVisibilityMode;
  TEditorId mObjId;
  uint xc_;
  CTransform4f mTransform;
  uchar x40_[0x10];
};
CHECK_SIZEOF(CMappableObject, 0x50)

#endif // _CMAPPABLEOBJECT
