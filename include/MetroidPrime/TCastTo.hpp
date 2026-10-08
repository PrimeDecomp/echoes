#ifndef _TCASTTO
#define _TCASTTO

class CEntity;

CEntity* TryCast(CEntity* entity, int typeId);

template < class T >
T* TCastToPtr(CEntity* p);

template < class T >
T* TCastToPtr(CEntity& p);

template < typename T >
static inline const T* TCastToConstPtr(const CEntity* p) {
  return TCastToPtr< T >(const_cast< CEntity* >(p));
}

template < typename T >
static inline const T* TCastToConstPtr(const CEntity& p) {
  return TCastToPtr< T >(const_cast< CEntity& >(p));
}

#endif // _TCASTTO
