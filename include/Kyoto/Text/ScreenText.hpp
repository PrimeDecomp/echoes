#ifndef _SCREENTEXT
#define _SCREENTEXT

#include "rstl/string.hpp"

class CRasterFont;
class CTextExecuteBuffer;
template < class T >
class TToken;

// Guessed namespace and names for the screen-space text utilities.
namespace ScreenText {
void DrawString(const rstl::string& text, int x, int y, const TToken< CRasterFont >& font);
void DrawExecuteBuffer(const CTextExecuteBuffer& buffer);
} // namespace ScreenText

#endif // _SCREENTEXT
