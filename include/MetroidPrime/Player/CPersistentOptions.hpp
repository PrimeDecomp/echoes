#ifndef _CPERSISTENTOPTIONS
#define _CPERSISTENTOPTIONS

class CEnvironmentVariable;

class CPersistentOptions {
public:
  CEnvironmentVariable* FindEnvironmentVariable(const char* name); // Guessed name

private:
  char pad[0x2c];
};

#endif // _CPERSISTENTOPTIONS
