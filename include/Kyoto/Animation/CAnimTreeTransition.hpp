#ifndef _CANIMTREETRANSITION
#define _CANIMTREETRANSITION

#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"
#include "Kyoto/Animation/CPOINode.hpp"

class CAnimTreeTransition : public CAnimTreeTweenBase {
public:
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
                      const bool runA, int flags, const rstl::string& name);
  CAnimTreeTransition(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& duration,
                      const CCharAnimTime& timeInTrans, bool runA, bool loopA, int flags,
                      const rstl::string& name, bool initialized);

  // IAnimReader
  ~CAnimTreeTransition() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;

  // CAnimTreeNode
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;

  // CAnimTreeTweenBase
  void SetBlendingWeight(float weight) override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VReverseSimplified() override;
  float VGetBlendingWeight() const override;

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b, float duration);

private:
  rstl::pair< CCharAnimTime, SAdvancementDeltas >
  AdvanceViewForTransitionalPeriod(const CCharAnimTime& time);

  // Guessed name.
  static uint GetLoopPOIHash();

  CCharAnimTime mTransDur;
  CCharAnimTime mTimeInTrans;
  const bool mRunA;
  bool mLoopA;
  bool mInitialized;
};
CHECK_SIZEOF(CAnimTreeTransition, 0x40)

#endif // _CANIMTREETRANSITION
