#ifndef _CSCRIPTRUBIKSPUZZLE
#define _CSCRIPTRUBIKSPUZZLE

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CGenericFSM2State.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRubiksPuzzle.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CActor;
class CStateManager;
class CTransform4f;

// Guessed class: a sliding-block puzzle made of nine pieces laid out in a three by three grid.
// The player presses one of four buttons to turn a two by two block of pieces a quarter turn.
// The puzzle is solved when every row holds the pieces that belong to it.
class CScriptRubiksPuzzle : public CEntity {
public:
  CScriptRubiksPuzzle(TUniqueId uid, const CEntityInfo& info, const rstl::string& name,
                      const SLdrRubiksPuzzleData& data);

  // CEntity
  ~CScriptRubiksPuzzle() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CScriptRubiksPuzzle
  void Start(CStateManager& mgr, int msg, float dt);        // Guessed name
  void Waiting(CStateManager& mgr, int msg, float dt);      // Guessed name
  void Rotating(CStateManager& mgr, int msg, float dt);     // Guessed name
  bool ButtonPressed(CStateManager& mgr, const float& arg); // Guessed name
  bool RotationOver(CStateManager& mgr, const float& arg);  // Guessed name

private:
  enum { kPieceCount = 9 };
  enum { kMaxQueuedPresses = 8 };

  // Guessed names: a piece with the row it belongs in when the puzzle is solved, and a button
  // press with the block of pieces it turns.
  typedef rstl::pair< TUniqueId, int > SPiece;
  typedef rstl::pair< TUniqueId, int > SButtonPress;

  // Guessed name: a growable array that is consumed from the front and never reuses the space in
  // front of its head.
  class CPressRing {
  public:
    CPressRing() : mFirst(0), mEnd(0), mCapacity(0), mItems(nullptr) {}
    ~CPressRing();

    int size() const { return mEnd - mFirst; }
    SButtonPress& front() { return mItems[mFirst]; }
    void pop_front() { ++mFirst; }
    void push_back(const SButtonPress& press);
    void clear();

  private:
    void erase(int first, int last);
    void grow(int count);
    void reserve(int capacity);

    int mUnused; // Guessed name; never initialised or read
    int mFirst;
    int mEnd;
    int mCapacity;
    SButtonPress* mItems;
  };

  // Guessed name: the button presses waiting for the current rotation to finish.
  class CPressQueue {
  public:
    CPressQueue() {}
    ~CPressQueue();

    int size() const { return mRing.size(); }
    SButtonPress& front() { return mRing.front(); }
    void pop_front() { mRing.pop_front(); }
    void push_back(const SButtonPress& press);

  private:
    CPressRing mRing;
  };

  void SortPieces(CStateManager& mgr);
  void RecordPiecePositions(CStateManager& mgr);
  void RestorePiecePositions(CStateManager& mgr);
  void SendToConnected(CStateManager& mgr, EScriptObjectState state,
                       const EScriptObjectMessage& msg);
  bool UpdateRow(CStateManager& mgr, bool force, int row, EScriptObjectState stateA,
                 EScriptObjectState stateB, EScriptObjectState stateC);
  void UpdateIndicators(CStateManager& mgr);

  SLdrRubiksPuzzleData mProperties;                                       // Guessed name
  rstl::optional_object< TToken< CGenericFSM2 > > mStateMachine;          // Guessed name
  rstl::single_ptr< CGenericFSM2State< CPatterned > > mStateMachineState; // Guessed name
  rstl::reserved_vector< SPiece, kPieceCount > mPieces;                   // Guessed name
  rstl::reserved_vector< CVector3f, kPieceCount > mPiecePositions;        // Guessed name
  TUniqueId mCenterId;                                                    // Guessed name
  TUniqueId mButtonId;                                                    // Guessed name
  int mBlock;                                                             // Guessed name
  float mRotationAngle;                                                   // Guessed name
  int x104_;
  uchar x108_[8];
  CPressQueue mPressQueue;   // Guessed name
  bool mRotating : 1;        // Guessed name
  bool mSolved : 1;          // Guessed name
  bool mTopRowLocked : 1;    // Guessed name
  bool mBottomRowLocked : 1; // Guessed name
};
CHECK_SIZEOF(CScriptRubiksPuzzle, 0x128)

#endif // _CSCRIPTRUBIKSPUZZLE
