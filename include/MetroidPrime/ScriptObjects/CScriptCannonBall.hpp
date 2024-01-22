#ifndef _CSCRIPTCANNONBALL
#define _CSCRIPTCANNONBALL

#include "MetroidPrime/CActor.hpp"
#include "rstl/reserved_vector.hpp"

class CScriptCannonBall : public CActor {
public:
  struct TrackedShot {
    TrackedShot(TUniqueId, bool);

    TUniqueId m_scriptObject;
    float m_f;
    int m_updateFrameIdx;
    bool m_b : 1;
    bool m_flag2 : 1;

    void Think(float dt, CStateManager& mgr, int i);
    void FreeScriptObject(CStateManager& mgr);
    void OnIncrementMsg(CStateManager& mgr, int);
  };

  CScriptCannonBall(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, CAssetId effect);
  ~CScriptCannonBall();

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;

private:
  CAssetId m_effect;
  rstl::reserved_vector< TrackedShot, 4 > m_fields;
};

CEntity* LoadCannonBall(CStateManager& mgr, CInputStream& input, CEntityInfo& info);

#endif // _CSCRIPTCANNONBALL
