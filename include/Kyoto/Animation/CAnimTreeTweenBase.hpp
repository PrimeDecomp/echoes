#ifndef _CANIMTREETWEENBASE
#define _CANIMTREETWEENBASE

#include "Kyoto/Animation/CAnimTreeDoubleChild.hpp"

class CAnimTreeTweenBase : public CAnimTreeDoubleChild {
public:
  static const int kBlendRoot_Offset;
  static const int kBlendRoot_Rotation;

  CAnimTreeTweenBase(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                     const rstl::ncrc_ptr< CAnimTreeNode >& b, int flags, const rstl::string& name);

  // IAnimReader
  ~CAnimTreeTweenBase() override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                   const CCharAnimTime& time) const override;
  void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data) const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;

  // CAnimTreeNode
  void VGetWeightedReaders(
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;

  // CAnimTreeDoubleChild
  float VGetRightChildWeight() const override;

  virtual void SetBlendingWeight(float weight) = 0;
  virtual rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VReverseSimplified();
  virtual float VGetBlendingWeight() const = 0;

  float GetBlendingWeight() const;
  bool CharacterSpaceBlend() const { return mCharacterSpaceBlend != 0; }
  int GetBlendRoot() const { return mFlags; }
  static bool ShouldCullTree();
  static void IncAdvancementDepth() { ++sAdvancementDepth; }
  static void DecAdvancementDepth() { --sAdvancementDepth; }

protected:
  int mFlags;
  s32 mCharacterSpaceBlend : 1;
  s32 mCullSelector : 2;

private:
  // Guessed names: common implementations of the timed and current-pose virtuals.
  void BlendSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                    rstl::optional_object< CCharAnimTime > time) const;
  void BlendSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                            rstl::optional_object< CCharAnimTime > time) const;
  static s32 sAdvancementDepth;
};
CHECK_SIZEOF(CAnimTreeTweenBase, 0x2c)

#endif // _CANIMTREETWEENBASE
