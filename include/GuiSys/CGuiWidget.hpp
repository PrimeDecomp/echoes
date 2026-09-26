#ifndef _CGUIWIDGET
#define _CGUIWIDGET

#include "GuiSys/CGuiObject.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"

class CFinalInput;
class CGuiFrame;
class CGuiWidgetDrawParms;

enum ETraversalMode { kTM_ChildrenAndSiblings = 0, kTM_Children = 1, kTM_Single = 2 };

class CGuiWidget : public CGuiObject {
public:
  enum EGuiModelDrawFlags {
    kGMDF_Shadeless = 0,
    kGMDF_Opaque = 1,
    kGMDF_Alpha = 2,
    kGMDF_Additive = 3,
    kGMDF_AlphaAdditiveOverdraw = 4
  };

  class CGuiWidgetParms {
  public:
    CGuiFrame* mFrame;
    short mSelfId;
    short mParentId;
    CColor mColor;
    EGuiModelDrawFlags mDrawFlags;
    bool mCullFaces;
    bool mDefaultVisible;
    bool mDefaultActive;
    bool mDepthTest;
    bool mDepthWrite;
    bool mDepthGreater;
  };

  explicit CGuiWidget(const CGuiWidgetParms& parms);

  // CGuiObject
  ~CGuiWidget() override;

  virtual FourCC GetWidgetTypeID() const;
  virtual int GetWidgetTypeIndex() const; // Guessed name
  virtual bool AddWorkerWidget(CGuiWidget* worker);
  virtual bool GetIsActive() const;
  virtual bool GetIsVisible() const;
  virtual void Update(float dt);
  virtual void Draw(const CGuiWidgetDrawParms& parms) const;
  virtual void ProcessUserInput(const CFinalInput& input);
  virtual CGuiWidget* GetWorkerWidget(int workerId);
  virtual void OnVisible();
  virtual void OnActivate();
  virtual void Initialize();

  void SetIsVisible(bool visible);
  void SetIsActive(bool active);
  void SetColor(const CColor& color);
  void SetVisibility(bool visible, ETraversalMode mode);
  void RecalcWidgetColor(ETraversalMode mode);
  void ReapplyXform();
  void SetIdleXform(const CTransform4f& xf, bool reapply = true);
  CGuiWidget* FindWidget(short id);

  short GetWidgetID() const { return mSelfId; }
  short GetWorkerId() const { return mWorkerId; }
  const CColor& GetColor() const { return mColor; }
  const CColor& GetModifiedColor() const { return mColor2; }
  const CTransform4f& GetIdleXform() const { return mTransform; }
  CGuiFrame* GetParentFrame() const { return mFrame; }

protected:
  short mSelfId;
  short mParentId;
  CTransform4f mTransform;
  CColor mColor;
  CColor mColor2;
  EGuiModelDrawFlags mDrawFlags;
  CGuiFrame* mFrame;
  short mWorkerId;
  bool mIsVisible : 1;
  bool mIsActive : 1;
  bool mIsSelectable : 1;
  bool mEventLock : 1;
  bool mCullFaces : 1;
  bool mDepthGreater : 1;
  bool mDepthTest : 1;
  bool mDepthWrite : 1;
  bool xbb_24_ : 1;
};
NESTED_CHECK_SIZEOF(CGuiWidget, CGuiWidgetParms, 0x18)
CHECK_SIZEOF(CGuiWidget, 0xbc)

#endif // _CGUIWIDGET
