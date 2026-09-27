#include "MetroidPrime/CIOWin.hpp"

CIOWin::CIOWin(const rstl::string& name) : mName(name) {}

CIOWin::~CIOWin() {}

const rstl::string& CIOWin::GetName() const { return mName; }

CIOWin::EMessageReturn CIOWin::OnMessage(const CArchitectureMessage&, CArchitectureQueue&) {
  return kMR_Normal;
}

bool CIOWin::GetIsContinueDraw() const { return true; }

void CIOWin::Draw() const {}

void CIOWin::PreDraw() const {}
