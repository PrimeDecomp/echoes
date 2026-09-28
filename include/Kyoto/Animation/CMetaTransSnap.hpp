#ifndef _CMETATRANSSNAP
#define _CMETATRANSSNAP

#include "Kyoto/Animation/IMetaTrans.hpp"

class CMetaTransSnap : public IMetaTrans {
public:
  // IMetaTrans
  EMetaTransType GetType() const override { return kMTT_Snap; }

  rstl::ncrc_ptr< CAnimTreeNode > VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                     const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                     const CAnimSysContext& animSys) const override;

  void WriteTransData(COutputStream&) const override;
};

CHECK_SIZEOF(CMetaTransSnap, 0x4)

#endif // _CMETATRANSSNAP
