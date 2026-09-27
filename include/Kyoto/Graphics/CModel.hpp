#ifndef _CMODEL
#define _CMODEL

#include "types.h"

#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CAABox;
class CCubeModel;
class CModelFlags;
class IObjectStore;

class CModel {
  struct SShader;
  friend struct SShader;

  // Echoes tracks texture timeouts per material set instead of per model.
  struct SShader {
    rstl::vector< TCachedToken< CTexture > > mTextures;
    uchar* mData;
    CModel* mOwner;
    SShader* mPrev;
    SShader* mNext;

    SShader(uchar* data, CModel* owner);
    SShader(const SShader& other);
    ~SShader();

    void UnlockTextures();
    void RemoveFromList();
    void MoveToThisFrameList();
  };

  static uint sTotalMemory;
  static SShader* sThisFrameList;
  static SShader* sOneFrameList;
  static SShader* sTwoFrameList;

public:
  enum EDrawFlatFlags {
    kDF_Unknown0,
  };

  CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store);
  ~CModel();
  void Touch(int) const;
  void Draw(const CModelFlags&) const;
  void Draw(u64 mask, const CModelFlags& flags) const;
  void DrawUnsortedParts(const CModelFlags& flags) const;
  void DrawSortedParts(const CModelFlags& flags) const;
  void DolphinDrawFlat(EDrawFlatFlags flags) const;
  void PreDrawModel(const CModelFlags& flags) const;
  bool IsLoaded(int matIdx) const;
  const CAABox& GetAABB() const;
  const float* GetPositions() const;
  const float* GetNormals() const;
  const CCubeModel* GetModelInstance() const { return mModelInstance.get(); }
  void UpdateLastFrame() const;
  void VerifyCurrentShader(int shader) const;
  // Retail buffer relocation methods; names are inferred from their implementations.
  rstl::auto_ptr< uchar > GetData();
  uint GetDataSize() const;
  void RemapData(uchar* data);

  static void DisableTextureTimeout();
  static void EnableTextureTimeout();
  static void FrameDone();
  static void AddToTotal(uint amt) { sTotalMemory += amt; }
  static void RemoveFromTotal(uint amt) { sTotalMemory -= amt; }
  static uint GetTotalMemory() { return sTotalMemory; }

private:
  void* SetupSkinMatrices() const;

  rstl::single_ptr< uchar > mData;
  uint mDataLen;
  rstl::vector< void* > mSurfaces;
  mutable rstl::vector< SShader > mMatSets;
  rstl::single_ptr< CCubeModel > mModelInstance;
  mutable uint mLastFrame;
  mutable uint mCurrentMatxIdx : 16;
  uint x30_16_ : 1;
  uint mHasSkinMatrices : 1;
};
CHECK_SIZEOF(CModel, 0x34)

const CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr,
                                     int len, const CVParamTransfer& xfer);

#endif // _CMODEL
