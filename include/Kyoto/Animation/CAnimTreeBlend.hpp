#ifndef _CANIMTREEBLEND
#define _CANIMTREEBLEND

#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"

class CAnimTreeBlend : public CAnimTreeTweenBase {
public:
  CAnimTreeBlend(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                 const rstl::ncrc_ptr< CAnimTreeNode >& b, float weight, const rstl::string& name)
  : CAnimTreeTweenBase(characterSpaceBlend, a, b, kBlendRoot_Offset | kBlendRoot_Rotation, name)
  , mBlendWeight(weight) {}

  // IAnimReader
  ~CAnimTreeBlend() override {}
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;

  // CAnimTreeTweenBase
  void SetBlendingWeight(float weight) override;
  float VGetBlendingWeight() const override;

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b, float weight);

private:
  float mBlendWeight;
};
CHECK_SIZEOF(CAnimTreeBlend, 0x30)

#endif // _CANIMTREEBLEND
