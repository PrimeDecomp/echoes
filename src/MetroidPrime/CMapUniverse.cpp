#include "MetroidPrime/CMapUniverse.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CMapArea.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

class CMapObjectSortInfoGreaterThan {
public:
  bool operator()(const CMapUniverse::CMapObjectSortInfo& a,
                  const CMapUniverse::CMapObjectSortInfo& b) const {
    return a.GetZDistance() > b.GetZDistance();
  }
};

CMapUniverse::CMapObjectSortInfo::CMapObjectSortInfo(float zDistance, int worldIndex, int areaIndex,
                                                     int objectIndex, CColor surfaceColor,
                                                     CColor outlineColor)
: mZDistance(zDistance)
, mWorldIndex(worldIndex)
, mAreaIndex(areaIndex)
, mObjectIndex(objectIndex)
, mSurfaceColor(surfaceColor)
, mOutlineColor(outlineColor) {}

CMapUniverse::CMapUniverseDrawParms::CMapUniverseDrawParms(
    float alpha, int worldIdx, CAssetId worldId, int closestHex, float flashPulse,
    const CStateManager& mgr, const CTransform4f& model, const CTransform4f& view,
    bool teleportMode)
: mAlpha(alpha)
, mFocusWorldIndex(worldIdx)
, mFocusWorldRes(worldId)
, mFocusAreaIndex(closestHex)
, mFlashPulse(flashPulse)
, mStateManager(mgr)
, mPaneProjectionTransform(model)
, mCameraTransform(view)
, mTeleportMode(teleportMode) {}

CMapUniverse::CMapUniverse(CInputStream& in, uint version)
: mHexagonId(in.Get< CAssetId >())
, mHexagonToken(gpSimplePool->GetObj(SObjectTag('MAPA', mHexagonId)))
, mUniverseCenter(CVector3f::Zero())
, mUniverseRadius(1600.f) {
  mWorldDatas.reserve(in.Get< uint >());
  for (int i = 0; i < mWorldDatas.capacity(); ++i) {
    mWorldDatas.push_back_unsafe(CMapWorldData(in, version));
  }
  mHexagonToken.Lock();
}

CMapUniverse::~CMapUniverse() {}

void CMapUniverse::Draw(const CMapUniverseDrawParms& parms, const CVector3f& pos, float depth1,
                        float depth2) const {
  if (mHexagonToken.TryCache()) {
    int surfaceCount = 0;
    int numSurfaces = mHexagonToken.GetObject()->GetNumSurfaces();
    for (int i = 0; i < mWorldDatas.size(); ++i) {
      surfaceCount += numSurfaces * mWorldDatas[i].GetNumMapAreaDatas();
    }

    rstl::vector< CMapObjectSortInfo > sortInfos;
    sortInfos.reserve(surfaceCount);
    const float alpha = parms.GetAlpha();
    const CTransform4f& model = parms.GetPaneProjectionTransform();
    const CTransform4f& camera = parms.GetCameraTransform();
    const CMapArea& mapArea = *mHexagonToken.GetObject();

    for (int w = 0; w < mWorldDatas.size(); ++w) {
      const CMapWorldData& world = mWorldDatas[w];
      if (gpGameState->StateForWorld(world.GetWorldAssetId()).GetMapWorldInfo()->IsAnythingSet()) {
        const float worldAlpha =
            alpha * (parms.GetTeleportMode() && world.GetWorldLabel() == "TempleHub" ? 0.2f : 1.f);
        const bool selected = w == parms.GetFocusWorldIndex();
        const CColor surfaceColor =
            selected ? world.GetSurfaceColorSelected().WithAlphaModulatedBy(worldAlpha)
                     : world.GetSurfaceColorUnselected().WithAlphaModulatedBy(worldAlpha);
        const CColor outlineColor =
            selected ? world.GetOutlineColorSelected().WithAlphaModulatedBy(worldAlpha)
                     : world.GetSurfaceColorUnselected().WithAlphaModulatedBy(worldAlpha);
        for (int h = 0; h < world.GetNumMapAreaDatas(); ++h) {
          const CTransform4f transform =
              camera.GetQuickInverse() * world.GetMapAreaData(h).GetTransform();
          for (int s = 0; s < mapArea.GetNumSurfaces(); ++s) {
            const CVector3f center = transform * mapArea.GetSurface(s).GetCenterPosition();
            sortInfos.push_back_unsafe(
                CMapObjectSortInfo(center.GetY(), w, h, s, surfaceColor, outlineColor));
          }
        }
      }
    }

    if (!sortInfos.empty()) {
      rstl::sort(sortInfos.begin(), sortInfos.end(), CMapObjectSortInfoGreaterThan());
      CMapArea::CMapAreaSurface::SetupGXMaterial();
      int lastWorld = -1;
      int lastArea = -1;
      for (int i = 0; i < sortInfos.size(); ++i) {
        const CMapObjectSortInfo& info = sortInfos[i];
        int worldIndex = info.GetWorldIndex();
        int areaIndex = info.GetAreaIndex();
        CColor surfaceColor = info.GetSurfaceColor();
        CColor outlineColor = info.GetOutlineColor();
        const CMapWorldData& world = mWorldDatas[worldIndex];
        const CTransform4f& transform = world.GetMapAreaData(areaIndex).GetTransform();
        const CMapArea::CMapAreaSurface& surface = mapArea.GetSurface(info.GetObjectIndex());

        if (parms.GetFocusWorldRes() == world.GetWorldAssetId() &&
            areaIndex == parms.GetFocusAreaIndex()) {
          const uchar surfaceAlpha = surfaceColor.GetAlphau8();
          const uchar outlineAlpha = outlineColor.GetAlphau8();
          surfaceColor =
              CColor::Lerp(gpTweakAutoMapper->GetSurfaceVisitedSelectColor(),
                           gpTweakAutoMapper->GetAreaFlashPulseColor(), parms.GetFlashPulse());
          outlineColor =
              CColor::Lerp(gpTweakAutoMapper->GetOutlineVisitedSelectColor(),
                           gpTweakAutoMapper->GetAreaFlashPulseColor(), parms.GetFlashPulse());
          surfaceColor.SetAlpha(surfaceAlpha);
          outlineColor.SetAlpha(outlineAlpha);
        }

        CTransform4f normalTransform = transform;
        normalTransform.Orthonormalize();
        const float linear = gpTweakAutoMapper->GetMapSurfaceNormColorLinear();
        const float constant = gpTweakAutoMapper->GetMapSurfaceNormColorConstant();
        const float lit =
            linear *
            rstl::max_val(0.f, CVector3f::Dot(-1.f * camera.GetForward(),
                                              normalTransform.Rotate(surface.GetNormal())));
        const float shade = constant + lit;
        const CColor color = CColor::Modulate(surfaceColor, CColor(shade, shade, shade, 1.f));

        const bool changed = lastArea != areaIndex || lastWorld != worldIndex;
        if (changed) {
          gpRender->SetModelMatrix(model * transform);
        }
        surface.Draw(changed ? mapArea.GetVertices() : nullptr, color, outlineColor, 2.f);
        lastWorld = worldIndex;
        lastArea = areaIndex;
      }
    }
  }
}

CMapUniverse::CMapAreaData::CMapAreaData(CInputStream& in) : mTransform(in) {}

CMapUniverse::CMapWorldData::CMapWorldData(CInputStream& in, uint version)
: mLabel(in)
, mWorldAssetId(in.Get< CAssetId >())
, mTransform(in)
, mAreaDatas(in)
, mSurfColorSelected(version != 0 ? CColor(in) : CColor(0))
, mOutlineColorSelected(static_cast< uchar >(255), 0, 255)
, mSurfColorUnselected(static_cast< uchar >(255), 0, 255)
, mOutlineColorUnselected(static_cast< uchar >(255), 0, 255)
, mCenterPoint(CVector3f::Zero()) {
  if (version == 0) {
    mSurfColorSelected = CColor(mWorldAssetId).WithAlphaOf(1.f);
  }

  mOutlineColorSelected = CColor::Lerp(CColor::White(), mSurfColorSelected, 0.5f);
  mSurfColorUnselected = CColor::Lerp(CColor::Black(), mSurfColorSelected, 0.5f);
  mOutlineColorUnselected = CColor::Lerp(CColor::White(), mSurfColorUnselected, 0.5f);

  for (int i = 0; i < mAreaDatas.size(); ++i) {
    mCenterPoint += mAreaDatas[i].GetTransform().GetTranslation();
  }
  mCenterPoint *= 1.f / static_cast< float >(mAreaDatas.size());
}

const CMapUniverse::CMapWorldData& CMapUniverse::GetMapWorldDataByWorldId(CAssetId id) {
  for (int i = 0; i < GetNumMapWorldDatas(); ++i) {
    const CMapWorldData& world = GetMapWorldData(i);
    if (world.GetWorldAssetId() == id) {
      return world;
    }
  }

  return mWorldDatas[0];
}

CFactoryFnReturn FMapUniverseFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer) {
  in.Get< uint >();
  uint version = in.Get< uint >();
  return rs_new CMapUniverse(in, version);
}
