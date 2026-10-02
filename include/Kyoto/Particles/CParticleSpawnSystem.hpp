#ifndef _CPARTICLESPAWNSYSTEM
#define _CPARTICLESPAWNSYSTEM

#include "types.h"

#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"

class CSpawnSystemDescription;

// Constructor signature from the Echoes Wii SEL export; runtime layout is target-derived.
class CParticleSpawnSystem : public CParticleGen {
public:
  CParticleSpawnSystem(TToken< CSpawnSystemDescription > desc,
                       CElementGen::EOptionalSystemFlags flags, bool modelsUseLights);
  // CParticleGen
  ~CParticleSpawnSystem() override;

  const bool Update(double dt) override;
  void Render() override;
  void SetOrientation(const CTransform4f& orientation) override;
  void SetTranslation(const CVector3f& translation) override;
  void SetGlobalOrientation(const CTransform4f& orientation) override;
  void SetGlobalTranslation(const CVector3f& translation) override;
  void SetGlobalScale(const CVector3f& scale) override;
  void SetLocalScale(const CVector3f& scale) override;
  void SetParticleEmission(bool emission) override;
  void SetModulationColor(const CColor& col) override;
  const CTransform4f& GetOrientation() const override;
  const CVector3f& GetTranslation() const override;
  const CTransform4f& GetGlobalOrientation() const override;
  const CVector3f& GetGlobalTranslation() const override;
  const CVector3f& GetGlobalScale() const override;
  bool GetParticleEmission() const override;
  const CColor& GetModulationColor() const override;
  int GetEmitterTime() const override;
  int GetSystemCount() override;
  bool IsSystemDeletable() override;
  rstl::optional_object< CAABox > GetBounds() override;
  int GetParticleCount() override;
  bool SystemHasLight() override;
  CLight GetLight() override;
  void DestroyParticles() override;
  uint Get4CharId() const override;

  // Additional virtual; guessed name, matching the PART lifetime operation it forwards.
  virtual void EndLifetime();

  // Guessed names: force the initial transforms through nested spawn systems.
  void ForceSetTranslation(const CVector3f& translation);
  void ForceSetOrientation(const CTransform4f& orientation);
  void ForceSetGlobalTranslation(const CVector3f& translation);
  void ForceSetGlobalOrientation(const CTransform4f& orientation);
  static ushort GetGlobalSeed() { return sSeed; }
  static void SetGlobalSeed(ushort seed) { sSeed = seed; }

private:
  // Guessed names for GIVL's native transform-propagation modes.
  enum ETranslationMode {
    kTM_None,
    kTM_Local,
    kTM_Global,
  };

  // Guessed names, based on the native property evaluation and child-system calls.
  void UpdateOrientation(bool evaluate, bool force, const CTransform4f* orientation);
  void UpdateTranslation(bool evaluate, bool force, const CVector3f* translation);
  void UpdateGlobalOrientation(bool evaluate, bool force, const CTransform4f* orientation);
  void UpdateGlobalTranslation(bool evaluate, bool force, const CVector3f* translation);
  void UpdateChildParticleSystems(double dt);
  CParticleGen* ConstructChildParticleSystem(const CToken& description, uint type, ushort seed);
  void BuildParticleSystemBounds();
  void UpdateVelocitySource(int index);
  CVector3f GetTranslationOffset() const;
  CVector3f GetGlobalTranslationOffset() const;

  static ushort sSeed; // Guessed name; constructor and child creation share this seed.
  // Guessed semantic names, inferred from native setters, property evaluation and child calls.
  TLockedToken< CSpawnSystemDescription > mDescription;
  rstl::vector< CParticleGen* > mChildren;
  CRandom16 mRandom;
  int mPrevFrame;
  int mCurFrame;
  double mCurSeconds;
  int mLifetime;
  int mParticleCount;
  CElementGen::EOptionalSystemFlags mOptionalFlags;
  bool mModelsUseLights : 1;
  bool mParticleEmission : 1;
  bool mIgnoreGlobalTransform : 1;
  bool mIgnoreLocalTransform : 1;
  ETranslationMode mTranslationMode;
  CVector3f mVelocity;
  CVector3f mTranslation;
  CTransform4f mOrientation;
  CVector3f mLocalScale;
  CMatrix3f mOrientationInverse;
  CVector3f mGlobalTranslation;
  CTransform4f mGlobalOrientation;
  CVector3f mGlobalScale;
  CVector3f mTranslationOffset;
  CTransform4f mOrientationOffset;
  CVector3f mLocalScaleMultiplier;
  CVector3f mGlobalTranslationOffset;
  CTransform4f mGlobalOrientationOffset;
  CVector3f mGlobalScaleMultiplier;
  CTransform4f mBillboardAxisTransform;
  CVector3f mBillboardAxis;
  CColor mParticleColor;
  CColor mModulationColor;
  CAABox mBounds;
  bool mVelocitySourceLocal[2];
  CModVectorElement* mVelocitySources[2];
};
CHECK_SIZEOF(CParticleSpawnSystem, 0x220)

#endif // _CPARTICLESPAWNSYSTEM
