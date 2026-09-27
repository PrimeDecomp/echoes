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

class CRainSplashGenerator {
private:
  struct SSplashLine {
    float mTime;
    float mEndX;
    float mEndY;
    float mSpeed;
    float mParabolaHeight;
    uchar mLineWidth;
    uchar mLength;
    bool mActive : 1;

    SSplashLine()
    : mTime(0.f)
    , mEndX(0.f)
    , mEndY(0.f)
    , mSpeed(4.f)
    , mParabolaHeight(0.015625f)
    , mLineWidth(3)
    , mLength(1)
    , mActive(true) {}

    void Update(float dt, CStateManager& mgr);
    void Draw(float alpha, float dt, const CVector3f& position) const;
    void SetActive();
  };

  struct SRainSplash {
    rstl::reserved_vector< SSplashLine, 4 > mLines;
    CVector3f mPosition;
    float x70_;

    SRainSplash();
    void Update(float dt, CStateManager& mgr);
    bool IsActive() const;
    void Draw(float alpha, float dt, const CVector3f& position) const;
    void SetPoint(const CVector3f& position);
  };

public:
  CRainSplashGenerator(const CVector3f& scale, int maxSplashes, int generationRate, float minZ,
                       float alpha);
  ~CRainSplashGenerator() {}

  bool IsRaining() const { return mRaining; }

  void Update(float dt, CStateManager& mgr);
  void Draw(const CTransform4f& xf) const;
  void GeneratePoints(const CSkinnedModel& model, const SSkinningWorkspace& workspace);
  // Guessed name
  CVector3f GeneratePoint(const CSkinnedModel& model, const SSkinningWorkspace& workspace);

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
  bool mForceRaining : 1; // Guessed name

  void UpdateRainSplashRange(CStateManager& mgr, int start, int end, float dt);
  void UpdateRainSplashes(CStateManager& mgr, float magnitude, float dt);
  void AddPoint(const CVector3f& position);
  void DoDraw(const CTransform4f& xf) const;

  static int GetNextBestPt(int point, const CSkinnedModel& model,
                           const SSkinningWorkspace& workspace, int count, CRandom16& random,
                           float minZ);
};
CHECK_SIZEOF(CRainSplashGenerator, 0x4c)

#endif // _CRAINSPLASHGENERATOR
