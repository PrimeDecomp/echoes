#ifndef _CPORTALTRANSITION
#define _CPORTALTRANSITION

// Guessed name. Pointer-only interface; the object layout is not yet declared.
// TODO: Recover the complete class before constructing or embedding it.
class CPortalTransition {
public:
  ~CPortalTransition();
  bool IsReady() const;
  void Draw() const;
  void Update(float dt);
  void TouchModels();
  bool IsFinished() const;
};

#endif // _CPORTALTRANSITION
