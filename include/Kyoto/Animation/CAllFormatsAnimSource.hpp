#ifndef _CALLFORMATSANIMSOURCE
#define _CALLFORMATSANIMSOURCE

#include "Kyoto/Animation/CAnimSource.hpp"
#include "Kyoto/Animation/CFBStreamedCompression.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

class CInputStream;
class IObjectStore;
class CAnimPOIData;

class CAnimFormatUnion {
public:
  // Format names reconstructed from the concrete classes selected by the target.
  enum EFormat { kF_AnimSource = 0, kF_FBStreamedCompression = 2 };

  CAnimFormatUnion(CInputStream& in, IObjectStore& store);
  ~CAnimFormatUnion();

  int GetType() const { return mFormatType; }
  const CAnimSource& AsCAnimSource() const {
    return *reinterpret_cast< const CAnimSource* >(mFormatData);
  }
  const CFBStreamedCompression& AsCFBStreamedCompression() const {
    return *reinterpret_cast< const CFBStreamedCompression* >(mFormatData);
  }

  static void SubConstruct(uchar* ptr, const uint format, CInputStream& in, IObjectStore& store);

private:
  uint mFormatType;
  uchar mFormatData[sizeof(CAnimSource) > sizeof(CFBStreamedCompression)
                        ? sizeof(CAnimSource)
                        : sizeof(CFBStreamedCompression)] ATTRIBUTE_ALIGN(4);
};
CHECK_SIZEOF(CAnimFormatUnion, 0x88)

class CAllFormatsAnimSource {
public:
  CAllFormatsAnimSource(CInputStream& in, IObjectStore& store, const SObjectTag& tag);
  ~CAllFormatsAnimSource();

  int GetType() const { return mFormatUnion.GetType(); }
  CCharAnimTime GetAnimationDuration() const {
    switch (GetType()) {
    case CAnimFormatUnion::kF_AnimSource:
      return AsCAnimSource().GetAnimationDuration();
    case CAnimFormatUnion::kF_FBStreamedCompression:
      return AsCFBStreamedCompression().GetAnimationDuration();
    default:
      return AsCAnimSource().GetAnimationDuration();
    }
  }
  float GetAverageVelocity() const {
    switch (GetType()) {
    case CAnimFormatUnion::kF_AnimSource:
      return AsCAnimSource().GetAverageVelocity();
    case CAnimFormatUnion::kF_FBStreamedCompression:
      return AsCFBStreamedCompression().GetAverageVelocity();
    default:
      return AsCAnimSource().GetAverageVelocity();
    }
  }
  const CAnimSource& AsCAnimSource() const { return mFormatUnion.AsCAnimSource(); }
  const CFBStreamedCompression& AsCFBStreamedCompression() const {
    return mFormatUnion.AsCFBStreamedCompression();
  }
  void GetFormatPointer(const CAnimSource*& ptr) const { ptr = &AsCAnimSource(); }
  void GetFormatPointer(const CFBStreamedCompression*& ptr) const {
    ptr = &AsCFBStreamedCompression();
  }

  static rstl::ownership_transfer< IAnimReader >
  GetNewReader(const TLockedToken< CAllFormatsAnimSource >& tok, const CCharAnimTime& time,
               const CAnimPOIData* poiData);

private:
  CAnimFormatUnion mFormatUnion;
  CVector3f x88_;
  SObjectTag mTag;
};
CHECK_SIZEOF(CAllFormatsAnimSource, 0x9c)

template < typename T >
inline TSubAnimTypeToken< T >::TSubAnimTypeToken(const TLockedToken< CAllFormatsAnimSource >& token)
: mToken(token) {
  mToken->GetFormatPointer(mSource);
}

CFactoryFnReturn AnimSourceFactory(const SObjectTag& tag, CInputStream& in,
                                   const CVParamTransfer& param);

#endif // _CALLFORMATSANIMSOURCE
