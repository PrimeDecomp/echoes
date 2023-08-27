#ifndef _TCASTTO
#define _TCASTTO

class CEntity;
class CPlayer;

template <typename T>
T* CastTo(CEntity*);

template <typename T>
const T* CastTo(const CEntity*);

#endif // _TCASTTO
