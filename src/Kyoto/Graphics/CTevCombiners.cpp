#include "Kyoto/Graphics/CTevCombiners.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGX.hpp"

#include <dolphin/gx/GXTev.h>

namespace {
const int kMaxPasses = 2;
}

int CTevCombiners::sNextUniquePass = 0;
const CTevCombiners::AlphaVar CTevCombiners::skAlphaOne(kAS_Konst);
const CTevCombiners::ColorVar CTevCombiners::skColorOne(kCS_One);
const CTevCombiners::CTevPass
    CTevCombiners::kEnvPassthru(ColorPass(ColorVar(kCS_Zero), ColorVar(kCS_Zero),
                                          ColorVar(kCS_Zero), ColorVar(kCS_RasterColor)),
                                AlphaPass(AlphaVar(kAS_Zero), AlphaVar(kAS_Zero),
                                          AlphaVar(kAS_Zero), AlphaVar(kAS_RasterAlpha)));

bool CTevCombiners::sValidPasses[kMaxPasses] = {false, false};
uint CTevCombiners::sNumEnabledPasses = -1;

CTevCombiners::AlphaVar::AlphaVar(EAlphaSrc src) : mSrc(src) {}

CTevCombiners::ColorVar::ColorVar(EColorSrc src) : mSrc(src) {}

void CTevCombiners::RecomputePasses() {
  uchar count = sValidPasses[kMaxPasses - 1] != false;
  sNumEnabledPasses = ++count;
  CGX::SetNumTevStages(count);
}

void CTevCombiners::Init() {
  for (int i = 0; i < kMaxPasses; ++i) {
    sValidPasses[i] = true;
  }
  sNumEnabledPasses = kMaxPasses;

  for (int i = 0; i < kMaxPasses; ++i) {
    DeletePass(i);
  }

  for (int i = 0; i < kMaxPasses; ++i) {
    sValidPasses[i] = false;
  }
  RecomputePasses();
}

void CTevCombiners::DeletePass(int stage) {
  SetPassCombiners(stage, kEnvPassthru);
  sValidPasses[stage] = false;
  RecomputePasses();
}

void CTevCombiners::SetupPass(int stage, const CTevPass& pass) {
  if (&pass == &kEnvPassthru) {
    DeletePass(stage);
    return;
  }

  if (SetPassCombiners(stage, pass)) {
    sValidPasses[stage] = true;
    RecomputePasses();
  }
}

bool CTevCombiners::SetPassCombiners(int stage, const CTevPass& pass) {
  pass.Execute(stage);
  return true;
}

void CTevCombiners::CTevPass::Execute(int stage) const {
  const GXTevStageID stageId = static_cast< GXTevStageID >(stage);
  CGX::SetTevColorIn(stageId, static_cast< GXTevColorArg >(mColorPass.GetA().GetSource()),
                     static_cast< GXTevColorArg >(mColorPass.GetB().GetSource()),
                     static_cast< GXTevColorArg >(mColorPass.GetC().GetSource()),
                     static_cast< GXTevColorArg >(mColorPass.GetD().GetSource()));
  CGX::SetTevAlphaIn(stageId, static_cast< GXTevAlphaArg >(mAlphaPass.GetA().GetSource()),
                     static_cast< GXTevAlphaArg >(mAlphaPass.GetB().GetSource()),
                     static_cast< GXTevAlphaArg >(mAlphaPass.GetC().GetSource()),
                     static_cast< GXTevAlphaArg >(mAlphaPass.GetD().GetSource()));
  CGX::SetTevColorOp(stageId, static_cast< GXTevOp >(mColorOp.GetOp()),
                     static_cast< GXTevBias >(mColorOp.GetBias()),
                     static_cast< GXTevScale >(mColorOp.GetScale()), mColorOp.GetClamp(),
                     static_cast< GXTevRegID >(mColorOp.GetOutput()));
  CGX::SetTevAlphaOp(stageId, static_cast< GXTevOp >(mAlphaOp.GetOp()),
                     static_cast< GXTevBias >(mAlphaOp.GetBias()),
                     static_cast< GXTevScale >(mAlphaOp.GetScale()), mAlphaOp.GetClamp(),
                     static_cast< GXTevRegID >(mAlphaOp.GetOutput()));
  CGX::SetTevKColorSel(stageId, GX_TEV_KCSEL_8_8);
  CGX::SetTevKAlphaSel(stageId, GX_TEV_KASEL_8_8);
}

// Guessed name.
void CTevCombiners::SetTevRegisterColor(int index, const CColor& color) {
  GXSetTevColor(static_cast< GXTevRegID >(index + 1), color.GetGXColor());
}

void CTevCombiners::ResetStates() {
  for (int i = 0; i < kMaxPasses; ++i) {
    sValidPasses[i] = false;
  }

  kEnvPassthru.Execute(0);
  sNumEnabledPasses = 1;
  CGX::SetNumTevStages(1);
}
