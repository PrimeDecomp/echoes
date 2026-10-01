#ifndef _CRELFILE
#define _CRELFILE

#include "types.h"

#include "Kyoto/CRelFileDebugInfo.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

class CDvdRequest;
struct OSModuleHeader;

// Guessed class and method names. A reference-counted REL module on disc.
class CRelFile {
public:
  enum EState {
    kS_Loading,
    kS_Loaded,
    kS_Cancelling,
    kS_Unloaded,
  };

  CRelFile(const rstl::string& name);
  ~CRelFile();

  const rstl::string& GetFileName() const { return mName; }
  bool IsDeletable() const;
  bool IsLoaded() const;
  void RemoveLoadRequest();
  void AddLoadRequest();
  void Update();
  short RemoveReference();
  void AddReference();
  int GetReferenceCount() const { return mReferenceCount; }
  int GetLoadRequestCount() const { return mLoadRequestCount; }

private:
  void FreeData();
  bool StartLoad();
  void Unlink();
  void Link();

  rstl::string mName;
  rstl::single_ptr< CDvdRequest > mDvdRequest;
  rstl::single_ptr< uchar > mData;
  uint mDataSize;
  OSModuleHeader* mModule;
  short mReferenceCount;
  short mLoadRequestCount;
  EState mState;
  CRelFileDebugInfo mDebugInfo;
};
CHECK_SIZEOF(CRelFile, 0x3c)

#endif // _CRELFILE
