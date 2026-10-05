#ifndef _CSCANNABLEOBJECTINFO
#define _CSCANNABLEOBJECTINFO

#include "Kyoto/SObjectTag.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CAnimRes;
class CFactoryFnReturn;
class CInputStream;
class CModelData;
class CVParamTransfer;

class CScannableObjectInfo {
public:
  CScannableObjectInfo(CInputStream& in, CAssetId id);
  CScannableObjectInfo(CAssetId id, CAssetId stringTable, float downloadTime, bool critical,
                       bool useScanModel);

  CAssetId GetScannableObjectId() const { return mScannableObjectId; }
  CAssetId GetStringTableId() const { return mStringTableId; }
  CAssetId GetScanTextureId() const { return mScanTextureId; }
  CAssetId GetStaticModelId(int index) const { return mStaticModels[index]; }
  CAssetId GetAnimatedModelId(int index) const { return mAnimatedModels[index]; }
  float GetTotalDownloadTime() const;
  bool IsCritical() const { return mCritical; }
  bool UsesScanModel() const { return mUseScanModel; }
  // Guessed names.
  rstl::auto_ptr< CModelData > CreateModel(int index) const;
  rstl::auto_ptr< CModelData > CreateStaticModel(int index) const;
  CAnimRes GetAnimationResource(int index) const;
  const rstl::string& GetModelLocator(int index) const;
  const rstl::vector< SObjectTag >& GetDependencies() const { return mDependencies; }
  float GetModelScale() const { return mModelScale; }
  float GetModelInitialPitch() const { return mModelInitialPitch; }
  float GetModelInitialYaw() const { return mModelInitialYaw; }

private:
  // Guessed names.
  void ReadLegacy(CInputStream& in, uint version);
  void ReadProperties(CInputStream& in);

  CAssetId mScannableObjectId;
  CAssetId mStringTableId;
  CAssetId mScanTextureId;
  rstl::reserved_vector< CAssetId, 11 > mStaticModels;
  rstl::reserved_vector< CAssetId, 11 > mAnimatedModels;
  rstl::reserved_vector< int, 11 > mCharacterIndices;
  rstl::reserved_vector< int, 11 > mAnimationIndices;
  rstl::vector< SObjectTag > mDependencies;
  rstl::reserved_vector< rstl::string, 11 > mModelLocators;
  float mTotalDownloadTime;
  float mModelScale;
  float mModelInitialPitch;
  float mModelInitialYaw;
  bool mCritical : 1;
  bool mUseScanModel : 1;
};
CHECK_SIZEOF(CScannableObjectInfo, 0x1a4)

CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag& tag, CInputStream& in,
                                             const CVParamTransfer& xfer);

#endif // _CSCANNABLEOBJECTINFO
