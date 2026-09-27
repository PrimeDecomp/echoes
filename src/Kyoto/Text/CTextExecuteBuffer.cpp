#include "Kyoto/Text/CTextExecuteBuffer.hpp"

#include "Kyoto/Text/CBlockInstruction.hpp"
#include "Kyoto/Text/CCharacterExtraSpaceInstruction.hpp"
#include "Kyoto/Text/CColorInstruction.hpp"
#include "Kyoto/Text/CColorOverrideInstruction.hpp"
#include "Kyoto/Text/CFontInstruction.hpp"
#include "Kyoto/Text/CImageInstruction.hpp"
#include "Kyoto/Text/CLineExtraSpaceInstruction.hpp"
#include "Kyoto/Text/CLineInstruction.hpp"
#include "Kyoto/Text/CLineSpacingInstruction.hpp"
#include "Kyoto/Text/CPopStateInstruction.hpp"
#include "Kyoto/Text/CPushStateInstruction.hpp"
#include "Kyoto/Text/CRemoveColorOverrideInstruction.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#include "Kyoto/Text/CWordInstruction.hpp"
#include "rstl/math.hpp"

CTextExecuteBuffer::CTextExecuteBuffer()
: mCurrentBlock(nullptr)
, mCurrentLine(nullptr)
, mCurrentWord(mInstructions.end())
, mCurrentWordX(0)
, mCurrentWordY(0)
, mSpaceDistance(0)
, mImageBaseline(false) {}

void CTextExecuteBuffer::Clear() {
  mInstructions.clear();
  mState = CSaveableState();
  mCurrentBlock = nullptr;
  mCurrentLine = nullptr;
  mCurrentWord = mInstructions.end();
  mCurrentWordX = 0;
  mCurrentWordY = 0;
  mSpaceDistance = 0;
}

void CTextExecuteBuffer::BeginBlock(int x, int y, int width, int height, bool imageBaseline,
                                    ETextDirection direction, EJustification justification,
                                    EVerticalJustification verticalJustification) {
  mImageBaseline = imageBaseline;
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CBlockInstruction(
      x, y, width, height, direction, justification, verticalJustification);
  mCurrentBlock = static_cast< CBlockInstruction* >(instruction.GetPtr());
  if (mState.IsFinishedLoading()) {
    mCurrentBlock->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                   mState.GetFont()->GetCarriageAdvance(),
                                   mState.GetFont()->GetBaseLine());
  }
  Add(instruction);
  mState.GetOptions().SetTextDirection(direction);
  mState.SetJustification(justification);
  mState.SetVerticalJustification(verticalJustification);
}

void CTextExecuteBuffer::EndBlock() {
  if (mCurrentLine) {
    TerminateLine(true);
  }
  mCurrentLine = nullptr;
  mCurrentBlock = nullptr;
}

void CTextExecuteBuffer::AddFont(const TToken< CRasterFont >& font) {
  Add(rs_new CFontInstruction(font));
  mState.SetFont(font);
  if (font.IsLoaded()) {
    if (mCurrentBlock) {
      mCurrentBlock->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                     mState.GetFont()->GetCarriageAdvance(),
                                     mState.GetFont()->GetBaseLine());
    }
    if (mCurrentLine) {
      mCurrentLine->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                    mState.GetFont()->GetCarriageAdvance(),
                                    mState.GetFont()->GetBaseLine());
    }
  }
}

void CTextExecuteBuffer::AddImage(const CFontImageDef& image) {
  // TODO: update line metrics and wrap loaded images before appending the instruction.
  Add(rs_new CImageInstruction(image));
}

void CTextExecuteBuffer::AddColor(EColorType type, const CTextColor& color) {
  Add(rs_new CColorInstruction(type, color));
}

void CTextExecuteBuffer::AddColorOverride(int index, const CTextColor& color) {
  Add(rs_new CColorOverrideInstruction(index, color));
}

void CTextExecuteBuffer::AddRemoveColorOverride(int index) {
  Add(rs_new CRemoveColorOverrideInstruction(index));
}

void CTextExecuteBuffer::AddLineSpacing(float spacing) {
  Add(rs_new CLineSpacingInstruction(spacing));
  mState.SetLineSpacing(spacing);
}

void CTextExecuteBuffer::AddLineExtraSpace(int spacing) {
  Add(rs_new CLineExtraSpaceInstruction(spacing));
  mState.SetLineExtraSpace(spacing);
}

void CTextExecuteBuffer::AddCharacterExtraSpace(int spacing) {
  Add(rs_new CCharacterExtraSpaceInstruction(spacing));
  mState.GetOptions().SetCharacterExtraSpace(spacing);
}

void CTextExecuteBuffer::AddJustification(EJustification justification) {
  mState.SetJustification(justification);
  if (mCurrentLine && mCurrentLine->GetWidth() == 0) {
    mCurrentLine->SetJustification(justification);
  }
}

void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification justification) {
  mState.SetVerticalJustification(justification);
  if (mCurrentLine && mCurrentLine->GetWidth() == 0) {
    mCurrentLine->SetVerticalJustification(justification);
  }
}

void CTextExecuteBuffer::AddPushState() {
  Add(rs_new CPushStateInstruction());
  mStateStack.push_front(mState);
}

void CTextExecuteBuffer::AddPopState() {
  Add(rs_new CPopStateInstruction());
  mState = mStateStack.front();
  mStateStack.pop_front();
  if (mCurrentLine->GetWidth() == 0) {
    mCurrentLine->SetJustification(mState.GetJustification());
    mCurrentLine->SetVerticalJustification(mState.GetVerticalJustification());
  }
}

void CTextExecuteBuffer::TerminateLineLTR(bool lastLine) {
  if (mCurrentLine->GetY() == 0 && mState.IsFinishedLoading()) {
    mCurrentLine->SetHeight(
        rstl::max_val(mState.GetFont()->GetCarriageAdvance(), mCurrentLine->GetHeight()));
  }
  mCurrentBlock->AddHeight(
      mCurrentBlock->GetVerticalJustification() == kVerticalJustification_Full || lastLine
          ? mCurrentLine->GetY()
          : mState.GetLineExtraSpacing() +
                static_cast< int >(mCurrentLine->GetY() * mState.GetLineSpacing()));
}

void CTextExecuteBuffer::TerminateLine(bool lastLine) {
  if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
    TerminateLineLTR(lastLine);
  }
}

void CTextExecuteBuffer::StartNewWord() {
  mCurrentWord = Add(rs_new CWordInstruction());
  mCurrentX = 0;
  mCurrentY = 0;
  mCurrentWordX = mCurrentLine->GetWidth();
  mCurrentWordY = mCurrentLine->GetY();
  mCurrentLine->IncWords();
}

void CTextExecuteBuffer::StartNewLine() {
  if (mCurrentLine) {
    TerminateLine(false);
  }
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineInstruction(
      0, 0, 0, mState.GetJustification(), mState.GetVerticalJustification(), mImageBaseline);
  mCurrentWord = Add(instruction);
  mCurrentLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  mSpaceDistance = 0;
  StartNewWord();
  mCurrentBlock->IncLines();
}

void CTextExecuteBuffer::MoveWordLTR() {
  mCurrentLine->SubWidth(mCurrentX + mSpaceDistance);
  if (mCurrentLine->GetY() > mCurrentWordY) {
    mCurrentLine->SetHeight(mCurrentWordY);
  }
  mSpaceDistance = 0;
  mCurrentLine->DecWords();
  TerminateLineLTR(false);

  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new CLineInstruction(1, mCurrentX, mCurrentY, mState.GetJustification(),
                              mState.GetVerticalJustification(), mImageBaseline);
  mCurrentLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  mInstructions.insert(mCurrentWord, instruction);
  mInstructions.insert(mCurrentWord, rs_new CWordInstruction());
  mCurrentBlock->IncLines();
}

int CTextExecuteBuffer::WrapOneLTR(const wchar_t* str, int len) {
  // TODO: measure glyphs, select word-break ranks and append text fragments.
  return len;
}

void CTextExecuteBuffer::AddStringFragment(const wchar_t* str, int len) {
  if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
    int consumed = 0;
    while (consumed != len) {
      consumed += WrapOneLTR(str + consumed, len - consumed);
    }
  }
}

void CTextExecuteBuffer::AddString(const wchar_t* str, int len) {
  // TODO: split words/newlines and account for space metrics in each text direction.
}

rstl::vector< CToken > CTextExecuteBuffer::GetAssets() const {
  int count = 0;
  for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
    count += (*it)->GetAssetCount();
  }

  rstl::vector< CToken > assets;
  if (count > 0) {
    assets.reserve(count);
    for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
      (*it)->GetAssets(assets);
    }
  }
  return assets;
}

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBuffer() const {
  // TODO: invoke instructions in allocation-tally and buffer-fill passes.
  return CTextRenderBuffer(CTextRenderBuffer::kM_AllocTally);
}

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBufferPage(InstList::const_iterator start,
                                                            InstList::const_iterator pageStart,
                                                            InstList::const_iterator pageEnd) {
  // TODO: replay state before pageStart, then invoke the page's instructions in both passes.
  return CTextRenderBuffer(CTextRenderBuffer::kM_AllocTally);
}

rstl::list< CTextRenderBuffer >
CTextExecuteBuffer::BuildRenderBufferPages(const CVector2i& extent) const {
  // TODO: paginate at line instructions using the accumulated rendering-state height.
  return rstl::list< CTextRenderBuffer >();
}
