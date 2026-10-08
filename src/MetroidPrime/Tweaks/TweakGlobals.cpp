#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "MetroidPrime/Tweaks/CTweakParticle.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControls.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetroidPrime/Tweaks/CTweakSlideShow.hpp"
#include "MetroidPrime/Tweaks/CTweakTargeting.hpp"

rstl::single_ptr< CTweakAutoMapper > gpTweakAutoMapper;
rstl::single_ptr< CTweakBall > gpTweakBall;
rstl::single_ptr< CTweakGame > gpTweakGame;
rstl::single_ptr< CTweakGui > gpTweakGui;
rstl::single_ptr< CTweakGuiColors > gpTweakGuiColors;
rstl::single_ptr< CTweakParticle > gpTweakParticle;
rstl::single_ptr< CTweakPlayer > gpTweakPlayerB;
rstl::single_ptr< CTweakPlayer > gpTweakPlayerA;
rstl::single_ptr< CTweakPlayerControls > gpTweakPlayerControlsB;
rstl::single_ptr< CTweakPlayerControls > gpTweakPlayerControlsA;
rstl::single_ptr< CTweakPlayerGun > gpTweakPlayerGunMulti;
rstl::single_ptr< CTweakPlayerGun > gpTweakPlayerGunSingle;
rstl::single_ptr< CTweakPlayerRes > gpTweakPlayerRes;
rstl::single_ptr< CTweakSlideShow > gpTweakSlideShow;
rstl::single_ptr< CTweakTargeting > gpTweakTargeting;
CTweakPlayerGun* gpTweakPlayerGun;
