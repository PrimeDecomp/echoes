#ifndef _CPERSISTENTOPTIONS
#define _CPERSISTENTOPTIONS

class CEnvironmentVariable;

class CPersistentOptions {
public:
  CEnvironmentVariable* FindEnvironmentVariable(const char* name); // Guessed name
  void SetSaveIdx(int idx) { mSaveIdx = idx; }                     // Guessed name

private:
  char x0_[0x28];
  int mSaveIdx;
};

#endif // _CPERSISTENTOPTIONS
