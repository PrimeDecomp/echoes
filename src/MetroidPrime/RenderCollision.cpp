#include "MetroidPrime/RenderCollision.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Math/CLine.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"

#include <float.h>

namespace {
const CMaterialList skImplicitWorldMaterials(kMT_Unknown59, kMT_Unknown60);

inline int ReadSignedDisplayListShort(const uchar* data) {
  return static_cast< short >((uint(data[0]) << 8) | data[1]);
}

inline bool IntersectTriangle(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                              const CLine& line, const CMaterialList& material,
                              CRayCastResult& result, float& nearest) {
  const CVector3f edge1 = b - a;
  const CVector3f edge2 = c - a;
  const CVector3f cross0 = CVector3f::Cross(line.GetNormal(), edge2);
  const float determinant = CVector3f::Dot(edge1, cross0);
  if (determinant < 10.f * FLT_EPSILON) {
    return false;
  }

  const float inverse = 1.f / determinant;
  const CVector3f displacement = line.GetRefPoint() - a;
  const float u = inverse * CVector3f::Dot(displacement, cross0);
  if (u < 0.f || !(u <= 1.f)) {
    return false;
  }

  const CVector3f cross1 = CVector3f::Cross(displacement, edge1);
  const float distance = inverse * CVector3f::Dot(cross1, edge2);
  if (distance < 0.f || distance >= nearest) {
    return false;
  }

  const float v = inverse * CVector3f::Dot(cross1, line.GetNormal());
  if (v < 0.f || !(u + v <= 1.f)) {
    return false;
  }

  nearest = distance;
  result = CRayCastResult(distance, line.GetRefPoint() + distance * line.GetNormal(),
                          CPlane(a, b, c), material);
  return true;
}
} // namespace

int RenderCollision::RaySurfaceIntersection(const CCubeSurface& surface,
                                            const CCubeMaterial& material,
                                            const CVector3f* positions, const CLine& line,
                                            CRayCastResult& result, float& nearest) {
  const uint descriptor = material.GetVertexDesc();
  int stride = 0;
  for (int i = 0; i < 16; ++i) {
    const uint type = (descriptor >> (i * 2)) & 3;
    if (type == 2) {
      ++stride;
    } else if (type == 3) {
      stride += 2;
    }
  }

  CMaterialList hitMaterial(material.GetMaterialMask());
  hitMaterial.Add(kMT_Unknown59);
  hitMaterial.Add(kMT_Unknown60);
  const uchar* displayList = static_cast< const uchar* >(surface.GetDisplayList());
  int triangles = 0;
  int offset = 0;
  while (offset < int(surface.GetDisplayListSize())) {
    const uint primitive = displayList[offset++] & 0xfc;
    if (primitive == 0) {
      continue;
    }
    const int count = ReadSignedDisplayListShort(displayList + offset);
    offset += 2;
    const uchar* vertices = displayList + offset;
    switch (primitive) {
    case GX_TRIANGLESTRIP:
      triangles += count - 2;
      for (int i = 2; i < count; ++i) {
        const int a = (i & 1) ? i - 1 : i - 2;
        const int b = (i & 1) ? i - 2 : i - 1;
        IntersectTriangle(positions[ReadSignedDisplayListShort(vertices + a * stride)],
                          positions[ReadSignedDisplayListShort(vertices + b * stride)],
                          positions[ReadSignedDisplayListShort(vertices + i * stride)], line,
                          hitMaterial, result, nearest);
      }
      break;
    case GX_TRIANGLES:
      triangles += count / 3;
      for (int i = 0; i < count; i += 3) {
        IntersectTriangle(positions[ReadSignedDisplayListShort(vertices + i * stride)],
                          positions[ReadSignedDisplayListShort(vertices + (i + 1) * stride)],
                          positions[ReadSignedDisplayListShort(vertices + (i + 2) * stride)], line,
                          hitMaterial, result, nearest);
      }
      break;
    case GX_TRIANGLEFAN:
      triangles += count - 2;
      for (int i = 2; i < count; ++i) {
        IntersectTriangle(positions[ReadSignedDisplayListShort(vertices)],
                          positions[ReadSignedDisplayListShort(vertices + (i - 1) * stride)],
                          positions[ReadSignedDisplayListShort(vertices + i * stride)], line,
                          hitMaterial, result, nearest);
      }
      break;
    }
    offset += count * stride;
  }
  return triangles;
}

CRayCastResult RenderCollision::RayWorldIntersection(const CStateManager& mgr,
                                                     const CVector3f& origin,
                                                     const CVector3f& direction, float length,
                                                     const CMaterialFilter& filter,
                                                     rstl::pair< TAreaId, int >* modelOut) {
  CRayCastResult result;
  if (length <= 0.f) {
    length = 100000.f;
  }
  float nearest = length;
  const CMaterialFilter worldFilter = filter.WithImplicitMaterials(skImplicitWorldMaterials);
  if (worldFilter.GetType() == CMaterialFilter::kFT_Never) {
    return result;
  }

  const CLine line(origin, CUnitVector3f(direction, CUnitVector3f::kN_No));
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  bounds.AccumulateBounds(origin);
  bounds.AccumulateBounds(origin + length * direction);
  const CWorld& world = *mgr.GetWorld();
  for (CGameArea::CConstChainIterator area = world.GetChainHead(CWorld::kC_Alive);
       area != CWorld::skGlobalEnd; ++area) {
    if (area->GetOcclusionState() != CGameArea::kOS_Visible) {
      continue;
    }
    const CGameArea::CPostConstructed& post = *area->GetPostConstructed();
    if (!post.mRenderOctTree) {
      continue;
    }
    const CAreaRenderOctTree& tree = *post.mRenderOctTree;
    rstl::vector< uint > bitmap(tree.GetBitmapWordCount(), 0);
    tree.FindOverlappingModels(bitmap.data(), bounds);
    for (uint word = 0; word < tree.GetBitmapWordCount(); ++word) {
      if (bitmap[word] == 0) {
        continue;
      }
      for (int bit = 0; bit < 32; ++bit) {
        if ((bitmap[word] & (1u << bit)) == 0) {
          continue;
        }
        const int modelIndex = word * 32 + bit;
        const SAreaSurface& areaSurface = post.mSurfaces[modelIndex + 1];
        if (areaSurface.mModelIndex == -1 || areaSurface.mSurfaceGroupIndex == -1) {
          continue;
        }
        const CMetroidModelInstance& model = post.mModelInstances[areaSurface.mModelIndex];
        const ushort count = model.GetSurfaceCountInGroup(areaSurface.mSurfaceGroupIndex);
        const ushort* indices = model.GetSurfaceIndices(areaSurface.mSurfaceGroupIndex);
        for (ushort i = 0; i < count; ++i) {
          const CCubeSurface surface(model.GetSurfaces()[indices[i]]);
          const CAABox surfaceBounds = surface.GetBounds();
          if (!surfaceBounds.DoBoundsOverlap(bounds) ||
              CollisionUtil::RayAABoxIntersection(origin, direction, nearest, surfaceBounds) == 0) {
            continue;
          }
          const CCubeMaterial material = model.GetMaterialByIndex(surface.GetMaterialIndex());
          if (!worldFilter.Passes(CMaterialList(material.GetMaterialMask()))) {
            continue;
          }
          const float previousNearest = nearest;
          RaySurfaceIntersection(surface, material,
                                 static_cast< const CVector3f* >(model.GetVertexPointer()), line,
                                 result, nearest);
          if (modelOut != nullptr && previousNearest != nearest) {
            modelOut->first = area->GetId();
            modelOut->second = modelIndex;
          }
        }
      }
    }
  }
  return result;
}
