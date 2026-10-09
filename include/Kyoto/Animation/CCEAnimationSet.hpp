#ifndef _CCEANIMATIONSET
#define _CCEANIMATIONSET

#include "Kyoto/Animation/CAdditiveAnimationInfo.hpp"
#include "Kyoto/Animation/CAnimPOIData.hpp"
#include "Kyoto/Animation/CAnimation.hpp"
#include "Kyoto/Animation/CHalfTransition.hpp"
#include "Kyoto/Animation/CTransition.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"

#include "rstl/construct.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

class CCEAnimationSet {
public:
  typedef rstl::vector< CAnimation > AnimationList;
  typedef rstl::vector< CTransition > TransitionList;
  typedef rstl::vector< CHalfTransition > HalfTransitionList;
  typedef rstl::vector< rstl::pair< uint, CAdditiveAnimationInfo > > AdditiveAnimationList;
  typedef rstl::vector< CAnimPOIData > EventSetList; // Guessed name.

  explicit CCEAnimationSet(CInputStream& in);

  const AnimationList& GetAnimations() const { return mAnimations; }
  const TransitionList& GetTransitions() const { return mTransitions; }
  const HalfTransitionList& GetHalfTransitions() const { return mHalfTransitions; }
  const rstl::rc_ptr< IMetaTrans >& GetDefaultTransition() const { return mDefaultTransition; }
  const AdditiveAnimationList& GetAdditiveAnimInfoList() const { return mAdditiveAnimations; }
  const CAdditiveAnimationInfo& GetDefaultAdditiveAnimInfo() const {
    return mDefaultAdditiveAnimation;
  }
  const EventSetList& GetEventSets() const { return mEventSets; } // Guessed name.

  static AdditiveAnimationList StreamAdditiveAnimInfoList(ushort tableCount, CInputStream& in);
  static CAdditiveAnimationInfo StreamDefaultAdditiveAnimInfo(ushort tableCount, CInputStream& in);
  static HalfTransitionList StreamHalfTransitions(ushort tableCount, CInputStream& in);
  static EventSetList StreamEventSetList(ushort tableCount, CInputStream& in); // Guessed name.

private:
  ushort mTableCount;
  AnimationList mAnimations;
  TransitionList mTransitions;
  rstl::rc_ptr< IMetaTrans > mDefaultTransition;
  AdditiveAnimationList mAdditiveAnimations;
  CAdditiveAnimationInfo mDefaultAdditiveAnimation;
  HalfTransitionList mHalfTransitions;
  EventSetList mEventSets; // Guessed name: embedded animation event data.
};
CHECK_SIZEOF(CCEAnimationSet, 0x64)

#endif // _CCEANIMATIONSET
