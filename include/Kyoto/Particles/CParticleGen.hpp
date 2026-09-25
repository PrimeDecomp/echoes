#ifndef _CPARTICLEGEN
#define _CPARTICLEGEN

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CWarp.hpp"

#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"

class CWarp;

class CParticleGen {
public:
  CParticleGen() : mDrawFlags(0) {}
  virtual ~CParticleGen() {}
  virtual const bool Update(double) = 0;
  virtual void Render() = 0;
  virtual void SetOrientation(const CTransform4f& orientation) = 0;
  virtual void SetTranslation(const CVector3f& translation) = 0;
  virtual void SetGlobalOrientation(const CTransform4f& orientation) = 0;
  virtual void SetGlobalTranslation(const CVector3f& translation) = 0;
  virtual void SetGlobalScale(const CVector3f& scale) = 0;
  virtual void SetLocalScale(const CVector3f& scale) = 0;
  virtual void SetParticleEmission(bool emission) = 0;
  virtual void SetModulationColor(const CColor& col) = 0;
  virtual void SetGeneratorRate(float rate) {}
  // Names of the draw-flag methods are hypotheses based on the sDrawFlags/sDrawMask Wii exports.
  virtual void SetDrawFlags(uint flags) { mDrawFlags = flags; }
  virtual const CTransform4f& GetOrientation() const = 0;
  virtual const CVector3f& GetTranslation() const = 0;
  virtual const CTransform4f& GetGlobalOrientation() const = 0;
  virtual const CVector3f& GetGlobalTranslation() const = 0;
  virtual const CVector3f& GetGlobalScale() const = 0;
  virtual bool GetParticleEmission() const = 0;
  virtual const CColor& GetModulationColor() const = 0;
  virtual float GetGeneratorRate() const { return 1.f; }
  virtual int GetEmitterTime() const = 0;
  virtual uint GetDrawFlags() const { return mDrawFlags; }
  virtual bool ShouldDraw() const { return (GetDrawFlags() & sDrawMask) == sDrawFlags; }
  virtual int GetActiveParticleCount() = 0;
  virtual bool IsSystemDeletable() = 0;
  virtual rstl::optional_object< CAABox > GetBounds() = 0;
  virtual int GetParticleCount() = 0;
  virtual bool SystemHasLight() = 0;
  virtual CLight GetLight() = 0;
  virtual void DestroyParticles() = 0;
  virtual void AddModifier(CWarp*);
  virtual uint Get4CharId() const = 0;

  static uint sDrawFlags;
  static uint sDrawMask;

private:
  rstl::list< CWarp* > x4_modifiersList;
  uint mDrawFlags;
};

#endif // _CPARTICLEGEN
