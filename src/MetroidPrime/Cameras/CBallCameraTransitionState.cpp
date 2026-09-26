#include "MetroidPrime/Cameras/CBallCamera.hpp"

CBallCamera::SFromBallTransition::SFromBallTransition()
: mPlayerXf(CTransform4f::Identity())
, mLookPos(CVector3f::Zero())
, mSpline(false, 1.f, CMotionSpline::kST_BSpline) {}

CBallCamera::SToBallTransition::SToBallTransition()
: mPlayerXf(CTransform4f::Identity())
, mLookPos(CVector3f::Zero())
, mSpline(false, 1.f, CMotionSpline::kST_BSpline) {}
