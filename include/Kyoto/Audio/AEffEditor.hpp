#ifndef _AEFFEDITOR
#define _AEFFEDITOR

#include "Kyoto/Audio/AEffect.hpp"

// SDK-correlated old editor interface. No concrete editor implementation is recovered here.
class AEffEditor {
public:
  virtual ~AEffEditor();
  virtual long getRect(ERect** rect);
  virtual long open(void* window);
  virtual void close();
  virtual void idle();
  virtual void update();
  virtual void postUpdate();
};

#endif // _AEFFEDITOR
