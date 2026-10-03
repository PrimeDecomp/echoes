#ifndef _CSCANTREENODE
#define _CSCANTREENODE

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/string.hpp"

class CStringTable;
class SLdrTransform;

// Class, method and member names are guessed.
class CScanTreeNode {
public:
  enum ENodeType {
    kNT_Category = 0,
    kNT_Scan = 1,
    kNT_Inventory = 2,
    kNT_Menu = 3,
    kNT_Slider = 4
  };

  CScanTreeNode(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                const rstl::string& nameStringName);
  virtual ~CScanTreeNode() = 0;
  virtual void LockResources();
  virtual void UnlockResources();
  virtual bool AreResourcesLoaded();
  virtual ENodeType GetNodeType() const = 0;

  void SetViewed(bool viewed);
  bool IsViewed() const;
  void SetVisibleDescendantCount(int count);
  int GetVisibleDescendantCount() const;
  void SetDescendantCount(int count);
  int GetDescendantCount() const;
  void SetOpacity(float opacity);
  float GetOpacity() const;
  void SetVisible(bool visible);
  bool IsVisible() const;
  void SetAcceleration(const CVector3f& acceleration);
  CVector3f GetAcceleration() const;
  void SetVelocity(const CVector3f& velocity);
  CVector3f GetVelocity() const;
  const rstl::string& GetNameStringName() const;
  rstl::wstring GetName() const;
  void SetParentNode(int node);
  int GetParentNode() const;
  int GetId() const;
  void SetPosition(const CVector3f& position);
  const CVector3f& GetPosition() const;
  void SetDisplayPosition(const CVector3f& position);
  const CVector3f& GetDisplayPosition() const;

private:
  CVector3f mDisplayPosition;
  CVector3f mPosition;
  CVector3f mVelocity;
  CVector3f mAcceleration;
  int mId;
  int mParentNode;
  int mDescendantCount;
  int mVisibleDescendantCount;
  float mOpacity;
  rstl::string mNameStringName;
  rstl::auto_ptr< TCachedToken< CStringTable > > mNameStringTable;
  bool mVisible;
  bool mViewed;
};
CHECK_SIZEOF(CScanTreeNode, 0x64)

#endif // _CSCANTREENODE
