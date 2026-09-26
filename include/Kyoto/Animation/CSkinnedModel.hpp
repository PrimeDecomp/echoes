#ifndef _CSKINNEDMODEL
#define _CSKINNEDMODEL

#include "types.h"

#include "Kyoto/TToken.hpp"

class CModel;
class CSkinRules;
class CCharLayoutInfo;

class CSkinnedModel {
public:
  CSkinnedModel(const TLockedToken< CModel >& model, const TLockedToken< CSkinRules >& skinRules,
                const TLockedToken< CCharLayoutInfo >& layoutInfo);
  ~CSkinnedModel();

  static void ClearPointGeneratorFunc();

  TLockedToken< CModel >& Model() { return mModel; }
  const TLockedToken< CModel >& GetModel() const { return mModel; }
  const TLockedToken< CCharLayoutInfo >& GetLayoutInfo() const { return mLayoutInfo; }

  static void SetPointGeneratorFunc(void*,
                                    void (*)(void*, const CVector3f*, const CVector3f*, int));

private:
  TLockedToken< CModel > mModel;
  TLockedToken< CSkinRules > mSkinRules;
  TLockedToken< CCharLayoutInfo > mLayoutInfo;
};
CHECK_SIZEOF(CSkinnedModel, 0x24)

#endif // _CSKINNEDMODEL
