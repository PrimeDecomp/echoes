#ifndef _CSCRIPTFRONTENDDATANETWORK
#define _CSCRIPTFRONTENDDATANETWORK

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CActor.hpp"
#include "rstl/vector.hpp"

class CFinalInput;
class CScriptFrontEndDataNetwork;
class CTexture;

// Guessed name. One record per node of the network, owned by the root node.
struct SDataNetworkNode {
  SDataNetworkNode(TUniqueId id, int index, int parent, bool isProxy, bool parentIsProxy);

  TUniqueId GetId() const { return mId; }
  CScriptFrontEndDataNetwork* GetNetwork(CStateManager& mgr);
  const CScriptFrontEndDataNetwork* GetConstNetwork(const CStateManager& mgr) const;

  void SetX64(float v);
  void SetX60(float v);
  void SetX5C(float v);
  void SetSelectedChild(int v);
  int GetSelectedChild() const;
  void SetParent(int v);
  void SetVelocity(const CVector3f& v);
  const CVector3f& GetVelocity() const;
  void SetX38(const CVector3f& v);
  const CVector3f& GetX38() const;
  void SetRenderPos(const CVector3f& v);
  const CVector3f& GetRenderPos() const;
  void SetPos(const CVector3f& v);
  const CVector3f& GetPos() const;
  void SetOffset(const CVector3f& v);
  const CVector3f& GetOffset() const;
  void AddChild(int idx);

  TUniqueId mId;
  int mIndex;
  int mParent;
  rstl::vector< int > mChildren;
  int mSelectedChild;
  CVector3f mOffset;
  CVector3f mPos;
  CVector3f x38;
  CVector3f mVelocity;
  CVector3f mRenderPos;
  float x5c;
  float x60;
  float x64;
  bool mIsProxy : 1;
  bool mParentIsProxy : 1;
};
CHECK_SIZEOF(SDataNetworkNode, 0x6c)

class CScriptFrontEndDataNetwork : public CActor {
public:
  // Guessed names.
  enum ETransitionState {
    kTS_Idle,
    kTS_Shrink,
    kTS_Move,
    kTS_Expand,
  };

  CScriptFrontEndDataNetwork(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CMayaSpline& shrinkSpline,
                             const CMayaSpline& moveSpline, const CMayaSpline& expandSpline,
                             const CMayaSpline& moveInSpline, const bool isRoot, const bool b2,
                             const bool b3, const bool isProxy, const bool canBeSelected,
                             const bool isLocked, const bool b7, const bool b8,
                             CAssetId hotDotTexture, CAssetId hotDotHaloTexture,
                             CAssetId hotDotAButtonTexture, const CColor& selectedColor,
                             const CColor& unselectedMinColor, const CColor& unselectedMaxColor,
                             const CColor& disabledColor, TSfxId rotationSound,
                             int rotationSoundVolume, float shrinkTime, float moveTime,
                             float expandTime, float moveInTime, float connectionRadius);

  // CEntity
  ~CScriptFrontEndDataNetwork() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;

  // CActor
  void AddToRenderer(const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  bool CanRenderUnsorted(const CStateManager&) const override;

  TUniqueId GetPlatformId() const;
  float GetConnectionRadius() const { return mConnectionRadius; }
  void SetRootId(TUniqueId id);

private:
  void BuildNetwork(CStateManager& mgr);
  int AddNode(CStateManager& mgr, TUniqueId id, int parent);
  void LayoutChildren(int idx, CStateManager& mgr);
  void ResetTransition(CStateManager& mgr);
  void UpdateTransition(CStateManager& mgr, float dt);
  void SimulateChildren(CStateManager& mgr, int idx, float dt);
  void UpdateRenderPositions(CStateManager& mgr, int idx, float dt);
  void SetLocked(bool locked, CStateManager& mgr);
  void OpenNode(TUniqueId id, CStateManager& mgr);
  void CloseNode(CStateManager& mgr);
  void FaceNode(TUniqueId id, const CStateManager& mgr, bool onlyWhenInactive);
  void AddController(int controller);
  void ClearControllers(CStateManager& mgr);
  void RenderNode(const CStateManager& mgr, const CTransform4f& xf, int idx, float alpha) const;
  void DrawConnection(const CTransform4f& xf, const CVector3f& a, const CVector3f& b,
                      const CColor& colorA, const CColor& colorB, float width) const;
  void DrawBillboard(const CTransform4f& xf, const CVector3f& pos, float size, const CColor& color,
                     bool additive) const;
  uchar HandleRotation(const CFinalInput& input, CStateManager& mgr);
  uchar HandleButtons(const CFinalInput& input, CStateManager& mgr);
  uchar HandleStick(const CFinalInput& input, CStateManager& mgr);
  void SetSelection(CStateManager& mgr, int index, bool immediate);
  CVector3f GetFalloff(float radius, float strength, const CVector3f& a, const CVector3f& b) const;
  CVector3f GetSeparation(const CStateManager& mgr, const SDataNetworkNode& node, int idx) const;
  CVector3f GetCohesion(const CStateManager& mgr, const SDataNetworkNode& node, int idx) const;
  CVector3f GetAttraction(const SDataNetworkNode& node, const CVector3f& pos) const;

  TUniqueId mRootId;
  TUniqueId mPlatformId;
  rstl::vector< SDataNetworkNode > mNodes;
  int mPrevIndex;
  int mCurIndex;
  CVector2f mSpin;
  CVector2f mSpinAccel;
  CQuaternion mOrientation;
  ETransitionState mTransitionState;
  int mTransitionForward;
  float mTransitionT;
  float mTransitionDuration;
  float x1a4;
  float x1a8;
  CMayaSpline mShrinkSpline;
  float mShrinkTime;
  CMayaSpline mMoveSpline;
  float mMoveTime;
  CMayaSpline mExpandSpline;
  float mExpandTime;
  CMayaSpline mMoveInSpline;
  float mMoveInTime;
  bool mIsRoot;
  bool x2cd;
  bool x2ce;
  bool mIsProxy;
  bool mCanBeSelected;
  bool mIsLocked;
  bool x2d2;
  bool x2d3;
  float mConnectionRadius;
  CAssetId mHotDotTexture;
  CAssetId mHotDotHaloTexture;
  CAssetId mHotDotAButtonTexture;
  CColor mSelectedColor;
  CColor mUnselectedMinColor;
  CColor mUnselectedMaxColor;
  CColor mDisabledColor;
  int mController;
  rstl::vector< int > mControllers;
  int mActiveController;
  CSfxHandle mRotationSfx;
  TSfxId mRotationSound;
  int mRotationSoundVolume;
};
CHECK_SIZEOF(CScriptFrontEndDataNetwork, 0x318)

#endif // _CSCRIPTFRONTENDDATANETWORK
