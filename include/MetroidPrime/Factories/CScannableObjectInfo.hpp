#ifndef _CSCANNABLEOBJECTINFO
#define _CSCANNABLEOBJECTINFO

#include "Kyoto/SObjectTag.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

// Layout shared with the scan display; resource construction remains in its own TU.
class CScannableObjectInfo {
public:
  CAssetId GetScannableObjectId() const { return mScannableObjectId; }
  CAssetId GetStringTableId() const { return mStringTableId; }
  CAssetId GetScanTextureId() const { return mScanTextureId; }
  CAssetId GetStaticModelId(int index) const { return mStaticModels[index]; }
  float GetTotalDownloadTime() const;
  bool IsCritical() const { return mCritical; }
  bool UsesScanModel() const { return mUseScanModel; }

private:
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

#endif // _CSCANNABLEOBJECTINFO
