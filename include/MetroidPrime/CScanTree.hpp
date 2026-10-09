#ifndef _CSCANTREE
#define _CSCANTREE

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/vector.hpp"

class CDvdRequest;
class CScanTreeNode;
class CScanTreeCategory;
class CStateManager;
class CInputStream;

// Class, method and member names are guessed.
class CScanTree {
public:
  CScanTree();
  ~CScanTree();

  CVector3f CalculateNeighborForce(const rstl::rc_ptr< CScanTreeCategory >& category,
                                   int node) const;
  CVector3f CalculateSeparationForce(const rstl::rc_ptr< CScanTreeCategory >& category,
                                     int node) const;
  CVector3f CalculatePairForce(float radius, float strength, const CVector3f& position,
                               const CVector3f& otherPosition) const;
  bool UpdateCategoryViewed(int node);
  void UpdateViewedCategories();
  void MarkViewed(CStateManager& mgr, int node);
  void RefreshViewed(CStateManager& mgr);
  void Update(float dt);
  void UpdateNodePhysics(float dt);
  float GetLayoutProgress() const;
  void UpdateLayout(float dt);
  void ScaleChildren(int node, float otherScale, float selectedScale, bool excludeOptions);
  int GetRootNode() const;
  rstl::rc_ptr< CScanTreeNode > GetNode(int node) const;
  bool IsLoaded() const;
  float GetTransition() const;
  int GetPreviousNode() const;
  int GetSelectedNode() const;
  void SelectScan(CAssetId scannableInfo, float duration);
  void SelectNode(int node, float duration);
  void SelectNode(int node);
  bool PollLoad();
  void InitializeNodePositions(int node);
  void RandomizeChildPositions(int node);
  rstl::pair< uint, uint > GetScanCounts() const;
  void RefreshVisibility(CStateManager& mgr);
  bool UpdateNodeVisibility(CStateManager& mgr, int node);
  void InitializeHierarchy();
  void UpdateDescendantCounts();
  void SetRootNode(int node);
  void AddNode(CScanTreeNode* node);
  void ReserveNodes(int count);
  void LoadAsync();

private:
  int mSelectedNode;
  int mPreviousNode;
  float mTransition;
  float mTransitionDuration;
  float mInitialLayoutTransition;
  rstl::auto_ptr< uchar > mBuffer;
  uint mBufferLength;
  rstl::auto_ptr< CDvdRequest > mLoadRequest;
  rstl::vector< rstl::ncrc_ptr< CScanTreeNode > > mNodes;
  int mRootNode;
  CRandom16 mRandom;
};
CHECK_SIZEOF(CScanTree, 0x40)

// Guessed parser and factory names.
void ReadScanTree(CInputStream& input, CScanTree& tree);
CScanTreeNode* LoadScanTreeNode(uint type, int* id, const rstl::vector< int >& children,
                                CInputStream& input);

#endif // _CSCANTREE
