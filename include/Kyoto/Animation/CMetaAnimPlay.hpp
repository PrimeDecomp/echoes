#ifndef _CMETAANIMPLAY
#define _CMETAANIMPLAY

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"

class CMetaAnimPlay : public IMetaAnim {
public:
  explicit CMetaAnimPlay(CInputStream& in) : mPrimitive(in), mStartTime(in) {}

  // IMetaAnim
  ~CMetaAnimPlay() override {}
  void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const override;
  EMetaAnimType GetType() const override { return kMAT_Play; }
  void WriteAnimData(COutputStream& out) const override;
  rstl::ncrc_ptr< CAnimTreeNode >
  VGetAnimationTree(const CAnimSysContext& animSys,
                    const CMetaAnimTreeBuildOrders& orders) const override;

private:
  CPrimitive mPrimitive;
  CCharAnimTime mStartTime;
};
CHECK_SIZEOF(CMetaAnimPlay, 0x24)

#endif // _CMETAANIMPLAY
