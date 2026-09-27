#ifndef _CANIMTREEDOUBLECHILD
#define _CANIMTREEDOUBLECHILD

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "rstl/math.hpp"

class CAnimTreeDoubleChild : public CAnimTreeNode {
public:
  class CDoubleChildAdvancementResult {
  public:
    CDoubleChildAdvancementResult(const CCharAnimTime& time, const SAdvancementDeltas& left,
                                  const SAdvancementDeltas& right);
    const CCharAnimTime& GetTrueAdvancement() const { return mTrueAdvancement; }
    const SAdvancementDeltas& GetLeftAdvancementDeltas() const { return mLeftDeltas; }
    const SAdvancementDeltas& GetRightAdvancementDeltas() const { return mRightDeltas; }

  private:
    CCharAnimTime mTrueAdvancement;
    SAdvancementDeltas mLeftDeltas;
    SAdvancementDeltas mRightDeltas;
  };

  CAnimTreeDoubleChild(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                       const rstl::ncrc_ptr< CAnimTreeNode >& b, const rstl::string& name);

  // IAnimReader
  ~CAnimTreeDoubleChild() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int additive) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int additive) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
  bool VGetBoolPOIState(uint nameHash) const override;
  s32 VGetInt32POIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  // CAnimTreeNode
  uint Depth() const override { return rstl::max_val(mA->Depth(), mB->Depth()) + 1; }
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  uint VGetNumChildren() const override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;

  virtual float VGetRightChildWeight() const = 0;

  void ReplaceLeftChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mA = child; }
  void ReplaceRightChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mB = child; }
  float GetLeftChildWeight() const { return 1.f - VGetRightChildWeight(); }
  float GetRightChildWeight() const { return VGetRightChildWeight(); }
  const rstl::rc_ptr< CAnimTreeNode >& GetLeftChild() const { return mA; }
  const rstl::rc_ptr< CAnimTreeNode >& GetRightChild() const { return mB; }

protected:
  CDoubleChildAdvancementResult AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft,
                                                        bool loopLeft);
  rstl::ncrc_ptr< CAnimTreeNode > mA;
  rstl::ncrc_ptr< CAnimTreeNode > mB;
};
CHECK_SIZEOF(CAnimTreeDoubleChild, 0x24)

#endif // _CANIMTREEDOUBLECHILD
