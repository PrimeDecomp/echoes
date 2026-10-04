#ifndef _CGUIWIDGET
#define _CGUIWIDGET

#include "GuiSys/CGuiObject.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"

class CFinalInput;
class CGuiFrame;
class CGuiWidgetDrawParms;
class CInputStream;
class CSimplePool;

enum ETraversalMode { kTM_ChildrenAndSiblings = 0, kTM_Children = 1, kTM_Single = 2 };

class CGuiWidget : public CGuiObject {
public:
  // Guessed names: select the frame's independent widget lists.
  enum EWidgetUsageFlags {
    kWUF_None = 0,
    kWUF_Draw = 1,
    kWUF_Update = 2,
    kWUF_Input = 4,
    kWUF_PreDraw = 8
  };

  enum EGuiModelDrawFlags {
    kGMDF_Shadeless = 0,
    kGMDF_Opaque = 1,
    kGMDF_Alpha = 2,
    kGMDF_Additive = 3,
    kGMDF_AlphaAdditiveOverdraw = 4,
    // Guessed names: additional Echoes flat-model draw modes.
    kGMDF_ClearAlpha = 5,
    kGMDF_DoubleColor = 6
  };

  class CGuiWidgetParms {
  public:
    CGuiWidgetParms(CGuiFrame* frame, short selfId, short parentId, const CColor& color,
                    EGuiModelDrawFlags drawFlags, bool cullFaces, bool defaultVisible,
                    bool defaultActive, bool depthTest, bool depthWrite, bool depthGreater);

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
  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool, uint version);
  // Guessed name: the legacy GRUP factory now constructs an ordinary widget.
  static CGuiWidget* CreateGroup(CGuiFrame* frame, CInputStream& in, CSimplePool* pool,
                                 uint version);
  static CGuiWidgetParms ReadWidgetHeader(CGuiFrame* frame, CInputStream& in);
  void ParseBaseInfo(CGuiFrame* frame, CInputStream& in, const CGuiWidgetParms& parms,
                     uint version);

  // CGuiObject
  ~CGuiWidget() override;

  virtual FourCC GetWidgetTypeID() const { return 'BWIG'; }
  virtual EWidgetUsageFlags GetWidgetUsageFlags() const { return kWUF_None; } // Guessed name
  virtual bool AddWorkerWidget(CGuiWidget* worker) { return false; }
  virtual bool GetIsActive() const { return mIsActive; }
  virtual bool GetIsVisible() const { return mIsVisible; }
  virtual void Update(float dt);
  virtual void Draw(const CGuiWidgetDrawParms& parms) const;
  virtual void ProcessUserInput(const CFinalInput& input);
  virtual CGuiWidget* GetWorkerWidget(int workerId);
  virtual void OnVisible();
  virtual void OnActivate();
  virtual void Initialize() {}

  void SetIsVisible(bool visible);
  void SetIsActive(bool active);
  void SetColor(const CColor& color);
  void SetVisibility(bool visible, ETraversalMode mode);
  void RecalcWidgetColor(ETraversalMode mode);
  void DispatchInitialize();
  void ReapplyXform();
  void SetIdleXform(const CTransform4f& xf, bool reapply = true);
  CVector3f GetIdlePosition() const;
  void AddChildWidget(CGuiWidget* widget, bool makeWorldLocal, bool atEnd);
  CGuiWidget* FindWidget(short id);

  short GetWidgetID() const { return mSelfId; }
  short GetWorkerId() const { return mWorkerId; }

  bool GetIsSelectable() const { return mIsSelectable; }

  const CColor& GetColor() const { return mColor; }
  const CColor& GetModifiedColor() const { return mColor2; }
  const CTransform4f& GetIdleXform() const { return mTransform; }
  CGuiFrame* GetParentFrame() const { return mFrame; }

  void SetDepthTest(bool enabled) { mDepthTest = enabled; }

  void SetDepthWrite(bool enabled) { mDepthWrite = enabled; }

protected:
  void ReadUnusedThing(CInputStream& in);

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
