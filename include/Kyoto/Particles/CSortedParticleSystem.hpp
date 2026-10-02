#ifndef _CSORTEDPARTICLESYSTEM
#define _CSORTEDPARTICLESYSTEM

#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/reserved_vector.hpp"

class CSortedParticleSystemDescription;

// Guessed name; SRSC batches up to eight PART children for sorted rendering.
// Its descriptor contains only SPWN, unlike CSpawnSystemDescription.
class CSortedParticleSystem : public CParticleGen {
public:
  CSortedParticleSystem(TToken< CSortedParticleSystemDescription > desc,
                        CElementGen::EOptionalSystemFlags flags, bool modelsUseLights);
  // CParticleGen
  ~CSortedParticleSystem() override;

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
  void SetGeneratorRate(float rate) override;
  const CTransform4f& GetOrientation() const override;
  const CVector3f& GetTranslation() const override;
  const CTransform4f& GetGlobalOrientation() const override;
  const CVector3f& GetGlobalTranslation() const override;
  const CVector3f& GetGlobalScale() const override;
  bool GetParticleEmission() const override;
  const CColor& GetModulationColor() const override;
  float GetGeneratorRate() const override;
  int GetEmitterTime() const override;
  int GetSystemCount() override;
  bool IsSystemDeletable() override;
  rstl::optional_object< CAABox > GetBounds() override;
  int GetParticleCount() override;
  bool SystemHasLight() override;
  CLight GetLight() override;
  void DestroyParticles() override;
  uint Get4CharId() const override;

  static ushort GetGlobalSeed() { return sSeed; }
  static void SetGlobalSeed(ushort seed) { sSeed = seed; }

private:
  // Guessed names, based on native keyframe spawning and child bounds aggregation.
  void UpdateChildParticleSystems(double dt);
  void BuildParticleSystemBounds();

  static ushort sSeed; // Guessed name; constructor and child creation share this seed.
  // Guessed semantic names, supported by constructor and runtime consumers.
  TLockedToken< CSortedParticleSystemDescription > mDescription;
  rstl::reserved_vector< CParticleGen*, 8 > mChildren;
  CRandom16 mRandom;
  int mCurFrame;
  double mCurSeconds;
  float mGeneratorRate;
  int mParticleCount;
  int mPrevFrame;
  CElementGen::EOptionalSystemFlags mOptionalFlags;
  bool mModelsUseLights : 1;
  bool mParticleEmission : 1;
  CVector3f mGlobalTranslation;
  CVector3f mGlobalScale;
  CTransform4f mGlobalOrientation;
  CAABox mBounds;
};
CHECK_SIZEOF(CSortedParticleSystem, 0xd8)

#endif // _CSORTEDPARTICLESYSTEM
