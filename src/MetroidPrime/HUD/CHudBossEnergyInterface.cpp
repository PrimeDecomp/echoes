#include "MetroidPrime/HUD/CHudBossEnergyInterface.hpp"

void CHudBossEnergyInterface::Update(float dt) {}

void CHudBossEnergyInterface::SetAlpha(float alpha) {}

void CHudBossEnergyInterface::SetBossParams(bool visible, const rstl::wstring& name, float energy,
                                            float maxEnergy) {}

CHudBossEnergyInterface::~CHudBossEnergyInterface() {}

CHudBossEnergyInterface::CHudBossEnergyInterface(CGuiFrame& frame, int hudState) {}

rstl::pair< CVector3f, CVector3f > CHudBossEnergyInterface::BallBossEnergyCoordFunc(float t) {}

rstl::pair< CVector3f, CVector3f > CHudBossEnergyInterface::BossEnergyCoordFunc(float t) {}
