#include "GuiSys/CGuiObject.hpp"

#include "Kyoto/Math/CMatrix3f.hpp"

CGuiObject::CGuiObject()
: mLocalXF(CTransform4f::Identity())
, mWorldXF(CTransform4f::Identity())
, mWorldTransformValid(false)
, mParent(nullptr)
, mChild(nullptr)
, mNextSibling(nullptr) {}

CGuiObject::~CGuiObject() {
  delete mChild;
  delete mNextSibling;
}

void CGuiObject::MoveInWorld(const CVector3f& offset) {
  if (mParent != nullptr) {
    // The original calls this conversion but discards its returned vector.
    mParent->RotateW2O(offset);
  }
  mLocalXF.AddTranslation(offset);
  RecalculateTransforms();
}

CVector3f CGuiObject::GetWorldPosition() const { return GetWorldTransform().GetTranslation(); }

CVector3f CGuiObject::GetLocalPosition() const { return mLocalXF.GetTranslation(); }

void CGuiObject::SetLocalPosition(const CVector3f& pos) {
  MoveInWorld(pos - mLocalXF.GetTranslation());
}

void CGuiObject::RotateReset() {
  const CVector3f position = mLocalXF.GetTranslation();
  mLocalXF = CTransform4f::Identity();
  mLocalXF.SetTranslation(position);
  RecalculateTransforms();
}

CVector3f CGuiObject::RotateW2O(const CVector3f& vec) const {
  const CVector3f result = GetWorldTransform().TransposeRotate(vec);
  return result;
}

CVector3f CGuiObject::RotateTranslateW2O(const CVector3f& vec) const {
  const CTransform4f& world = GetWorldTransform();
  const CVector3f result = world.TransposeRotate(vec - world.GetTranslation());
  return result;
}

void CGuiObject::MultiplyO2P(const CTransform4f& xf) {
  mLocalXF = xf * mLocalXF;
  RecalculateTransforms();
}

void CGuiObject::AddChildObject(CGuiObject* child, bool makeWorldLocal, bool atEnd) {
  child->mParent = this;
  CGuiObject* cur = mChild;
  if (cur == nullptr) {
    mChild = child;
  } else if (atEnd) {
    do {
      CGuiObject* next = cur->mNextSibling;
      if (next == nullptr) {
        cur->mNextSibling = child;
        break;
      }
      cur = next;
    } while (true);
  } else {
    child->mNextSibling = mChild;
    mChild = child;
  }

  if (makeWorldLocal) {
    const CTransform4f& parentWorld = child->mParent->GetWorldTransform();
    CTransform4f worldToLocal = CTransform4f::Identity();
    const CVector3f position = parentWorld.GetTranslation() * -1.f;
    const CVector3f scale(parentWorld.GetColumn(kDX).Magnitude(),
                          parentWorld.GetColumn(kDY).Magnitude(),
                          parentWorld.GetColumn(kDZ).Magnitude());
    const CVector3f& col2 = (1.f / scale.GetZ()) * parentWorld.GetColumn(kDZ);
    const CVector3f& col1 = (1.f / scale.GetY()) * parentWorld.GetColumn(kDY);
    const CVector3f& col0 = (1.f / scale.GetX()) * parentWorld.GetColumn(kDX);
    const CMatrix3f rotation(col0, col1, col2);
    const CVector3f translation = rotation * position;
    worldToLocal = CTransform4f(rotation.GetColumn(kDX).GetX(), rotation.GetColumn(kDY).GetX(),
                                rotation.GetColumn(kDZ).GetX(), translation.GetX(),
                                rotation.GetColumn(kDX).GetY(), rotation.GetColumn(kDY).GetY(),
                                rotation.GetColumn(kDZ).GetY(), translation.GetY(),
                                rotation.GetColumn(kDX).GetZ(), rotation.GetColumn(kDY).GetZ(),
                                rotation.GetColumn(kDZ).GetZ(), translation.GetZ());
    child->mLocalXF = worldToLocal * child->GetWorldTransform();
  }

  RecalculateTransforms();
}

const CGuiObject* CGuiObject::GetChildObject() const { return mChild; }

CGuiObject* CGuiObject::ChildObject() { return mChild; }

const CGuiObject* CGuiObject::GetNextSibling() const { return mNextSibling; }

CGuiObject* CGuiObject::NextSibling() { return mNextSibling; }

const CGuiObject* CGuiObject::GetParent() const { return mParent; }

CGuiObject* CGuiObject::Parent() { return mParent; }

void CGuiObject::SetO2PTransform(const CTransform4f& xf) {
  mLocalXF = xf;
  RecalculateTransforms();
}

void CGuiObject::SetO2WTransform(const CTransform4f& xf) {
  const CTransform4f inverse = mParent->GetWorldTransform().GetQuickInverse();
  const CTransform4f local = inverse * xf;
  SetO2PTransform(local);
}

inline const CTransform4f& CGuiObject::GetWorldTransform() const {
  if (!mWorldTransformValid) {
    if (mParent != nullptr) {
      mWorldXF = mParent->GetWorldTransform() * mLocalXF;
    } else {
      return mLocalXF;
    }
    mWorldTransformValid = true;
  }
  return mWorldXF;
}

inline void CGuiObject::RecalculateTransforms() {
  mWorldTransformValid = false;
  for (CGuiObject* child = mChild; child != nullptr; child = child->mNextSibling) {
    child->RecalculateTransforms();
  }
}
