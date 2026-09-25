#ifndef _CPARTICLESPAWNSYSTEM
#define _CPARTICLESPAWNSYSTEM

#include "types.h"

#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"

class CSpawnSystemDescription;

// Constructor signature from the Echoes Wii SEL export; layout is unrecovered.
class CParticleSpawnSystem : public CParticleGen {
public:
  CParticleSpawnSystem(TToken< CSpawnSystemDescription > desc,
                       CElementGen::EOptionalSystemFlags flags, bool);
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
  int GetActiveParticleCount() override;
  bool IsSystemDeletable() override;
  rstl::optional_object< CAABox > GetBounds() override;
  int GetParticleCount() override;
  bool SystemHasLight() override;
  CLight GetLight() override;
  void DestroyParticles() override;
  uint Get4CharId() const override;

private:
  uchar x20_[0x200];
};
CHECK_SIZEOF(CParticleSpawnSystem, 0x220)

class CSpawnRandomDescription;

// Guessed name. Generator for 'SRSC' effects (G2ME01 ctor 0x8032ECD8, vtable 0x803BB368);
// its asset uses the SPSC property layout. Parameters mirror CParticleSpawnSystem's call.
class CParticleSpawnRandom : public CParticleGen {
public:
  CParticleSpawnRandom(TToken< CSpawnRandomDescription > desc,
                       CElementGen::EOptionalSystemFlags flags, bool);
  ~CParticleSpawnRandom() override;

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
  int GetActiveParticleCount() override;
  bool IsSystemDeletable() override;
  rstl::optional_object< CAABox > GetBounds() override;
  int GetParticleCount() override;
  bool SystemHasLight() override;
  CLight GetLight() override;
  void DestroyParticles() override;
  uint Get4CharId() const override;

private:
  uchar x20_[0xb8];
};
CHECK_SIZEOF(CParticleSpawnRandom, 0xd8)

#endif // _CPARTICLESPAWNSYSTEM
