#include "Kyoto/Animation/CCEAnimationSet.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"

CCEAnimationSet::CCEAnimationSet(CInputStream& in)
: mTableCount(in.Get< ushort >())
, mAnimations(in)
, mTransitions(in)
, mDefaultTransition(CMetaTransFactory::CreateMetaTrans(in))
, mAdditiveAnimations(StreamAdditiveAnimInfoList(mTableCount, in))
, mDefaultAdditiveAnimation(StreamDefaultAdditiveAnimInfo(mTableCount, in))
, mHalfTransitions(StreamHalfTransitions(mTableCount, in))
, mEventSets(StreamEventSetList(mTableCount, in)) {}

CCEAnimationSet::AdditiveAnimationList
CCEAnimationSet::StreamAdditiveAnimInfoList(ushort tableCount, CInputStream& in) {
  if (tableCount > 1) {
    return AdditiveAnimationList(in);
  }

  return AdditiveAnimationList();
}

CAdditiveAnimationInfo CCEAnimationSet::StreamDefaultAdditiveAnimInfo(ushort tableCount,
                                                                      CInputStream& in) {
  if (tableCount > 1) {
    return CAdditiveAnimationInfo(in);
  }

  return CAdditiveAnimationInfo(0.f, 0.f);
}

CCEAnimationSet::HalfTransitionList CCEAnimationSet::StreamHalfTransitions(ushort tableCount,
                                                                           CInputStream& in) {
  if (tableCount > 2) {
    return HalfTransitionList(in);
  }

  return HalfTransitionList();
}

// Guessed name.
CCEAnimationSet::EventSetList CCEAnimationSet::StreamEventSetList(ushort tableCount,
                                                                  CInputStream& in) {
  if (tableCount > 3) {
    return EventSetList(in);
  }

  return EventSetList();
}
