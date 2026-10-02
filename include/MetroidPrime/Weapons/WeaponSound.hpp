#ifndef _WEAPONSOUND
#define _WEAPONSOUND

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CActor;
class CPlayer;

// Guessed helper names; these are native weapon-sound helpers, not CSfxManager overloads.
CSfxHandle AddEmitter(const CActor& actor, ushort sfx, bool useAcoustics, bool looped,
                      short priority, uchar maxVolume, uchar minVolume, float maxDistance,
                      float distanceCompensation);
CSfxHandle PlaySfxForPlayer(CPlayer* player, ushort sfx, short pan, int area, bool underwater,
                            bool looped);

#endif // _WEAPONSOUND
