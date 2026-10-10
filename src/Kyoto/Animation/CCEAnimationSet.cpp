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

extern "C" void fn_8028DD1C(const char* ptr, int obj) {
    for (int i = *(int*)ptr; i != (unsigned int)*(int*)obj; i += 12) {
        if ((unsigned int)i != 0 && i + 4) {
            ((rstl::rc_ptr<IMetaTrans>*)(i + 4))->ReleaseData();
        }
    }
}

extern "C" void fn_8028DE38(const char* ptr, int obj) {
    for (int i = *(int*)ptr; i != (unsigned int)*(int*)obj; i += 20) {
        if ((unsigned int)i != 0 && i + 12) {
            ((rstl::rc_ptr<IMetaTrans>*)(i + 12))->ReleaseData();
        }
    }
}

extern "C" void fn_8028DF40(int obj, const char* ptr) {
    int val;
    int val2 = *(int*)((char*)obj + 0x4);
    int val3 = *(int*)((char*)obj + 0xc);
    *(int*)((char*)obj + 0x4) = val2 + 1;
    int val4 = val3 + val2 * 20;
    if (!val4) {
        return;
    }
    val = *(int*)(ptr + 0x4);
    *(int*)val4 = *(int*)ptr;
    *(int*)((char*)val4 + 0x4) = val;
    *(int*)((char*)val4 + 0x8) = (*(int*)(ptr + 0x8));
    *(int*)((char*)val4 + 0xc) = (*(int*)(ptr + 0xc));
    *(int*)((char*)val4 + 0x10) = (*(int*)(ptr + 0x10));
    int val8 = *(int*)((char*)val4 + 0x10);
    *(int*)val8 = *(int*)val8 + 1;
}
