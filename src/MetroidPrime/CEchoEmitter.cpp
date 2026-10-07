#include "MetroidPrime/CEchoEmitter.hpp"

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGXTransientBuffer.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/reserved_vector.hpp"
#include <dolphin/os.h>
#include <string.h>

CEchoEmitter::CEchoEmitter(const CAABox& bounds, const SEchoParameters& parameters)
: mParameters(parameters)
, mBounds(bounds)
, mDamage(0.f)
, mYellowDamage(0.f)
, mDamageReductionRate(0.f)
, mYellowDamageReductionRate(0.f)
, mActive(true)
, mPendingDeletion(true)
, mNextEmitter(nullptr) {
  ResetPlayerState();
}

void CEchoEmitter::ResetPlayerState() {
  for (int i = 0; i < 4; ++i) {
    mPlayerEchoVisibility[i] = 0.f;
    mPlayerEchoTokens[i] = 0;
  }
}

void CEchoEmitter::ResetPlayerState(CStateManager& mgr) {
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    mPlayerEchoTokens[i] = mgr.GetPlayer(i)->GetEchoPulseCounter() - 1;
    mPlayerEchoVisibility[i] = 0.f;
  }
}

CEchoEmitter::~CEchoEmitter() {}

void CEchoEmitter::CreateEmitter(CStateManager& mgr) {
  mActive = true;
  mPendingDeletion = false;
  ResetPlayerState(mgr);
}

void CEchoEmitter::DestroyEmitter(CStateManager& mgr) { mPendingDeletion = true; }

void CEchoEmitter::Think(float dt, CStateManager& mgr) {
  if (!mActive) {
    return;
  }

  const CVector3f center = mBounds.GetCenterPoint();
  for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
    const CPlayer& player = *mgr.GetPlayer(i);
    float visibility =
        rstl::max_val(0.f, mPlayerEchoVisibility[i] - dt / mParameters.mVisibilityDecayTime);
    const uint pulseCounter = player.GetEchoPulseCounter();
    if (pulseCounter != mPlayerEchoTokens[i]) {
      const float dx = center.GetX() - player.GetTranslation().GetX();
      const float dy = center.GetY() - player.GetTranslation().GetY();
      const float radius = player.GetEchoPulsePhase() * gpTweakGui->GetEchoPulseRadiusScale();
      if (dx * dx + dy * dy + 0.f <= radius * radius) {
        visibility = 1.f;
        mPlayerEchoTokens[i] = pulseCounter;
      }
    }
    mPlayerEchoVisibility[i] = visibility;
  }

  mDamage = rstl::max_val(0.f, mDamage - dt * mDamageReductionRate);
  mYellowDamage = rstl::max_val(0.f, mYellowDamage - dt * mYellowDamageReductionRate);
}

void CEchoEmitter::Render(const CStateManager& mgr) const {
  if (mgr.GetRenderVisorMode() != CStateManager::kRVM_Echo || !mActive) {
    return;
  }

  rstl::reserved_vector< CVector3f, 8 > corners;
  for (int i = 0; i < 8; ++i) {
    corners.push_back(mBounds.GetPoint(i));
  }
  const SProjection projection =
      ProjectPoints(corners.data(), corners.size(), corners.data(), nullptr);
  const float depth = projection.mProjectedCenter.GetY();
  float minX = corners[0].GetX();
  float minZ = corners[0].GetZ();
  float maxX = minX;
  float maxZ = minZ;
  for (int i = 1; i < 8; ++i) {
    if (corners[i].GetX() < minX) {
      minX = corners[i].GetX();
    }
    if (corners[i].GetX() > maxX) {
      maxX = corners[i].GetX();
    }
    if (corners[i].GetZ() < minZ) {
      minZ = corners[i].GetZ();
    }
    if (corners[i].GetZ() > maxZ) {
      maxZ = corners[i].GetZ();
    }
  }

  rstl::reserved_vector< CVector3f, 4 > contour;
  contour.push_back(CVector3f(minX, depth, minZ));
  contour.push_back(CVector3f(maxX, depth, minZ));
  contour.push_back(CVector3f(maxX, depth, maxZ));
  contour.push_back(CVector3f(minX, depth, maxZ));
  DrawContour(contour.data(), contour.size(), 8, projection, mgr);
}

void CEchoEmitter::DrawContour(const CVector3f* points, int count, int subdivisions,
                               const SProjection& projection, const CStateManager& mgr) const {
  if (!mParameters.mOnlyEmitDamage) {
    const float visibility = mPlayerEchoVisibility[mgr.GetCurrentRenderPlayer()->GetPlayerIndex()];
    const float inverse = 1.f - visibility;
    DrawWaves(points, count, subdivisions, projection, gpTweakGui->GetEchoOutlineColor(),
              1.f + inverse, inverse, visibility);
  }
  if (mDamage > 0.f) {
    const float inverse = 1.f - mDamage;
    DrawWaves(points, count, subdivisions, projection, gpTweakGui->GetEchoDamageColor(),
              1.f + inverse, inverse, mDamage);
  }
  if (mYellowDamage > 0.f) {
    const float inverse = 1.f - mYellowDamage;
    DrawWaves(points, count, subdivisions, projection, gpTweakGui->GetEchoYellowDamageColor(),
              1.f + inverse, inverse, mYellowDamage);
  }
}

CEchoEmitter::SProjection CEchoEmitter::ProjectPoints(const CVector3f* points, int count,
                                                      CVector3f* output, const int* indices) {
  CVector3f center = points[0];
  for (int i = 1; i < count; ++i) {
    center += points[i];
  }
  center *= 1.f / float(count);
  CTransform4f rotation =
      CTransform4f::LookAt(CGraphics::GetViewPoint(), center, CGraphics::GetViewMatrix().GetUp());
  rotation.SetTranslation(CVector3f::Zero());
  const CTransform4f inverse = rotation.GetQuickInverse();
  const float depth = (inverse * center).GetY();
  for (int i = 0; i < count; ++i) {
    const int index = indices != nullptr ? indices[i] : i;
    output[index] = inverse * points[i];
    output[index].SetY(depth);
  }
  return SProjection(rotation, inverse * center);
}

void CEchoEmitter::EnsureMinimumWidth(CVector3f& left, CVector3f& right, float width) {
  if (right.GetX() < left.GetX()) {
    rstl::swap(left, right);
  }
  const float padding = width - (right.GetX() - left.GetX());
  if (padding > 0.f) {
    left.SetX(left.GetX() - padding * 0.5f);
    right.SetX(right.GetX() + padding * 0.5f);
  }
}

void CEchoEmitter::LimitHeight(CVector3f& point, float origin, float height) {
  const float delta = point.GetZ() - origin;
  if (height < 0.f) {
    if (delta > height) {
      point.SetZ(origin + height);
    }
  } else if (delta < height) {
    point.SetZ(origin + height);
  }
}

CVector3f CEchoEmitter::InterpolateContourPoint(const CVector3f& first, const CVector3f& second,
                                                const CVector3f& target, float amount,
                                                float minimumDistance, float maximumDistance) {
  const CVector3f midpoint = (first + second) * 0.5f;
  const float distance = (second - first).Magnitude();
  float weight;
  if (distance <= minimumDistance) {
    weight = 0.f;
  } else if (distance >= maximumDistance) {
    weight = 1.f;
  } else {
    weight = (distance - minimumDistance) / (maximumDistance - minimumDistance);
  }
  return CVector3f::Lerp(midpoint, target, weight * amount);
}

void CEchoEmitter::DrawWaves(const CVector3f* points, int count, int subdivisions,
                             const SProjection& projection, const CColor& color, float scale,
                             float spacing, float alpha) const {
  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvConstColor);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGraphics::DisableAllLights();
  CGraphics::SetLineWidth(mParameters.mWaveLineSize, kTO_One);

  // Guessed packet name; one NOP aligns the following position payload to four bytes.
  struct SLineStripHeader {
    uchar mNop;
    uchar mCommand;
    ushort mVertexCount;
  };
  const int vertexCount = count * subdivisions + 1;
  const int usedBytes = sizeof(SLineStripHeader) + vertexCount * sizeof(CVector3f);
  const int allocationSize = (usedBytes + 31) & ~31;
  SLineStripHeader* header =
      static_cast< SLineStripHeader* >(CGXTransientBuffer::EnsureAllocation(allocationSize));
  header->mNop = GX_NOP;
  header->mCommand = GX_LINESTRIP;
  header->mVertexCount = vertexCount;
  CVector3f* vertices = reinterpret_cast< CVector3f* >(header + 1);
  int vertex = 0;
  const float step = 1.f / float(subdivisions);
  for (int i = 0; i < count; ++i) {
    int next = i + 1;
    int after = i + 2;
    int last = i + 3;
    if (next >= count)
      next -= count;
    if (after >= count)
      after -= count;
    if (last >= count)
      last -= count;
    const CVector3f a = points[i];
    const CVector3f b = points[next];
    const CVector3f c = points[after];
    const CVector3f d = points[last];
    CVector3f edge = c - b;
    const float length = edge.Magnitude();
    edge.Normalize();
    CVector3f previous = a - b;
    previous.Normalize();
    CVector3f firstTangent = edge - previous;
    if (firstTangent.CanBeNormalized())
      firstTangent.Normalize();
    else
      firstTangent = CVector3f(0.f, 1.f, 0.f);
    firstTangent *= length;
    CVector3f secondTangent = (d - c).AsNormalized() + edge;
    if (secondTangent.CanBeNormalized())
      secondTangent.Normalize();
    else
      secondTangent = CVector3f(0.f, 1.f, 0.f);
    secondTangent *= length;

    float t = 0.f;
    for (int j = 0; j < subdivisions; ++j) {
      vertices[vertex++] = CMath::GetHermiteSplinePoint(b, c, firstTangent, secondTangent, t);
      t += step;
    }
  }

  CGraphics::SetLineWidth(3.f, kTO_One);
  gpRender->DisableDestinationAlpha();
  const CVector3f center = projection.mRotation * projection.mProjectedCenter;
  CGraphics::SetTevRegisterColor(0, color.WithAlphaModulatedBy(0.75f * alpha));
  CGX::ResetVtxDescv();
  CGX::SetVtxDesc(GX_VA_POS, GX_DIRECT);
  vertices[vertex] = vertices[0];
  if (usedBytes != allocationSize) {
    memset(reinterpret_cast< uchar* >(header) + usedBytes, 0, allocationSize - usedBytes);
  }
  DCFlushRange(header, allocationSize);

  const float increment = spacing * mParameters.mSpaceBetweenWaves + mParameters.mSpaceBetweenWaves;
  for (uint wave = 0; wave < mParameters.mNumSoundWaves; ++wave) {
    const CTransform4f model = CTransform4f::Translate(center) * CTransform4f::Scale(scale) *
                               CTransform4f::Translate(-center) * projection.mRotation;
    CGraphics::SetModelMatrix(model);
    CGX::CallDisplayList(header, allocationSize);
    scale += increment;
  }
  CGXTransientBuffer::ReleaseAllocation();
  CGraphics::SetLineWidth(1.f, kTO_One);
  gpRender->SetDestinationAlpha(0);
}

CEchoEmitter::SProjection::SProjection(const CTransform4f& rotation,
                                       const CVector3f& projectedCenter)
: mRotation(rotation), mProjectedCenter(projectedCenter) {}

void CEchoEmitter::RenderEmitters(const CStateManager& mgr, const CEchoEmitter* first) {
  for (const CEchoEmitter* emitter = first; emitter != nullptr; emitter = emitter->mNextEmitter) {
    emitter->Render(mgr);
  }
}

void CEchoEmitter::SetDamageExplicit(float damage) {
  mDamage = damage;
  mDamageReductionRate = 0.f;
}

void CEchoEmitter::SetAutoReducingDamage(float duration) {
  mDamage = 1.f;
  if (duration > 0.f) {
    mDamageReductionRate = 1.f / duration;
  } else {
    mDamageReductionRate = 1.f;
  }
}

void CEchoEmitter::SetAutoReducingYellowDamage(float duration) {
  mYellowDamage = 1.f;
  if (duration > 0.f) {
    mYellowDamageReductionRate = 1.f / duration;
  } else {
    mYellowDamageReductionRate = 1.f;
  }
}

void CEchoEmitter::TriggerDamageEcho() { SetAutoReducingDamage(mParameters.mVisibilityDecayTime); }
