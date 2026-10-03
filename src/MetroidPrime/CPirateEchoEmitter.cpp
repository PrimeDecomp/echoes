#include "MetroidPrime/CPirateEchoEmitter.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "rstl/math.hpp"

CPirateEchoEmitter::CPirateEchoEmitter(const CActor* actor, const CVector3f& position,
                                       const SEchoParameters& parameters, CSegId head,
                                       CSegId rightWing, CSegId leftWing, CSegId gun, CSegId swoosh,
                                       CSegId rightAnkle, CSegId leftAnkle)
: CEchoEmitter(CAABox(position, position), parameters)
, mActor(actor)
, mHead(head)
, mRightWing(rightWing)
, mLeftWing(leftWing)
, mGun(gun)
, mSwoosh(swoosh)
, mRightAnkle(rightAnkle)
, mLeftAnkle(leftAnkle)
, mUseHeadOnly(leftWing != CSegId::Invalid() && rightWing != CSegId::Invalid()) {}

CVector3f CPirateEchoEmitter::GetHeadPosition(const CPoseAsTransforms_Linear& pose) const {
  if (mUseHeadOnly) {
    return pose.GetOffset(mHead);
  }
  return (pose.GetOffset(mHead) + pose.GetOffset(mRightWing) + pose.GetOffset(mLeftWing)) * (1.f / 3.f);
}

void CPirateEchoEmitter::Render(const CStateManager& mgr) const {
  if (mgr.GetRenderVisorMode() != CStateManager::kRVM_Echo) {
    return;
  }

  const CTransform4f transform =
      mActor->GetTransform() * CTransform4f::Scale(mActor->GetModelData()->GetScale());
  const CPoseAsTransforms_Linear& pose = mActor->GetAnimationData()->Pose();
  const CVector3f head = transform * GetHeadPosition(pose);
  const CVector3f swoosh = transform * pose.GetOffset(mSwoosh);
  const CVector3f gun = transform * pose.GetOffset(mGun);
  const CVector3f leftAnkle = transform * pose.GetOffset(mLeftAnkle);
  const CVector3f rightAnkle = transform * pose.GetOffset(mRightAnkle);

  rstl::reserved_vector< CVector3f, 5 > points;
  points.push_back(head);
  points.push_back(swoosh);
  points.push_back(leftAnkle);
  points.push_back(rightAnkle);
  points.push_back(gun);

  static const int skProjectionIndices[5] = {0, 8, 6, 4, 2};
  rstl::reserved_vector< CVector3f, 10 > contour;
  contour.resize(10);
  const SProjection projection =
      ProjectPoints(points.data(), points.size(), contour.data(), skProjectionIndices);

  EnsureMinimumWidth(contour[8], contour[2], 0.5f);
  EnsureMinimumWidth(contour[6], contour[4], 0.25f);
  const float upperHeight = rstl::max_val(contour[8].GetZ(), contour[2].GetZ());
  const float lowerHeight = rstl::min_val(contour[8].GetZ(), contour[2].GetZ());
  LimitHeight(contour[0], upperHeight, 0.25f);
  LimitHeight(contour[6], lowerHeight, -0.25f);
  LimitHeight(contour[4], lowerHeight, -0.25f);

  const CVector3f& center = projection.mProjectedCenter;
  contour[9] = InterpolateContourPoint(contour[8], contour[6], center, 0.5f, 0.25f, 2.f);
  contour[3] = InterpolateContourPoint(contour[2], contour[4], center, 0.5f, 0.25f, 2.f);
  contour[5] = InterpolateContourPoint(contour[4], contour[6], center, -0.1f, 0.25f, 1.f);
  contour[7] = InterpolateContourPoint(contour[0], contour[8], center, -0.25f, 0.25f, 1.f);
  contour[1] = InterpolateContourPoint(contour[0], contour[2], center, -0.25f, 0.25f, 1.f);
  DrawContour(contour.data(), contour.size(), 6, projection, mgr);
}

CPirateEchoEmitter::~CPirateEchoEmitter() {}
