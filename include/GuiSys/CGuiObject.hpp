#ifndef _CGUIOBJECT
#define _CGUIOBJECT

#include "Kyoto/Math/CTransform4f.hpp"

class CGuiObject {
public:
  CGuiObject();
  virtual ~CGuiObject();

  void MoveInWorld(const CVector3f& offset);

  const CTransform4f& GetWorldTransform() const;
  CVector3f GetWorldPosition() const;
  const CTransform4f& GetO2PTransform() const { return mLocalXF; }
  CVector3f GetLocalPosition() const;
  void SetLocalPosition(const CVector3f& pos);
  void SetO2PTransform(const CTransform4f& xf);
  void SetO2WTransform(const CTransform4f& xf);
  void RotateReset();
  CVector3f RotateW2O(const CVector3f& vec) const;
  CVector3f RotateTranslateW2O(const CVector3f& vec) const;
  void MultiplyO2P(const CTransform4f& xf);
  void RecalculateTransforms();

  CGuiObject* Parent();
  const CGuiObject* GetParent() const;
  CGuiObject* ChildObject();
  const CGuiObject* GetChildObject() const;
  CGuiObject* NextSibling();
  const CGuiObject* GetNextSibling() const;
  void AddChildObject(CGuiObject* child, bool makeWorldLocal, bool atEnd);

private:
  CTransform4f mLocalXF;
  mutable CTransform4f mWorldXF;
  mutable bool mWorldTransformValid; // Guessed name
  CGuiObject* mParent;
  CGuiObject* mChild;
  CGuiObject* mNextSibling;
};
CHECK_SIZEOF(CGuiObject, 0x74)

#endif // _CGUIOBJECT
