#ifndef _CPIRATEECHOEMITTER
#define _CPIRATEECHOEMITTER

#include "Kyoto/Animation/CSegId.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"

class CActor;
class CPoseAsTransforms_Linear;

class CPirateEchoEmitter : public CEchoEmitter {
public:
  CPirateEchoEmitter(const CActor* actor, const CVector3f& position,
                     const SEchoParameters& parameters, CSegId head, CSegId rightWing,
                     CSegId leftWing, CSegId gun, CSegId swoosh, CSegId rightAnkle,
                     CSegId leftAnkle);

  // CEchoEmitter
  ~CPirateEchoEmitter() override;
  void Render(const CStateManager& mgr) const override;

private:
  // Guessed method and field names, from the native pose and locator consumers.
  CVector3f GetHeadPosition(const CPoseAsTransforms_Linear& pose) const;

  const CActor* mActor;
  CSegId mHead;
  CSegId mRightWing;
  CSegId mLeftWing;
  CSegId mGun;
  CSegId mSwoosh;
  CSegId mRightAnkle;
  CSegId mLeftAnkle;
  bool mUseHeadOnly;
};
CHECK_SIZEOF(CPirateEchoEmitter, 0x70)

#endif // _CPIRATEECHOEMITTER
