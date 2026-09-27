#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"

CAnimTreeTweenBase::CAnimTreeTweenBase(bool characterSpaceBlend,
                                       const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                       const rstl::ncrc_ptr< CAnimTreeNode >& b, int flags,
                                       const rstl::string& name)
: CAnimTreeDoubleChild(a, b, name)
, mFlags(flags)
, mCharacterSpaceBlend(characterSpaceBlend)
, mCullSelector(0) {}

CAnimTreeTweenBase::~CAnimTreeTweenBase() {}

bool CAnimTreeTweenBase::VHasOffset(const CSegId& seg) const {
  return mA->VHasOffset(seg) && mB->VHasOffset(seg);
}

CVector3f CAnimTreeTweenBase::VGetOffset(const CSegId& seg) const {
  const float weight = GetBlendingWeight();
  if (weight >= 1.f) {
    return mB->VGetOffset(seg);
  }
  return CVector3f::Lerp(mA->VGetOffset(seg), mB->VGetOffset(seg), weight);
}

CQuaternion CAnimTreeTweenBase::VGetRotation(const CSegId& seg) const {
  // TODO: Blend both child rotations with the animation-specific interpolation helper.
  return mB->VGetRotation(seg);
}

// Guessed name.
void CAnimTreeTweenBase::BlendSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                              rstl::optional_object< CCharAnimTime > time) const {
  // TODO: Blend rotation, translation and scale, including the recursion-depth fallback.
}

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list,
                                             CSegStatementSet& setOut) const {
  BlendSegStatementSet(list, setOut, rstl::optional_object_null());
}

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                             const CCharAnimTime& time) const {
  BlendSegStatementSet(list, setOut, time);
}

// Guessed name.
void CAnimTreeTweenBase::BlendSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                                      rstl::optional_object< CCharAnimTime > time) const {
  // TODO: Blend packed joint data, preserving scale/offset flags and depth fallback.
}

void CAnimTreeTweenBase::VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                                     const CCharAnimTime& time) const {
  BlendSegData(layout, data, time);
}

void CAnimTreeTweenBase::VGetSegData(const CCharLayoutInfo& layout,
                                     CJointData_LinearStorage& data) const {
  BlendSegData(layout, data, rstl::optional_object_null());
}

float CAnimTreeTweenBase::VGetRightChildWeight() const { return GetBlendingWeight(); }

float CAnimTreeTweenBase::GetBlendingWeight() const { return VGetBlendingWeight(); }

bool CAnimTreeTweenBase::ShouldCullTree() { return sAdvancementDepth >= 3; }

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTweenBase::VSimplified() {
  // TODO: Simplify children or clone the selected branch according to mCullSelector.
  return rstl::optional_object_null();
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeTweenBase::VReverseSimplified() {
  return CAnimTreeTweenBase::VSimplified();
}

void CAnimTreeTweenBase::VGetWeightedReaders(
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  const float blendWeight = GetBlendingWeight();
  mA->VGetWeightedReaders(weight * (1.f - blendWeight), out);
  mB->VGetWeightedReaders(weight * blendWeight, out);
}

s32 CAnimTreeTweenBase::sAdvancementDepth = 0;
