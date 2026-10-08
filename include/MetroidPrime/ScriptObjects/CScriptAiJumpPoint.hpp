#ifndef _CSCRIPTAIJUMPPOINT
#define _CSCRIPTAIJUMPPOINT

#include "MetroidPrime/CActor.hpp"

// Original Wii export name, independently correlated with the GameCube AJMP loader.
class CScriptAiJumpPoint : public CActor {
public:
  CScriptAiJumpPoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, float jumpApex, int type);

  // CEntity
  ~CScriptAiJumpPoint() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  bool GetInUse(TUniqueId uid) const;
  TUniqueId GetJumpPoint() const { return mCurrentWaypoint; }
  TUniqueId GetJumpTarget() const { return mNextWaypoint; }
  float GetJumpApex() const { return mJumpApex; }
  int GetType() const { return mType; } // Guessed name

private:
  float mJumpApex;
  int mType; // Verified loader property; individual type meanings remain unresolved.
  rstl::optional_object< CAABox > mTouchBounds;
  bool mInUse : 1;
  TUniqueId mOccupant;
  TUniqueId mCurrentWaypoint;
  TUniqueId mNextWaypoint;
  float mTimeRemaining;
};
CHECK_SIZEOF(CScriptAiJumpPoint, 0x188)

#endif // _CSCRIPTAIJUMPPOINT
