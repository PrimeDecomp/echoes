#include "Kyoto/Text/CFont.hpp"

CFont::CFont(float scale) : mFontSize(scale * 16.f), mScale(scale) {}

CFont::~CFont() {}

int CFont::CharWidth(char) const { return mScale * 15.f; }

void CFont::DrawString(const char*, long, long, const CColor&) const {}
