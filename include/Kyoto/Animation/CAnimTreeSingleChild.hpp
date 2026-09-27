#ifndef _CANIMTREESINGLECHILD
#define _CANIMTREESINGLECHILD

#include "Kyoto/Animation/CAnimTreeNode.hpp"

class CAnimTreeSingleChild : public CAnimTreeNode {
public:
  CAnimTreeSingleChild(const rstl::ncrc_ptr< CAnimTreeNode >& child, const rstl::string& name)
  : CAnimTreeNode(name), mChild(child) {}

  // IAnimReader
  ~CAnimTreeSingleChild() override {}
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
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
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                   const CCharAnimTime& time) const override;
  void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data) const override;
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  // CAnimTreeNode
  uint Depth() const override { return mChild->Depth() + 1; }
  uint VGetNumChildren() const override;
  void VGetWeightedReaders(
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;

  void ReplaceChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mChild = child; }

protected:
  rstl::ncrc_ptr< CAnimTreeNode > mChild;
};
CHECK_SIZEOF(CAnimTreeSingleChild, 0x1c)

#endif // _CANIMTREESINGLECHILD
