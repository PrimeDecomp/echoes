#include "WorldFormat/CAreaOctTree.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Kyoto/Math/CLine.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "float.h"

static bool _close_enough(float a, float b, float epsilon) { return CMath::AbsF(a - b) <= epsilon; }

static bool BoxLineTest(const CAABox& box, const CLine& line, float& lowT, float& highT) {
  const CVector3f& min = box.GetMinPoint();
  const CVector3f& max = box.GetMaxPoint();
  const CVector3f& origin = line.GetRefPoint();
  const CUnitVector3f& direction = line.GetNormal();
  lowT = -FLT_MAX;
  highT = FLT_MAX;

  for (int i = 0; i < 3; ++i) {
    if (_close_enough(direction[i], 0.f, 0.0001f)) {
      if (origin[i] < min[i] || origin[i] > max[i]) {
        return false;
      }
    } else {
      const float reciprocal = 1.f / direction[i];
      if (direction[i] < 0.f) {
        const float nearDistance = max[i] - origin[i];
        const float farDistance = min[i] - origin[i];
        if (nearDistance < lowT * direction[i]) {
          lowT = nearDistance * reciprocal;
        }
        if (farDistance > highT * direction[i]) {
          highT = farDistance * reciprocal;
        }
      } else {
        const float nearDistance = min[i] - origin[i];
        const float farDistance = max[i] - origin[i];
        if (nearDistance > lowT * direction[i]) {
          lowT = nearDistance * reciprocal;
        }
        if (farDistance < highT * direction[i]) {
          highT = farDistance * reciprocal;
        }
      }
    }
  }
  return lowT <= highT;
}

void CAreaOctTree::Node::LineTestEx(const CLine& line, const CMaterialFilter& filter,
                                    SRayResult& result, float length) const {
  if (mNodeType == kTT_Invalid) {
    return;
  }

  float lowT = 0.f;
  float highT = 0.f;
  if (!BoxLineTest(mAabb, line, lowT, highT)) {
    return;
  }

  LineTestExInternal(line, filter, result, lowT - 0.0001f, highT + 0.0001f, length,
                     CVector3f(1.f / line.GetNormal().GetX(), 1.f / line.GetNormal().GetY(),
                               1.f / line.GetNormal().GetZ()));
}

bool CAreaOctTree::Node::LineTest(const CLine& line, const CMaterialFilter& filter,
                                  float length) const {
  if (mNodeType == kTT_Invalid) {
    return true;
  }

  float lowT = 0.f;
  float highT = 0.f;
  if (!BoxLineTest(mAabb, line, lowT, highT)) {
    return true;
  }

  return LineTestInternal(line, filter, lowT - 0.0001f, highT + 0.0001f, length,
                          CVector3f(1.f / line.GetNormal().GetX(), 1.f / line.GetNormal().GetY(),
                                    1.f / line.GetNormal().GetZ()));
}

bool CAreaOctTree::Node::LineTestInternal(const CLine& line, const CMaterialFilter& filter,
                                          float lowT, float highT, float maxT,
                                          const CVector3f& directionReciprocal) const {
  // TODO: Traverse intersected children and test leaf triangles with the material
  // filter. The target returns true when the ray is clear, not when a hit is found.
  return true;
}

void CAreaOctTree::Node::LineTestExInternal(const CLine& line, const CMaterialFilter& filter,
                                            SRayResult& result, float lowT, float highT, float maxT,
                                            const CVector3f& directionReciprocal) const {
  // TODO: Traverse in ray order and fill the nearest accepted surface, plane and t.
}
