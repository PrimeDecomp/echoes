#ifndef _CAUDIOGROUPSET
#define _CAUDIOGROUPSET

#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

#include "Kyoto/CResFactory.hpp"

class CInputStream;
class CAudioGrpSetLoc {
public:
  CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length);
  const rstl::string& GetBaseDirName() const { return mBaseDirName; }
  const rstl::string& GetGroupSetName() const { return mGroupSetName; }
  uint AramUsage() const;
  const void* GetPool() const;
  const void* GetProject() const;
  const void* GetSample() const;
  const void* GetSampleDir() const;

private:
  rstl::auto_ptr< uchar > mData;
  rstl::auto_ptr< uchar > x8_;
  rstl::string mBaseDirName;
  rstl::string mGroupSetName;
  uint x30_;
  uint x34_;
  uint x38_;
  uint x3c_;
  uint x40_;
};

class CAudioGroupSet {
public:
  CAudioGroupSet(const TLockedToken< CAudioGrpSetLoc >& group);
  ~CAudioGroupSet();
  void Reload();
  void FreeSampleBuffer();
  void FreeAllBuffers();

private:
  rstl::string mBaseDir;
  rstl::string mGroupSetName;
  rstl::optional_object< TLockedToken< CAudioGrpSetLoc > > mGroupSetTok;
};

#endif // _CAUDIOGROUPSET
