#ifndef _CSCRIPTTRIGGERELLIPSOID
#define _CSCRIPTTRIGGERELLIPSOID

#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CScriptTriggerEllipsoid : public CScriptTrigger {
public:
  enum EShapeType {
    kST_Ellipsoid,
    kST_Cylinder,
  };

  CScriptTriggerEllipsoid(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                          const CVector3f& scale, const CTransform4f& xf, const CDamageInfo& damage,
                          const CVector3f& forceField, uint flags, bool deactivateOnEntered,
                          bool deactivateOnExited, EShapeType shape);

  // CEntity
  ~CScriptTriggerEllipsoid() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CScriptTrigger
  bool BoundsOverlap(const CAABox& bounds) const override;

  bool IsPointInside(const CVector3f& point) const; // Guessed name
  void SetScale(const CVector3f& scale);            // Guessed name
  const CVector3f& GetScale() const { return mScale; }
  EShapeType GetShape() const { return mShape; } // Guessed name.

private:
  static CAABox CalculateBounds(const CTransform4f& xf, const CVector3f& scale);

  CVector3f mScale; // Half extents (radii) of the shape.
  CVector3f mInverseScale;
  CAABox mWorldBounds;
  float x1f8_;
  EShapeType mShape;
};
CHECK_SIZEOF(CScriptTriggerEllipsoid, 0x200)

#endif // _CSCRIPTTRIGGERELLIPSOID
