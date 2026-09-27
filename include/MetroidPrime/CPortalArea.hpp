#ifndef _CPORTALAREA
#define _CPORTALAREA

class CActor;
class CStateManager;

// Partial interface; layout is not yet recovered. Names are inferred.
class CPortalArea {
public:
  void UpdateActor(CStateManager& mgr, CActor& actor);
};

#endif // _CPORTALAREA
