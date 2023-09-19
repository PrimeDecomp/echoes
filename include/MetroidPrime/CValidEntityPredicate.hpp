#ifndef _CVALIDENTITYPREDICATE
#define _CVALIDENTITYPREDICATE

class CEntity;

class CValidEntityPredicate {
public:
  virtual ~CValidEntityPredicate();
  virtual bool IsValid(const CEntity&) const;
};

#endif // _CVALIDENTITYPREDICATE
