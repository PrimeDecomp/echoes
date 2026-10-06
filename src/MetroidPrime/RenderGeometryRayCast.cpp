#include "MetroidPrime/RenderGeometryRayCast.hpp"

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

// Display-list shorts are big-endian, like the indices CDisplayListReader reads.
inline ushort ReadShort(const uchar* data) {
  uchar bytes[2];
  bytes[0] = data[0];
  bytes[1] = data[1];
  return *reinterpret_cast< const ushort* >(bytes);
}

inline const CVector3f& GetPosition(const CVector3f* positions, const uchar* vertex) {
  return positions[static_cast< short >(ReadShort(vertex))];
}

inline bool RayTriangleIntersection(const CLine& line, const CVector3f& a, const CVector3f& b,
                                    const CVector3f& c, float& nearest) {
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
  if (u < 0.f || u > 1.f) {
    return false;
  }

  const CVector3f cross1 = CVector3f::Cross(displacement, edge1);
  const float distance = inverse * CVector3f::Dot(cross1, edge2);
  if (distance < 0.f || distance >= nearest) {
    return false;
  }

  const float v = inverse * CVector3f::Dot(cross1, line.GetNormal());
  if (v < 0.f || u + v > 1.f) {
    return false;
  }

  nearest = distance;
  return true;
}

inline void IntersectTriangle(const CVector3f* positions, const uchar* va, const uchar* vb,
                              const uchar* vc, const CLine& line, const CCubeMaterial& material,
                              CRayCastResult& result, float& nearest) {
  const CVector3f& a = GetPosition(positions, va);
  const CVector3f& b = GetPosition(positions, vb);
  const CVector3f& c = GetPosition(positions, vc);
  if (RayTriangleIntersection(line, a, b, c, nearest)) {
    CMaterialList hitMaterial(kMT_Unknown59, kMT_Unknown60);
    hitMaterial.Add(CMaterialList(material.GetMaterialMask()));
    result = CRayCastResult(nearest, line.GetRefPoint() + nearest * line.GetNormal(),
                            CPlane(a, b, c), hitMaterial);
  }
}
} // namespace

int RenderGeometryRayCast::RaySurfaceIntersection(const CCubeSurface& surface,
                                                  const CCubeMaterial& material,
                                                  const CVector3f* positions, const CLine& line,
                                                  CRayCastResult& result, float& nearest) {
  const uint descriptor = material.GetVertexDesc();
  const int size = surface.GetDisplayListSize();
  int stride = 0;
  for (int i = 0; i < 16; ++i) {
    switch (static_cast< GXAttrType >((descriptor >> (i * 2)) & 3)) {
    case GX_NONE:
    case GX_DIRECT:
      break;
    case GX_INDEX8:
      ++stride;
      break;
    case GX_INDEX16:
      stride += 2;
      break;
    }
  }

  const uchar* displayList = static_cast< const uchar* >(surface.GetDisplayList());
  int triangles = 0;
  int offset = 0;
  while (offset < size) {
    const int primitive = displayList[offset++] & 0xfc;
    if (primitive == GX_NOP) {
      continue;
    }
    const short count = ReadShort(displayList + offset);
    offset += 2;

    switch (primitive) {
    case GX_TRIANGLES: {
      triangles += count / 3;
      int vertexOffset = offset;
      for (int i = 0; i < count; i += 3, vertexOffset += stride * 3) {
        const uchar* vertex = displayList + vertexOffset;
        IntersectTriangle(positions, vertex, vertex + stride, vertex + stride * 2, line, material,
                          result, nearest);
      }
      break;
    }
    case GX_TRIANGLEFAN: {
      triangles += count - 2;
      const uchar* first = displayList + offset;
      const uchar* previous = first + stride;
      const uchar* current = previous + stride;
      for (int i = 2; i < count; ++i) {
        IntersectTriangle(positions, first, previous, current, line, material, result, nearest);
        previous = current;
        current += stride;
      }
      break;
    }
    case GX_TRIANGLESTRIP: {
      triangles += count - 2;
      const uchar* vertices = displayList + offset;
      for (int i = 2; i < count; ++i) {
        if (i & 1) {
          IntersectTriangle(positions, vertices + stride * (i - 1), vertices + stride * (i - 2),
                            vertices + stride * i, line, material, result, nearest);
        } else {
          IntersectTriangle(positions, vertices + stride * (i - 2), vertices + stride * (i - 1),
                            vertices + stride * i, line, material, result, nearest);
        }
      }
      break;
    }
    }
    offset += count * stride;
  }
  return triangles;
}

CRayCastResult RenderGeometryRayCast::RayWorldIntersection(const CStateManager& mgr,
                                                           const CVector3f& origin,
                                                           const CVector3f& direction, float length,
                                                           const CMaterialFilter& filter,
                                                           rstl::pair< TAreaId, int >* modelOut) {
  CRayCastResult result;
  float nearest = length > 0.f ? length : 100000.f;
  const CMaterialFilter worldFilter = filter.WithImplicitMaterials(skImplicitWorldMaterials);
  if (worldFilter.GetType() == CMaterialFilter::kFT_Never) {
    return result;
  }

  const CLine line(origin, CUnitVector3f(direction, CUnitVector3f::kN_No));
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  bounds.AccumulateBounds(origin);
  bounds.AccumulateBounds(origin + nearest * direction);
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
        const CMetroidModelInstance::CSurfaceGroups groups = model.GetSurfaceGroups();
        const ushort count = groups.GetSurfaceCount(areaSurface.mSurfaceGroupIndex);
        const ushort* indices = groups.GetSurfaceIndices(areaSurface.mSurfaceGroupIndex);
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
