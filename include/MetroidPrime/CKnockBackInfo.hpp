#ifndef _CKNOCKBACKINFO
#define _CKNOCKBACKINFO

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CKnockBackInfo {
public:
  CKnockBackInfo(const CVector3f& direction, TUniqueId source, TUniqueId owner,
                 const CDamageInfo& damage, bool direct);

  const CVector3f& GetDirection() const { return mDirection; }
  TUniqueId GetSourceId() const { return mSourceId; }
  TUniqueId GetOwnerId() const { return mOwnerId; }
  const CDamageInfo& GetDamageInfo() const { return mDamageInfo; }
  bool IsDirect() const { return mDirect; }

private:
  CVector3f mDirection;
  // Guessed roles; both IDs are forwarded to the follow-up effect.
  TUniqueId mSourceId;
  TUniqueId mOwnerId;
  CDamageInfo mDamageInfo;
  bool mDirect : 1;
};
CHECK_SIZEOF(CKnockBackInfo, 0x30)

#endif
