#include "MetroidPrime/CMapArea.hpp"

#include "MetroidPrime/CMappableObject.hpp"
#include "MetroidPrime/CMemoryDrawEnum.hpp"
#include "MetroidPrime/IGameArea.hpp"
#include "MetroidPrime/IWorld.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"

#include "Kyoto/Alloc/CCallStack.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"

#include <dolphin/gx.h>
#include <dolphin/mtx.h>
#include <dolphin/os.h>

static Mtx sViewMtx;

CMapArea::CMapArea(CInputStream& in, uint size)
: mMagic(in.ReadInt32())
, mVersion(in.ReadInt32())
, mFlags(in.ReadInt32())
, mVisibilityMode(static_cast< EVisMode >(in.ReadInt32()))
, mBox(in)
, mMapAdjustment(mVersion >= 3 ? CVector3f(in) : CVector3f::Zero())
, mMappableObjCount(in.ReadInt32())
, mVertexCount(in.ReadInt32())
, mSurfaceCount(in.ReadInt32()) {
  mSize = size - 52;
  if (mVersion >= 3) {
    mSize -= 12;
  }
  mBuf = rs_new uchar[mSize];
  in.Get(mBuf.get(), mSize);
  PostConstruct();
  BuildDisplayLists();

  DCFlushRange(mVertexStart, mVertexCount * 0xc);
  CMemoryDrawEnum::AddWorldMemory(sizeof(*this) + mSize + mRenderBufSize);
}

CMapArea::~CMapArea() {
  CMemoryDrawEnum::SubtractWorldMemory(sizeof(*this) + mSize + mRenderBufSize);
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame, mBuf.release());
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        mRenderBuf.release());
}

void CMapArea::PostConstruct() {
  uchar* moStart = mBuf.get();
  mMoStart = reinterpret_cast< CMappableObject* >(mBuf.get());
  moStart += mMappableObjCount * sizeof(CMappableObject);
  mVertexStart = reinterpret_cast< CVector3f* >(moStart);
  moStart += mVertexCount * sizeof(CVector3f);
  mSurfaceStart = reinterpret_cast< CMapAreaSurface* >(moStart);

  for (int i = 0; i < mMappableObjCount; ++i) {
    mMoStart[i].PostConstruct(mBuf.get());
  }
  float* floatStart = reinterpret_cast< float* >(mVertexStart);
  for (int i = 0; i < mVertexCount * 3; ++i) {
    floatStart[i] = CBasics::SwapBytes(floatStart[i]);
  }
  for (int i = 0; i < mSurfaceCount; ++i) {
    mSurfaceStart[i].PostConstruct(mBuf.get());
  }
}

void CMapArea::BuildDisplayLists() {
  mSurfaceDisplayListSize = 0;
  mOutlineDisplayListSize = 0;

  int numDoors = 0;
  for (int i = 0; i < mMappableObjCount; ++i) {
    if (CMappableObject::IsDoorType(mMoStart[i].GetType())) {
      ++numDoors;
    }
  }

  for (int i = 0; i < mSurfaceCount; ++i) {
    const CMapAreaSurface& surf = mSurfaceStart[i];
    const int* surface = surf.mSurfOffset;

    int numSurfaces = *surface++;
    for (int j = 0; j < numSurfaces; ++j) {
      surface++;
      int numVertices = *surface++;
      surface += ((numVertices + 3) & ~3) / 4;
      mSurfaceDisplayListSize += 2;
      mSurfaceDisplayListSize += 2;
      mSurfaceDisplayListSize += numVertices * 2;
    }

    const int* outline = surf.mOutlineOffset;
    int numOutlines = *outline++;
    for (int j = 0; j < numOutlines; ++j) {
      int numVertices = *outline++;
      outline += ((numVertices + 3) & ~3) / 4;
      mOutlineDisplayListSize += 2;
      mOutlineDisplayListSize += 2;
      mOutlineDisplayListSize += numVertices;
    }
  }

  mDoorVerticesSize = (numDoors * sizeof(CMappableObject::skDoorVerts) + 31) & ~31;
  mSurfaceNormalsSize = (mSurfaceCount * sizeof(CVector3f) + 31) & ~31;
  mSurfaceDisplayListSize = (mSurfaceDisplayListSize + 31) & ~31;
  mOutlineDisplayListSize = (mOutlineDisplayListSize + 31) & ~31;
  mRenderBufSize = mDoorVerticesSize + mSurfaceNormalsSize + mSurfaceDisplayListSize +
                   mOutlineDisplayListSize;
  mRenderBuf = static_cast< uchar* >(CMemory::Alloc(mRenderBufSize, IAllocator::kHI_RoundUpLen,
                                                    IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                                                    CCallStack(-1, "??(??)")));

  CMemoryStreamOut out(mRenderBuf.get(), mRenderBufSize);
  mDoorVertices = mRenderBuf.get();
  mSurfaceNormals = mDoorVertices + mDoorVerticesSize;
  mSurfaceDisplayList = mSurfaceNormals + mSurfaceNormalsSize;

  for (int i = 0; i < mMappableObjCount; ++i) {
    const CMappableObject& obj = mMoStart[i];
    if (CMappableObject::IsDoorType(obj.GetType())) {
      const CTransform4f& xf = obj.GetTransform();
      for (const CVector3f* v = CMappableObject::skDoorVerts;
           v != CMappableObject::skDoorVerts + 8; ++v) {
        (mMapAdjustment + xf * *v).PutTo(out);
      }
    }
  }
  while (out.GetWrittenBytes() & 31) {
    out.WriteUint8(0);
  }

  for (int i = 0; i < mSurfaceCount; ++i) {
    mSurfaceStart[i].GetNormal().PutTo(out);
  }
  while (out.GetWrittenBytes() & 31) {
    out.WriteUint8(0);
  }

  for (int i = 0; i < mSurfaceCount; ++i) {
    const uchar normalIdx = i;
    int numSurfaces = *mSurfaceStart[i].mSurfOffset;
    const int* surface = &mSurfaceStart[i].mSurfOffset[1];
    for (int j = 0; j < numSurfaces; ++j) {
      uint primitive = *surface++;
      int numVertices = *surface++;
      const uchar* data = reinterpret_cast< const uchar* >(surface);
      surface += ((numVertices + 3) & ~3) / 4;

      out.WriteUint8(0);
      out.WriteUint8(primitive);
      out.WriteShort(numVertices);
      for (int v = 0; v < numVertices; ++v) {
        out.WriteUint8(data[v]);
        out.WriteUint8(normalIdx);
      }
    }
  }
  while (out.GetWrittenBytes() & 31) {
    out.WriteUint8(0);
  }

  mOutlineDisplayList = mSurfaceDisplayList + mSurfaceDisplayListSize;
  for (int i = 0; i < mSurfaceCount; ++i) {
    int numOutlines = *mSurfaceStart[i].mOutlineOffset;
    const int* outline = &mSurfaceStart[i].mOutlineOffset[1];
    for (int j = 0; j < numOutlines; ++j) {
      int numVertices = *outline++;
      const uchar* data = reinterpret_cast< const uchar* >(outline);
      outline += ((numVertices + 3) & ~3) / 4;

      out.WriteUint8(0);
      out.WriteUint8(GX_LINESTRIP | GX_VTXFMT0);
      out.WriteShort(numVertices);
      out.Put(data, numVertices);
    }
  }
  while (out.GetWrittenBytes() & 31) {
    out.WriteUint8(0);
  }
}

bool CMapArea::IsInDarkWorld() const { return mFlags & 1; }

bool CMapArea::GetIsVisibleToAutoMapper(bool worldVis, bool areaVis) const {
  switch (mVisibilityMode) {
  case kVM_Always:
    return true;
  case kVM_MapStationOrVisit:
    return worldVis || areaVis;
  case kVM_Visit:
    return areaVis;
  case kVM_Never:
    return false;
  default:
    return true;
  }
}

CVector3f CMapArea::GetAreaCenterPoint() const { return mBox.GetCenterPoint(); }

void CMapArea::CMapAreaSurface::PostConstruct(const void* buf) {
  mSurfOffset = reinterpret_cast< const int* >(static_cast< const uchar* >(buf) +
                                               reinterpret_cast< uint >(mSurfOffset));
  mOutlineOffset = reinterpret_cast< const int* >(static_cast< const uchar* >(buf) +
                                                  reinterpret_cast< uint >(mOutlineOffset));

  int numSurfaces = CBasics::SwapBytes(*mSurfOffset);
  const int* surfOffset = mSurfOffset + 1;
  for (int i = 0; i < numSurfaces; ++i) {
    int numVertices = CBasics::SwapBytes(*++surfOffset);
    surfOffset++; // skip primitive type
    surfOffset += ((numVertices + 3) & ~3) / 4;
  }

  int numOutlines = CBasics::SwapBytes(*mOutlineOffset);
  const int* outlineOffset = mOutlineOffset + 1;
  for (int i = 0; i < numOutlines; ++i) {
    int numVertices = CBasics::SwapBytes(*outlineOffset++);
    outlineOffset += ((numVertices + 3) & ~3) / 4;
  }
}

void CMapArea::CMapAreaSurface::Draw(const CVector3f* verts, const CColor& surfColor,
                                     const CColor& lineColor, float lineWidth) const {
  bool hasSurfAlpha = surfColor.GetAlpha() > 0.0f;
  bool hasLineAlpha = lineColor.GetAlpha() > 0.0f;
  int numSurfaces = *mSurfOffset;
  int numOutlines = *mOutlineOffset;
  if (verts) {
    CGX::SetArray(GX_VA_POS, verts, sizeof(CVector3f));
  }
  if (hasSurfAlpha) {
    CGX::SetTevKColor(GX_KCOLOR0, surfColor.GetGXColor());
    const int* surface = &mSurfOffset[1];
    for (int i = 0; i < numSurfaces; ++i) {
      GXPrimitive primType = static_cast< GXPrimitive >(*surface++);
      int numVertices = *surface++;
      const uchar* data = reinterpret_cast< const uchar* >(surface);
      surface += ((numVertices + 3) & ~3) / 4;

      CGX::Begin(primType, GX_VTXFMT0, numVertices);
      for (int v = 0; v < numVertices; ++v) {
        GXPosition1x8(data[v]);
      }
      CGX::End();
    }
  }
  if (hasLineAlpha) {
    bool thickLine = lineWidth > 1.f;
    for (int j = 0; j < (thickLine ? 1 : 0) + 1; ++j) {
      const int* outline = &mOutlineOffset[1];

      if (thickLine) {
        CGraphics::SetLineWidth(lineWidth - j, kTO_One);
      }
      CGX::SetTevKColor(GX_KCOLOR0,
                        lineColor.WithAlphaModulatedBy(thickLine ? 0.5f : 1.0f).GetGXColor());

      for (int i = 0; i < numOutlines; ++i) {
        int numVertices = *outline++;
        const uchar* data = reinterpret_cast< const uchar* >(outline);
        outline += ((numVertices + 3) & ~3) / 4;

        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, numVertices);
        for (int v = 0; v < numVertices; ++v) {
          GXPosition1x8(data[v]);
        }
        CGX::End();
      }
    }
  }
}

void CMapArea::CMapAreaSurface::SetupGXMaterial() {
  const GXVtxDescList list[2] = {
      {GX_VA_POS, GX_INDEX8},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(list);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
}

static GXVtxDescList sLitVtxDescList[3] = {
    {GX_VA_POS, GX_INDEX8},
    {GX_VA_NRM, GX_INDEX8},
    {GX_VA_NULL, GX_NONE},
};

static void SetupLitGXMaterial() {
  CGX::SetVtxDescv(sLitVtxDescList);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
}

static void SetupOutlineGXMaterial() {
  const GXVtxDescList list[2] = {
      {GX_VA_POS, GX_INDEX8},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(list);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
}

CTransform4f CMapArea::GetAreaPostTransform(const IWorld& world, int aid) {
  return CTransform4f::Translate(mMapAdjustment) * world.IGetAreaAlways(TAreaId(aid))->IGetTM();
}

const CVector3f& CMapArea::GetAreaPostTranslate(const IWorld& world) {
  return mMapAdjustment;
}

void CMapArea::SetupLighting(const CTransform4f& viewXf) {
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetUseNormalMatrix(false);
  CTransform4f oldView = CGraphics::GetViewMatrix();
  CGraphics::SetViewPointMatrix(viewXf);
  CGraphics::DisableAllLights();
  CColor lightColor(gpTweakAutoMapper->GetMapSurfaceNormColorLinear(), gpTweakAutoMapper->GetMapSurfaceNormColorLinear(),
                    gpTweakAutoMapper->GetMapSurfaceNormColorLinear(), 1.f);
  CLight light = CLight::BuildDirectional(viewXf.GetForward(), lightColor);
  CGraphics::LoadLight(kLight0, light);
  CGraphics::SetAmbientColor(CColor(gpTweakAutoMapper->GetMapSurfaceNormColorConstant(),
                                    gpTweakAutoMapper->GetMapSurfaceNormColorConstant(),
                                    gpTweakAutoMapper->GetMapSurfaceNormColorConstant(), 1.f));
  PSMTXCopy(CGraphics::GetCameraMtx(), sViewMtx);
  CGraphics::SetViewPointMatrix(oldView);
  CGraphics::SetUseNormalMatrix(true);
}

static GXVtxDescList sOutlineVtxDescList[2] = {
    {GX_VA_POS, GX_INDEX8},
    {GX_VA_NULL, GX_NONE},
};

void CMapArea::Draw(const CColor& surfColor, const CColor& outlineColor,
                           const CTransform4f& areaXf, const CTransform4f& modelXf, int curArea,
                           const CMapWorldInfo& mwInfo, float alpha) const {
  SetupLitGXMaterial();
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_Or, kAF_Always, 0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetArray(GX_VA_POS, mVertexStart, sizeof(CVector3f));
  CGX::SetArray(GX_VA_NRM, mSurfaceNormals, sizeof(CVector3f));
  CGX::SetTevKColor(GX_KCOLOR0, surfColor.GetGXColor());

  CGraphics::SetUseNormalMatrix(false);
  Mtx modelView;
  PSMTXConcat(sViewMtx, areaXf.GetCStyleMatrix(), modelView);
  Mtx normalMtx;
  PSMTXInvXpose(modelView, normalMtx);
  CGX::LoadNrmMtxImm(normalMtx, GX_PNMTX0);
  CGraphics::SetModelMatrix(modelXf * areaXf);
  CGraphics::EnableLight(kLight0);
  CGX::CallDisplayList(mSurfaceDisplayList, mSurfaceDisplayListSize);
  CGraphics::SetUseNormalMatrix(true);
  CGraphics::DisableAllLights();

  CGX::SetVtxDescv(sOutlineVtxDescList);
  CGX::SetTevKColor(GX_KCOLOR0, outlineColor.GetGXColor());
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  SetupOutlineGXMaterial();
  CGX::CallDisplayList(mOutlineDisplayList, mOutlineDisplayListSize);

  CGraphics::SetModelMatrix(modelXf);
  int doorOffset = 0;
  for (int i = 0; i < mMappableObjCount; ++i) {
    const CMappableObject& obj = mMoStart[i];
    if (CMappableObject::IsDoorType(obj.GetType())) {
      CGX::SetArray(GX_VA_POS, mDoorVertices + doorOffset, sizeof(CVector3f));
      obj.DrawDoor(curArea, mwInfo, alpha);
      doorOffset += 8 * sizeof(CVector3f);
    }
  }
}

static CAssetId gHackAssetId = kInvalidAssetId;

const CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in,
                                       const CVParamTransfer&) {
  gHackAssetId = objTag.GetId();
  return rs_new CMapArea(in, gpResourceFactory->ResourceSize(objTag));
}
