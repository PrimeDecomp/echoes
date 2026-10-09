#ifndef _ELISTENNOISETYPE
#define _ELISTENNOISETYPE

enum EListenNoiseType {
  kLNT_PlayerFire,
  kLNT_BombExplode,
  kLNT_ProjectileExplode,
  kLNT_Scream, // Guessed name
  kLNT_PathObstruction = 4, // Guessed name; path mesh or safe zone obstruction changed.
  kLNT_SafeZone = 6,        // Guessed name; a safe zone changed.
};

#endif
