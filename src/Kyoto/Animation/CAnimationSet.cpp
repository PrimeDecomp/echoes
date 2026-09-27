#include "Kyoto/Animation/CAnimationSet.hpp"

#include "Kyoto/Animation/CMetaTransFactory.hpp"

CAnimationSet::CAnimationSet(CInputStream& in)
: mTableCount(in.Get< ushort >())
, mAnimations(in)
, mTransitions(in)
, mDefaultTransition(CMetaTransFactory::CreateMetaTrans(in))
, mAdditiveAnimations(StreamAdditiveAnimInfoList(mTableCount, in))
, mDefaultAdditiveAnimation(StreamDefaultAdditiveAnimInfo(mTableCount, in))
, mHalfTransitions(StreamHalfTransitions(mTableCount, in))
, mEventSets(StreamEventSetList(mTableCount, in)) {}

CAnimationSet::AdditiveAnimationList CAnimationSet::StreamAdditiveAnimInfoList(ushort tableCount,
                                                                               CInputStream& in) {
  if (tableCount > 1) {
    return AdditiveAnimationList(in);
  }

  return AdditiveAnimationList();
}

CAdditiveAnimationInfo CAnimationSet::StreamDefaultAdditiveAnimInfo(ushort tableCount,
                                                                    CInputStream& in) {
  if (tableCount > 1) {
    return CAdditiveAnimationInfo(in);
  }

  return CAdditiveAnimationInfo(0.f, 0.f);
}

CAnimationSet::HalfTransitionList CAnimationSet::StreamHalfTransitions(ushort tableCount,
                                                                       CInputStream& in) {
  if (tableCount > 2) {
    return HalfTransitionList(in);
  }

  return HalfTransitionList();
}

// Guessed name.
CAnimationSet::EventSetList CAnimationSet::StreamEventSetList(ushort tableCount, CInputStream& in) {
  if (tableCount > 3) {
    return EventSetList(in);
  }

  return EventSetList();
}
