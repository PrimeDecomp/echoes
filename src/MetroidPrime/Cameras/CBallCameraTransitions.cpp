#include "MetroidPrime/Cameras/CBallCamera.hpp"

bool CBallCamera::CheckFailsafeFromMorphBallState(CStateManager& mgr) {
  // TODO: test the from-ball spline with the camera manager's collision query.
  return false;
}

bool CBallCamera::TransitionFromMorphBallState(CStateManager& mgr) {
  // TODO: build the from-ball spline from player and first-person camera transforms.
  return false;
}

bool CBallCamera::UpdateTransitionFromBallCamera(CStateManager& mgr) {
  // TODO: interpolate the from-ball transform and recover blocked transition placement.
  return false;
}

bool CBallCamera::CheckFailsafeToMorphBallState(CStateManager& mgr) {
  // TODO: test the to-ball spline with the camera manager's collision query.
  return false;
}

bool CBallCamera::TransitionToMorphBallState(CStateManager& mgr) {
  // TODO: build the to-ball spline with collision-constrained control points.
  return false;
}

bool CBallCamera::UpdateTransitionToBallCamera(float dt, CStateManager& mgr) {
  // TODO: advance the morph transition using the owned spline and player movement.
  return false;
}

bool CBallCamera::UpdateTransitionToBallCamera(CStateManager& mgr) {
  // TODO: finish the morphed-player transition and recover collision-safe placement.
  return false;
}
