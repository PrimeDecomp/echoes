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

// G2ME01 constructor 80182358, destructor 800cef84; nested records follow Prime.
class CRainSplashGenerator {
  struct SSplashLine {
    float x0_time;
    float x4_endX;
    float x8_endY;
    float xc_speed;
    float x10_parabolaHeight;
    uchar x14_lineWidth;
    uchar x15_length;
    bool x16_active : 1;
  };
  struct SRainSplash {
    rstl::reserved_vector< SSplashLine, 4 > x0_lines;
    CVector3f x64_position;
    float x70_;
  };

public:
  CRainSplashGenerator(const CVector3f& scale, int maxSplashes, int generationRate, float minZ,
                       float alpha);
  ~CRainSplashGenerator();
  void Update(float dt, CStateManager& mgr);
  void GeneratePoints(const CSkinnedModel& model, const SSkinningWorkspace& workspace);

private:
  rstl::vector< SRainSplash > x0_rainSplashes;
  CRandom16 x10_random;
  CVector3f x14_scale;
  float x20_generateTimer;
  float x24_generateInterval;
  float x28_dt;
  float x2c_minZ;
  float x30_alpha;
  int x34_currentPoint;
  int x38_queueTail;
  int x3c_queueHead;
  int x40_queueSize;
  int x44_generationRate;
  bool x48_24_ : 1;
  bool x48_25_raining : 1;
  bool x48_26_ : 1;
};
CHECK_SIZEOF(CRainSplashGenerator, 0x4c)
#endif // _CRAINSPLASHGENERATOR
