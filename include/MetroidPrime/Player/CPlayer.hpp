#ifndef _CPLAYER
#define _CPLAYER

#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"

class CMorphBall;
class CPlayerState;
class CCameraManager;

class CPlayer : public CPhysicsActor {
public:
  enum EPlayerMorphBallState {
    kMS_Unmorphed,
    kMS_Morphed,
    kMS_Morphing,
    kMS_Unmorphing,
  };

  int GetPlayerIndex() const;
  const CVector3f& GetLookDir() const { return m_lookDir; };
  CMorphBall* GetMorphBall() { return m_morphBall; }
  CPlayerState* GetPlayerState() { return m_playerState; }

  EPlayerMorphBallState GetMorphballTransitionState() const { return m_morphBallState; }
  EPlayerMorphBallState GetSpawnedMorphballState() const { return m_spawnedMorphBallState; }
  float fn_8000BE98() const;
  void fn_80019E40(CStateManager&, int);

  void Teleport(const CTransform4f& xf, CStateManager& mgr, bool resetBallCam);
  void SetSpawnedMorphBallState(EPlayerMorphBallState state, CStateManager& mgr);
  const CCameraManager* GetCameraManager() const { return m_cameraManager; }

private:
  char m_pad_1[0xBC]; // 0x2d0
  EPlayerMorphBallState m_morphBallState; // 0x38c
  EPlayerMorphBallState m_spawnedMorphBallState; // 0x390
  char m_pad_2[0xC3C]; // 0x394
  CVector3f m_lookDir; // 0xfd0
  char m_pad_3[0x198]; // 0xFDC
  CMorphBall* m_morphBall; // 0x1174
  char m_pad_4[0x19C]; // 0x1178
  CPlayerState* m_playerState; // 0x1314
  CCameraManager* m_cameraManager; // 0x1318
};

#endif // _CPLAYER
