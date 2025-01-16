#ifndef _CGAMEAREA
#define _CGAMEAREA

class CScriptAreaProperties;
class CGameArea {
public:
  enum EOcclusionState { kOS_Occluded, kOS_Visible };

  struct CPostConstructed {
    char pad1[0x13c];
    EOcclusionState m_occlusionState; // 0x13c

    CPostConstructed();
    ~CPostConstructed();
  };

  int GetPhase() const { return m_phase; }

  void SetAreaAttributes(CScriptAreaProperties*);
  bool IsLoaded() const { return m_phase == 0x10; }

  bool TryTakingOutOfARAM();
  EOcclusionState GetOcclusionState() const { return m_postConstructed->m_occlusionState; }

private:
  char pad1[0xf4];
  int m_phase;                         // 0xf4;
  CGameArea* m_next;                   // 0xf8
  CGameArea* m_prev;                   // 0xfc
  int m_curChain;                      // 0x100  (shouled be EChain)
  CPostConstructed* m_postConstructed; // 0x104
  int m_flagsActive;                   // 0x108
};

#endif // _CGAMEAREA
