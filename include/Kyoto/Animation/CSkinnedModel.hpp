#ifndef _CSKINNEDMODEL
#define _CSKINNEDMODEL

#include "types.h"

#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"

class CModel;
class CSkinRules;
class CCharLayoutInfo;

class CSkinnedModel {
public:
  virtual ~CSkinnedModel();

  static void ClearPointGeneratorFunc();

  TLockedToken< CModel >& Model() { return mModel; }
  const TLockedToken< CModel >& GetModel() const { return mModel; }

  static void SetPointGeneratorFunc(void*, void (*)(void*, const CVector3f*, const CVector3f*, int));

private:
  TLockedToken< CModel > mModel;
  TLockedToken< CSkinRules > mSkinRules;
  TLockedToken< CCharLayoutInfo > mLayoutInfo;
  rstl::auto_ptr< float[] > mVertWorkspace;
  rstl::auto_ptr< float[] > mNormalWorkspace;
  bool mOwned;
  bool mDisableWorkspaces;
};

#endif // _CSKINNEDMODEL
