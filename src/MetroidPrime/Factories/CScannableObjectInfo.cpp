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

void CScannableObjectInfo::ReadProperties(CInputStream& in) {
  SLdrScannableObjectInfo data;
  const ushort count = in.ReadUint16();
  for (int i = 0; i < count; ++i) {
    const int property = in.ReadInt32();
    const ushort size = in.ReadUint16();
    switch (property) {
    case 0x2f5b6423:
      data.scanInfoTextStringTable = in.ReadInt32();
      break;
    case 0xc308a322:
      data.unknown_0xc308a322.value = in.ReadInt32();
      break;
    case 0x7b714814:
      data.critical = in.ReadBool();
      break;
    case 0x1733b1ec:
      data.unknown_0x1733b1ec = in.ReadBool();
      break;
    case 0x53336141:
      data.scanTextureInHud = in.ReadInt32();
      break;
    case 0x3de0ba64:
      data.modelInitialPitch = in.ReadFloat();
      break;
    case 0x2add6628:
      data.modelInitialYaw = in.ReadFloat();
      break;
    case 0xd0c15066:
      data.modelScale = in.ReadFloat();
      break;
    case 0xb7adc418:
      data.staticModel = in.ReadInt32();
      break;
    case 0x15694ee1:
      LoadTypedefSLdrAnimationParameters(data.animatedModel, in);
      break;
    case 0x58f9fe99:
      LoadTypedefSLdrAnimationParameters(data.primarySecondAnimatedModel, in);
      break;
    case 0x1c5b4a3a:
      LoadTypedefSLdrScanInfoSecondaryModel(data.secondaryModel0, in);
      break;
    case 0x8728a0ee:
      LoadTypedefSLdrScanInfoSecondaryModel(data.secondaryModel1, in);
      break;
    case 0xf1cd99d3:
      LoadTypedefSLdrScanInfoSecondaryModel(data.secondaryModel2, in);
      break;
    case 0x6abe7307:
      LoadTypedefSLdrScanInfoSecondaryModel(data.secondaryModel3, in);
      break;
    case 0x1c07eba9:
      LoadTypedefSLdrScanInfoSecondaryModel(data.secondaryModel4, in);
      break;
    case 0x8774017d:
      LoadTypedefSLdrScanInfoSecondaryModel(data.secondaryModel5, in);
      break;
    case 0xf1913840:
      LoadTypedefSLdrScanInfoSecondaryModel(data.secondaryModel6, in);
      break;
    case 0x6ae2d294:
      LoadTypedefSLdrScanInfoSecondaryModel(data.secondaryModel7, in);
      break;
    case 0x1ce2091c:
      LoadTypedefSLdrScanInfoSecondaryModel(data.secondaryModel8, in);
      break;
    default:
      in.ReadBytes(nullptr, size);
      break;
    }
  }

  mStringTableId = data.scanInfoTextStringTable;
  mTotalDownloadTime = gpTweakGui->GetScanSpeed(data.unknown_0xc308a322.value);
  mCritical = data.critical;
  mUseScanModel = data.unknown_0x1733b1ec;
  mScanTextureId = data.scanTextureInHud;

  mStaticModels.push_back(data.staticModel);
  mAnimatedModels.push_back(data.animatedModel.ancs);
  mCharacterIndices.push_back(data.animatedModel.character_index);
  mAnimationIndices.push_back(data.animatedModel.initial_anim);

  mStaticModels.push_back(kInvalidAssetId);
  mAnimatedModels.push_back(data.primarySecondAnimatedModel.ancs);
  mCharacterIndices.push_back(data.primarySecondAnimatedModel.character_index);
  mAnimationIndices.push_back(data.primarySecondAnimatedModel.initial_anim);

  const SLdrScanInfoSecondaryModel* secondaryModels[] = {
      &data.secondaryModel0, &data.secondaryModel1, &data.secondaryModel2,
      &data.secondaryModel3, &data.secondaryModel4, &data.secondaryModel5,
      &data.secondaryModel6, &data.secondaryModel7, &data.secondaryModel8,
  };
  for (int i = 0; i < 9; ++i) {
    const SLdrScanInfoSecondaryModel& model = *secondaryModels[i];
    mStaticModels.push_back(model.secondaryStaticModel);
    mAnimatedModels.push_back(model.secondaryAnimatedModel.ancs);
    mCharacterIndices.push_back(model.secondaryAnimatedModel.character_index);
    mAnimationIndices.push_back(model.secondaryAnimatedModel.initial_anim);
    mModelLocators.push_back(model.secondaryModelLocator);
  }

  mModelInitialPitch = data.modelInitialPitch;
  mModelInitialYaw = data.modelInitialYaw;
  mModelScale = data.modelScale;
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
