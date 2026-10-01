#ifndef _CRELFILE
#define _CRELFILE

#include "types.h"

#include "rstl/string.hpp"

// Guessed class and method names. A reference-counted REL module on disc.
class CRelFile {
public:
  enum EState {
    kS_Unloaded = 3,
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
  rstl::string mName;
  void* x10_;
  void* x14_;
  uint x18_;
  uint x1c_;
  short mReferenceCount;
  short mLoadRequestCount;
  int mState;
  uchar x28_[0x14];
};
CHECK_SIZEOF(CRelFile, 0x3c)

#endif // _CRELFILE
