#ifndef _TTOKEN
#define _TTOKEN

#include "types.h"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/IObj.hpp"

#include "rstl/auto_ptr.hpp"

template < typename T >
class TToken : public CToken {
public:
  TToken() {}
  TToken(const CToken& token) : CToken(token) {}
  TToken(T* obj) : CToken(TObjOwnerDerivedFromIObj< T >::GetNewDerivedObject(obj).release()) {}
  TToken(const rstl::auto_ptr< T >& obj) : CToken(GetIObjObjectFor(obj).release()) {}

  T* GetT() { return reinterpret_cast< T* >(CToken::GetObj()->GetContents()); }
  T* operator*() { return GetT(); }
  T* operator->() { return GetT(); }

  static rstl::auto_ptr< TObjOwnerDerivedFromIObj< T > >
  GetIObjObjectFor(const rstl::auto_ptr< T >& obj) {
    return TObjOwnerDerivedFromIObj< T >::GetNewDerivedObject(obj);
  }

  TToken< T > NonConstCopy() const { return *const_cast< TToken< T >* >(this); }
};

template < typename T >
class TCachedToken : public TToken< T > {
public:
  TCachedToken() {}
  TCachedToken(const CToken& token) : TToken< T >(token), mItem(nullptr) {}
  TCachedToken(const CToken& token, bool) : TToken< T >(token), mItem(TToken< T >::GetT()) {}

  T* operator*() { return mItem; }
  T* GetObject() const { return mItem; }
  const CToken& GetToken() const { return *this; }

  bool IsLoaded() {
    if (mItem != nullptr) {
      return true;
    }
    if (CToken::HasLock() && CToken::IsLoaded()) {
      mItem = TToken< T >::GetT();
      return true;
    } else {
      return false;
    }
  }

  void Unlock() {
    mItem = nullptr;
    TToken< T >::Unlock();
  }

  void ForceCache() {
    if (mItem == nullptr) {
      mItem = TToken< T >::GetT();
    }
  }

private:
  T* mItem;
};

template < typename T >
class TLockedToken {
public:
  TLockedToken() {}
  TLockedToken(const CToken& token) : mToken(token), mItem(*mToken) {}
  TLockedToken(const TLockedToken< T >& token) : mToken(token), mItem(*token) {
    mToken.Lock();
  }

  TLockedToken& operator=(const TLockedToken< T >& token) {
    mToken = token;
    mItem = *token;
    return *this;
  }

  operator const TToken< T >&() const { return mToken; }
  T* operator*() const { return mItem; }
  T* operator->() const { return mItem; }
  bool IsLoaded() const { return mToken.IsLoaded(); }

private:
  TToken< T > mToken;
  T* mItem;
};

#endif // _TTOKEN
