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
  return GetWorldTransform().TransposeRotate(vec);
}

CVector3f CGuiObject::RotateTranslateW2O(const CVector3f& vec) const {
  const CTransform4f& world = GetWorldTransform();
  return world.TransposeRotate(vec - world.GetTranslation());
}

void CGuiObject::MultiplyO2P(const CTransform4f& xf) {
  mLocalXF = xf * mLocalXF;
  RecalculateTransforms();
}

void CGuiObject::AddChildObject(CGuiObject* child, bool makeWorldLocal, bool atEnd) {
  child->mParent = this;
  if (mChild == nullptr) {
    mChild = child;
  } else if (atEnd) {
    CGuiObject* last = mChild;
    while (last->mNextSibling != nullptr) {
      last = last->mNextSibling;
    }
    last->mNextSibling = child;
  } else {
    child->mNextSibling = mChild;
    mChild = child;
  }

  if (makeWorldLocal) {
    const CTransform4f& parentWorld = child->mParent->GetWorldTransform();
    const CVector3f position = parentWorld.GetTranslation() * -1.f;
    const CVector3f scale(parentWorld.GetColumn(kDX).Magnitude(),
                          parentWorld.GetColumn(kDY).Magnitude(),
                          parentWorld.GetColumn(kDZ).Magnitude());
    const CMatrix3f rotation((1.f / scale.GetX()) * parentWorld.GetColumn(kDX),
                             (1.f / scale.GetY()) * parentWorld.GetColumn(kDY),
                             (1.f / scale.GetZ()) * parentWorld.GetColumn(kDZ));
    const CTransform4f worldToLocal(rotation, rotation * position);
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

const CTransform4f& CGuiObject::GetWorldTransform() const {
  if (!mWorldTransformValid) {
    if (mParent == nullptr) {
      return mLocalXF;
    }
    mWorldXF = mParent->GetWorldTransform() * mLocalXF;
    mWorldTransformValid = true;
  }
  return mWorldXF;
}

void CGuiObject::RecalculateTransforms() {
  mWorldTransformValid = false;
  for (CGuiObject* child = mChild; child != nullptr; child = child->mNextSibling) {
    child->RecalculateTransforms();
  }
}
