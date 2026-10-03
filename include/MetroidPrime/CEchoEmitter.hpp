#ifndef _CECHOEMITTER
#define _CECHOEMITTER

#include "MetroidPrime/SEchoParameters.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

class CStateManager;
class CColor;

class CEchoEmitter {
public:
  CEchoEmitter(const CAABox& bounds, const SEchoParameters& parameters);
  virtual ~CEchoEmitter();
  // Guessed names; retain the original virtual order.
  virtual void DestroyEmitter(CStateManager& mgr);
  virtual void Think(float dt, CStateManager& mgr);
  virtual void Render(const CStateManager& mgr) const;

  void CreateEmitter(CStateManager& mgr);
  void TriggerDamageEcho(); // Guessed name; starts the configured damage-echo decay.
  void SetAutoReducingYellowDamage(float duration);
  void SetAutoReducingDamage(float duration);
  void SetDamageExplicit(float damage);
  // Guessed type and field names; native result stores a rotation and its projected center.
  struct SProjection {
    SProjection(const CTransform4f& rotation, const CVector3f& projectedCenter);
    CTransform4f mRotation;
    CVector3f mProjectedCenter;
  };
  // Guessed names for native projection and linked-list rendering helpers.
  static void RenderEmitters(const CStateManager& mgr, const CEchoEmitter* first);
  static SProjection ProjectPoints(const CVector3f* points, int count, CVector3f* output,
                                   const int* indices);
  static void EnsureMinimumWidth(CVector3f& left, CVector3f& right, float width);
  static void LimitHeight(CVector3f& point, float origin, float height);
  static CVector3f InterpolateContourPoint(const CVector3f& first, const CVector3f& second,
                                           const CVector3f& target, float amount,
                                           float minimumDistance, float maximumDistance);
  void SetBounds(const CAABox& bounds) { mBounds = bounds; }
  void SetParameters(const SEchoParameters& parameters) { mParameters = parameters; }
  bool IsPendingDeletion() const { return mPendingDeletion; }

private:
  // Guessed names for native runtime helpers.
  void ResetPlayerState();
  void ResetPlayerState(CStateManager& mgr);

protected:
  void DrawContour(const CVector3f* points, int count, int subdivisions,
                   const SProjection& projection, const CStateManager& mgr) const;

private:
  void DrawWaves(const CVector3f* points, int count, int subdivisions,
                 const SProjection& projection, const CColor& color, float scale, float spacing,
                 float alpha) const;
  // Guessed runtime field names, supported by construction and the native consumers.
  SEchoParameters mParameters;
  CAABox mBounds;
  uint mPlayerEchoTokens[4];
  float mPlayerEchoVisibility[4];
  float mDamage;
  float mYellowDamage;
  float mDamageReductionRate;
  float mYellowDamageReductionRate;
  bool mActive : 1;
  bool mPendingDeletion : 1;
  CEchoEmitter* mNextEmitter;
};
CHECK_SIZEOF(CEchoEmitter, 0x64)
NESTED_CHECK_SIZEOF(CEchoEmitter, SProjection, 0x3c)

#endif // _CECHOEMITTER
