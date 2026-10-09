#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"

static void GetSegStatementSet(const rstl::rc_ptr< CAnimTreeNode >& child, const CSegIdList& list,
                               CSegStatementSet& setOut,
                               rstl::optional_object< CCharAnimTime > time) {
  if (time.valid()) {
    child->VGetSegStatementSet(list, setOut, *time);
  } else {
    child->VGetSegStatementSet(list, setOut);
  }
}

static void GetSegData(const rstl::rc_ptr< CAnimTreeNode >& child, const CCharLayoutInfo& layout,
                       CJointData_LinearStorage& data,
                       rstl::optional_object< CCharAnimTime > time) {
  if (time.valid()) {
    child->VGetJointData_Linear(layout, data, *time);
  } else {
    child->VGetJointData_Linear(layout, data);
  }
}

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
  float blend_weight = GetBlendingWeight();
  if (blend_weight >= 1.0) {
    return mB->VGetOffset(seg);
  } else {
    CVector3f start_offset = mA->VGetOffset(seg);
    CVector3f end_offset = mB->VGetOffset(seg);
    return start_offset.Lerp(start_offset, end_offset, blend_weight);
  }
}

CQuaternion CAnimTreeTweenBase::VGetRotation(const CSegId& seg) const {
  float blend_weight = GetBlendingWeight();
  if (blend_weight >= 1.0) {
    return mB->VGetRotation(seg);
  } else {
    CQuaternion start_offset = mA->VGetRotation(seg);
    CQuaternion end_offset = mB->VGetRotation(seg);
    return CAnimMathUtils::SlerpLocal(start_offset, end_offset, blend_weight);
  }
}

// Guessed name.
void CAnimTreeTweenBase::BlendSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                              rstl::optional_object< CCharAnimTime > time) const {
  float weight = GetBlendingWeight();
  static int sStack = 0;
  ++sStack;
  if (weight >= 1.0) {
    GetSegStatementSet(mB, list, setOut, time);
  } else if (sStack > 3) {
    const rstl::ncrc_ptr< CAnimTreeNode >& child = weight > 0.5f ? mB : mA;
    rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
    if (!best)
      best = child;
    GetSegStatementSet(best, list, setOut, time);
  } else {
    CStackSegStatementSet setA;
    GetSegStatementSet(mA, list, setA, time);
    CStackSegStatementSet setB;
    GetSegStatementSet(mB, list, setB, time);
    int count = list.GetCount();
    for (int i = 0; i < count; ++i) {
      const CSegId& id = list.mSegList[i];
      const CQuaternion& rotationA = setA[id].Orientation();
      setOut[id].Set(CAnimMathUtils::SlerpLocal(rotationA, setB[id].Orientation(), weight));
      if (setA[id].OffsetValid() && setB[id].OffsetValid())
        setOut[id].Set(CVector3f::Lerp(setA[id].Offset(), setB[id].Offset(), weight));
      if (setA[id].ScaleValid() || setB[id].ScaleValid())
        setOut[id].SetScale(
            CVector3f::Lerp(setA[id].ScaleValid() ? setA[id].Scale() : CVector3f::One(),
                            setB[id].ScaleValid() ? setB[id].Scale() : CVector3f::One(), weight));
    }
  }
  --sStack;
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
  float weight = GetBlendingWeight();
  static int sStack = 0;
  ++sStack;
  if (weight >= 1.0) {
    GetSegData(mB, layout, data, time);
  } else if (sStack > 3) {
    const rstl::ncrc_ptr< CAnimTreeNode >& child = weight > 0.5f ? mB : mA;
    rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
    if (!best)
      best = child;
    GetSegData(best, layout, data, time);
  } else {
    GetSegData(mA, layout, data, time);
    CJointData_LinearStorage dataB(layout.GetNumSegments(), CJointData_LinearStorage::kAF_Pool);
    if (data.HasScales())
      dataB.SetHasScales(true);
    GetSegData(mB, layout, dataB, time);
    data.Blend(dataB, weight);
  }
  --sStack;
}

void CAnimTreeTweenBase::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                              CJointData_LinearStorage& data,
                                              const CCharAnimTime& time) const {
  BlendSegData(layout, data, time);
}

void CAnimTreeTweenBase::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                              CJointData_LinearStorage& data) const {
  BlendSegData(layout, data, rstl::optional_object_null());
}

float CAnimTreeTweenBase::VGetRightChildWeight() const { return GetBlendingWeight(); }

float CAnimTreeTweenBase::GetBlendingWeight() const { return VGetBlendingWeight(); }

bool CAnimTreeTweenBase::ShouldCullTree() { return sAdvancementDepth >= 3; }

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTweenBase::VSimplified() {
  if (mCullSelector == 0) {
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > a = mA->Simplified();
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > b = mB->Simplified();
    const bool simplifyA = a.valid();
    const bool simplifyB = b.valid();
    if (!simplifyA && !simplifyB)
      return rstl::optional_object_null();
    CAnimTreeTweenBase* clone = static_cast< CAnimTreeTweenBase* >(Clone().take_ownership());
    if (simplifyA)
      clone->ReplaceLeftChild(static_cast< CAnimTreeNode* >(a->take_ownership()));
    if (simplifyB)
      clone->ReplaceRightChild(static_cast< CAnimTreeNode* >(b->take_ownership()));
    return rstl::ownership_transfer< IAnimReader >(clone);
  } else {
    const rstl::ncrc_ptr< CAnimTreeNode >& child = mCullSelector == 1 ? mB : mA;
    rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
    if (!best)
      return child->Clone();
    else
      return best->Clone();
  }
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
