#ifndef _CTEAMAIROLE
#define _CTEAMAIROLE

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CScriptTeamAiMgr;

class CTeamAiRole {
public:
  enum ETeamAiRole {
    kTAR_Invalid = -1,
    kTAR_Initial = 0,
    kTAR_Melee = 1,
    kTAR_Projectile = 2,
    kTAR_Unknown = 3,
    kTAR_Unassigned = 4
  };

  CTeamAiRole(TUniqueId ownerId, ETeamAiRole roleA = kTAR_Invalid, ETeamAiRole roleB = kTAR_Invalid,
              ETeamAiRole roleC = kTAR_Invalid, int captainPriority = 0)
  : mOwnerId(ownerId)
  , mRoleA(roleA)
  , mRoleB(roleB)
  , mRoleC(roleC)
  , mCurRole(kTAR_Invalid)
  , mRoleIndex(-1)
  , mCaptainPriority(captainPriority)
  , mPosition(CVector3f::Zero())
  , mTargetId(kInvalidUniqueId) {}

  bool operator<(const CTeamAiRole& other) const {
    return mOwnerId.Value() < other.mOwnerId.Value();
  }

  bool AllowsRole(ETeamAiRole role) const {
    return mRoleA == role || mRoleB == role || mRoleC == role;
  }

  bool HasTeamAiRole() const {
    return mCurRole != kTAR_Initial && mCurRole >= kTAR_Initial && mCurRole <= kTAR_Unassigned;
  }

  TUniqueId GetOwnerId() const { return mOwnerId; }
  ETeamAiRole GetTeamAiRole() const { return mCurRole; }
  int GetRoleIndex() const { return mRoleIndex; }
  const CVector3f& GetTeamPosition() const { return mPosition; }
  TUniqueId GetTargetId() const { return mTargetId; }

private:
  TUniqueId mOwnerId;
  ETeamAiRole mRoleA;
  ETeamAiRole mRoleB;
  ETeamAiRole mRoleC;
  ETeamAiRole mCurRole;
  int mRoleIndex;
  int mCaptainPriority;
  CVector3f mPosition;
  TUniqueId mTargetId;

  friend class CScriptTeamAiMgr;
};
CHECK_SIZEOF(CTeamAiRole, 0x2c)

#endif
