#ifndef _CENVIRONMENTVARIABLE
#define _CENVIRONMENTVARIABLE

class CEnvironmentVariable {
  public:
    void Set(int value);
    int Get() const { return mValue; }

  private:
    int mValue;
};

#endif // _CENVIRONMENTVARIABLE
