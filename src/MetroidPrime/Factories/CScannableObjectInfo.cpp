#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/ScriptLoader/SLdrScannableObjectInfo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"

namespace {
const CAssetId skInvalidModelId = kInvalidAssetId;
}

void CScannableObjectInfo::ReadProperties(CInputStream& input) {
  SLdrScannableObjectInfo sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrScannableObjectInfo.inc"

  mStringTableId = sldrThis.scanInfoTextStringTable;
  mTotalDownloadTime = gpTweakGui->GetScanSpeed(sldrThis.scanSpeed);
  mCritical = sldrThis.critical;
  mUseScanModel = sldrThis.unknown_0x1733b1ec;
  mScanTextureId = sldrThis.scanTextureInHud;

  mStaticModels.push_back(sldrThis.staticModel);
  mAnimatedModels.push_back(sldrThis.animatedModel.ancs);
  mCharacterIndices.push_back(sldrThis.animatedModel.character_index);
  mAnimationIndices.push_back(sldrThis.animatedModel.initial_anim);

  mStaticModels.push_back(kInvalidAssetId);
  mAnimatedModels.push_back(sldrThis.primarySecondAnimatedModel.ancs);
  mCharacterIndices.push_back(sldrThis.primarySecondAnimatedModel.character_index);
  mAnimationIndices.push_back(sldrThis.primarySecondAnimatedModel.initial_anim);

  const SLdrScanInfoSecondaryModel* secondaryModels[] = {
      &sldrThis.secondaryModel0, &sldrThis.secondaryModel1, &sldrThis.secondaryModel2,
      &sldrThis.secondaryModel3, &sldrThis.secondaryModel4, &sldrThis.secondaryModel5,
      &sldrThis.secondaryModel6, &sldrThis.secondaryModel7, &sldrThis.secondaryModel8,
  };
  for (int i = 0; i < 9; ++i) {
    const SLdrScanInfoSecondaryModel& model = *secondaryModels[i];
    mStaticModels.push_back(model.secondaryStaticModel);
    mAnimatedModels.push_back(model.secondaryAnimatedModel.ancs);
    mCharacterIndices.push_back(model.secondaryAnimatedModel.character_index);
    mAnimationIndices.push_back(model.secondaryAnimatedModel.initial_anim);
    mModelLocators.push_back(model.secondaryModelLocator);
  }

  mModelInitialPitch = sldrThis.modelInitialPitch;
  mModelInitialYaw = sldrThis.modelInitialYaw;
  mModelScale = sldrThis.modelScale;
}

CScannableObjectInfo::CScannableObjectInfo(CInputStream& in, CAssetId id)
: mScannableObjectId(id)
, mStringTableId(kInvalidAssetId)
, mTotalDownloadTime(gpTweakGui->GetScanSpeed(0))
, mCritical(false) {
  const uint version = in.ReadInt32();
  if (version <= 5) {
    ReadLegacy(in, version);
  } else {
    const int scriptVersion = in.ReadInt32() & 0xff;
    in.ReadUint8();
    if (scriptVersion > 1) {
      in.ReadInt32();
    }

    // Skip the script-object envelope and connections before its properties.
    in.ReadInt32();
    in.ReadUint16();
    in.ReadInt32();
    const ushort connectionCount = in.ReadUint16();
    for (int i = 0; i < connectionCount; ++i) {
      in.ReadInt32();
      in.ReadInt32();
      in.ReadInt32();
    }
    in.ReadInt32();
    in.ReadUint16();
    ReadProperties(in);

    if (scriptVersion > 1) {
      mDependencies = rstl::vector< SObjectTag >(in);
    }
  }
}

CScannableObjectInfo::CScannableObjectInfo(CAssetId id, CAssetId stringTable, float downloadTime,
                                           const bool critical, bool useScanModel)
: mScannableObjectId(id)
, mStringTableId(stringTable)
, mScanTextureId(kInvalidAssetId)
, mStaticModels(3, skInvalidModelId)
, mAnimatedModels(3, skInvalidModelId)
, mTotalDownloadTime(downloadTime)
, mModelScale(0.f)
, mModelInitialPitch(0.f)
, mModelInitialYaw(0.f)
, mCritical(critical)
, mUseScanModel(useScanModel) {}

void CScannableObjectInfo::ReadLegacy(CInputStream& in, uint version) {
  in.ReadInt32();
  in.ReadInt32();
  mStringTableId = in.ReadInt32();
  if (version < 4) {
    mTotalDownloadTime = in.ReadFloat();
  } else {
    mTotalDownloadTime = gpTweakGui->GetScanSpeed(in.ReadInt32());
  }

  if (version > 4) {
    mCritical = in.ReadBool();
  }
}

rstl::auto_ptr< CModelData > CScannableObjectInfo::CreateModel(int index) const {
  const FourCC staticType = gpResourceFactory->GetResourceTypeById(mStaticModels[index]);
  const FourCC animatedType = gpResourceFactory->GetResourceTypeById(mAnimatedModels[index]);
  if (staticType == 0 && animatedType == 0) {
    return rs_new CModelData(CModelData::CModelDataNull());
  }
  if (animatedType == 'ANCS') {
    return rs_new CModelData(GetAnimationResource(index));
  }

  static const CVector3f unitScale(1.f, 1.f, 1.f);
  return rs_new CModelData(CStaticRes(mStaticModels[index], unitScale));
}

rstl::auto_ptr< CModelData > CScannableObjectInfo::CreateStaticModel(int index) const {
  static const CVector3f unitScale(1.f, 1.f, 1.f);
  return rs_new CModelData(CStaticRes(mStaticModels[index], unitScale));
}

CAnimRes CScannableObjectInfo::GetAnimationResource(int index) const {
  static const CVector3f unitScale(1.f, 1.f, 1.f);
  return CAnimRes(mAnimatedModels[index], mCharacterIndices[index], unitScale,
                  mAnimationIndices[index], true);
}

float CScannableObjectInfo::GetTotalDownloadTime() const { return mTotalDownloadTime; }

const rstl::string& CScannableObjectInfo::GetModelLocator(int index) const {
  return mModelLocators[index];
}

CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag& tag, CInputStream& in,
                                             const CVParamTransfer& xfer) {
  return rs_new CScannableObjectInfo(in, tag.GetId());
}
