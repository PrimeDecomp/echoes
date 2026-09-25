#ifndef _CRAINSPLASHGENERATOR
#define _CRAINSPLASHGENERATOR
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"
class CSkinnedModel;
struct SSkinningWorkspace;
class CStateManager;
class CTransform4f;

// Nested records follow Prime.
class CRainSplashGenerator {
  struct SSplashLine {
    float mTime;
    float mEndX;
    float mEndY;
    float mSpeed;
    float mParabolaHeight;
    uchar mLineWidth;
    uchar mLength;
    bool mActive : 1;
  };
  struct SRainSplash {
    rstl::reserved_vector< SSplashLine, 4 > mLines;
    CVector3f mPosition;
    float x70_;
  };

public:
  CRainSplashGenerator(const CVector3f& scale, int maxSplashes, int generationRate, float minZ,
                       float alpha);
  ~CRainSplashGenerator();
  void Update(float dt, CStateManager& mgr);
  void GeneratePoints(const CSkinnedModel& model, const SSkinningWorkspace& workspace);

private:
  rstl::vector< SRainSplash > mRainSplashes;
  CRandom16 mRandom;
  CVector3f mScale;
  float mGenerateTimer;
  float mGenerateInterval;
  float mDt;
  float mMinZ;
  float mAlpha;
  int mCurrentPoint;
  int mQueueTail;
  int mQueueHead;
  int mQueueSize;
  int mGenerationRate;
  bool x48_24_ : 1;
  bool mRaining : 1;
  bool x48_26_ : 1;
};
CHECK_SIZEOF(CRainSplashGenerator, 0x4c)
#endif // _CRAINSPLASHGENERATOR
