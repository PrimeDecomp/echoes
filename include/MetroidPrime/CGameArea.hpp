#ifndef _CGAMEAREA
#define _CGAMEAREA

class CScriptAreaProperties;
class CGameArea {
public:
  int GetPhase() const { return m_phase; }

  void SetAreaAttributes(CScriptAreaProperties*);
  bool IsLoaded() const; // { return xf0_24_postConstructed; }

private:
  char pad1[0xf4];
  int m_phase; // 0xf4;
};

#endif // _CGAMEAREA
