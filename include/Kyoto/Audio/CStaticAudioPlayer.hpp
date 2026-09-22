#ifndef _CSTATICAUDIOPLAYER
#define _CSTATICAUDIOPLAYER

typedef void (*FAudioCallback)();

class CStaticAudioPlayer {
public:
  static void RunDMACallback(FAudioCallback callback);
  static void CancelDMACallback(FAudioCallback callback);
};

#endif // _CSTATICAUDIOPLAYER
