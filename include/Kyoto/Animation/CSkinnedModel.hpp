#ifndef _CSKINNEDMODEL
#define _CSKINNEDMODEL

#include "types.h"

#include "Kyoto/TToken.hpp"

class CModel;
class CSkinRules;
class CCharLayoutInfo;
class CModelFlags;
class CTransform4f;
class CVector3f;
class CPoseAsTransforms_Linear;
struct SSkinningMatrices;

// Reconstructed workspace grouping; the field offsets are established by G2ME01.
struct SSkinningWorkspace {
  CTransform4f* mTransforms;
  SSkinningMatrices* mMatrices;
  int mBoneCount;
  mutable bool mOwned;
  bool mTransient;
  bool mUniformScale;
};
CHECK_SIZEOF(SSkinningWorkspace, 0x10)

class CSkinnedModelState {
  friend class CSkinnedModel;

public:
  CSkinnedModelState(int boneCount, bool transient);
  CSkinnedModelState(const CSkinnedModelState& other);
  ~CSkinnedModelState();

  const SSkinningWorkspace& GetWorkspace() const { return mWorkspace; }

private:
  CSkinnedModelState& operator=(const CSkinnedModelState&);
  SSkinningWorkspace mWorkspace;
};
CHECK_SIZEOF(CSkinnedModelState, 0x10)

class CSkinnedModel {
public:
  typedef void (*TPointGenFunc)(const CSkinnedModel&, const SSkinningWorkspace&, void*);
  typedef void (*TDrawFunc)(const SSkinningWorkspace&, void*);
  // Guessed flag names.
  enum EDrawFlags { kDF_Unsorted = 2, kDF_Sorted = 4, kDF_Flat = 8 };

  CSkinnedModel(const TLockedToken< CModel >& model, const TLockedToken< CSkinRules >& skinRules,
                const TLockedToken< CCharLayoutInfo >& layoutInfo);
  ~CSkinnedModel();

  static void ClearPointGeneratorFunc();

  TLockedToken< CModel >& Model() { return mModel; }
  const TLockedToken< CModel >& GetModel() const { return mModel; }
  const TLockedToken< CCharLayoutInfo >& GetLayoutInfo() const { return mLayoutInfo; }
  const TLockedToken< CSkinRules >& GetSkinRules() const { return mSkinRules; }

  CSkinnedModelState MakeDefaultStorage() const;
  CSkinnedModelState MakeStorage(bool transient) const; // Guessed name.
  void StoreCalculation(CSkinnedModelState& state, const CPoseAsTransforms_Linear* pose) const;
  void Draw(const CPoseAsTransforms_Linear* pose, const CModelFlags& flags) const;
  void Draw(const CPoseAsTransforms_Linear* pose, TDrawFunc callback, void* context) const;
  void DolphinDrawWithFlags(const CPoseAsTransforms_Linear* pose, uint drawFlags,
                            const CModelFlags& flags) const;
  void DrawFromState(const CSkinnedModelState& state, const CModelFlags& flags) const;

  // Guessed names for the workspace draw overloads and sampling helpers.
  void DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                const CModelFlags& flags) const;
  void DolphinDrawFromWorkspace(const SSkinningWorkspace& workspace, uint drawFlags,
                                const CModelFlags& flags, u64 mask) const;
  CVector3f GetSkinnedPosition(const SSkinningWorkspace& workspace, int vertex) const;
  CVector3f GetSkinnedNormal(const SSkinningWorkspace& workspace, int vertex) const;

  static void SetPointGeneratorFunc(void* context, TPointGenFunc callback);

private:
  // Guessed names; these have no original GameCube symbol evidence.
  void LoadMatrixBank(int& currentBank, int bank) const;
  void BuildSkinningMatrices(const CTransform4f* transforms, SSkinningMatrices* matrices,
                             bool uniformScale) const;
  template < class TDraw, class TFlatDraw >
  void DolphinDrawInternal(const SSkinningWorkspace& workspace, uint drawFlags,
                           const CModelFlags& flags, const TDraw& draw,
                           const TFlatDraw& flat) const;

  static TPointGenFunc sPointGen;
  static void* sPointGenData;

  TLockedToken< CModel > mModel;
  TLockedToken< CSkinRules > mSkinRules;
  TLockedToken< CCharLayoutInfo > mLayoutInfo;
};
CHECK_SIZEOF(CSkinnedModel, 0x24)

#endif // _CSKINNEDMODEL
