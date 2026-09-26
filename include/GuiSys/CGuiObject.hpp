#ifndef _CGUIOBJECT
#define _CGUIOBJECT

#include "Kyoto/Math/CTransform4f.hpp"

class CGuiObject {
public:
  CGuiObject();
  virtual ~CGuiObject();

  const CTransform4f& GetWorldTransform() const;
  const CTransform4f& GetO2PTransform() const { return mLocalXF; }
  CVector3f GetLocalPosition() const;
  void SetLocalPosition(const CVector3f& pos);
  void SetO2PTransform(const CTransform4f& xf);
  void RotateReset();
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
