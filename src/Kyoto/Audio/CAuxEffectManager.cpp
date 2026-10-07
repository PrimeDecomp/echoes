#include "Kyoto/Audio/CAuxEffectManager.hpp"

#include "Kyoto/Basics/CInterruptGuard.hpp"
#include "rstl/math.hpp"
#include <string.h>

static const int kStudios[3] = {1, 2, 0};
static const float kFadeStep = 0.000625f;
static const int kBufferSamples = 160;

void CAuxEffectManager::NoEffectCallback(uchar reason, SND_AUX_INFO* info, void* user) {}

CAuxEffectManager::SEffectSlot::SEffectSlot(float fade, EState state, int id,
                                            const CAuxEffect& effect)
: mFade(fade), mState(state), mId(id), mEffect(effect) {}

int CAuxEffectManager::SEffectSlot::GetPriority() const { return mEffect.GetPriority(); }

CAuxEffectManager::EState CAuxEffectManager::SEffectSlot::GetState() const { return mState; }

void CAuxEffectManager::SEffectSlot::SetState(EState state) { mState = state; }

float CAuxEffectManager::SEffectSlot::GetFade() const { return mFade; }

void CAuxEffectManager::SEffectSlot::SetFade(float fade) { mFade = fade; }

int CAuxEffectManager::SEffectSlot::GetId() const { return mId; }

void CAuxEffectManager::SEffectSlot::SetId(int id) { mId = id; }

void CAuxEffectManager::SEffectSlot::SetEffect(const CAuxEffect& effect) { mEffect = effect; }

void CAuxEffectManager::SEffectSlot::Prepare() { mEffect.Prepare(); }

void CAuxEffectManager::SEffectSlot::Shutdown() {
  mState = kES_Free;
  mEffect.Shutdown();
}

void CAuxEffectManager::SEffectSlot::Process(uchar reason, SND_AUX_INFO* info) {
  mEffect.Process(reason, info);
}

CAuxEffectManager::SCallbackContext::SCallbackContext(CAuxEffectManager* manager, int bus)
: mManager(manager), mBusIndex(bus) {}

CAuxEffectManager* CAuxEffectManager::SCallbackContext::GetManager() const { return mManager; }

int CAuxEffectManager::SCallbackContext::GetBusIndex() const { return mBusIndex; }

CAuxEffectManager::CAuxEffectManager() : mNextId(0) {
  for (int bus = 0; bus < 3; ++bus)
    mCallbackInstalled[bus] = false;
}

void CAuxEffectManager::Initialize() {
  mContexts.push_back(SCallbackContext(this, 0));
  mContexts.push_back(SCallbackContext(this, 1));
  mContexts.push_back(SCallbackContext(this, 2));
  mBuses.push_back(TBus());
  mBuses.push_back(TBus());
  mBuses.push_back(TBus());
  for (int bus = 0; bus < 3; ++bus)
    mCallbackInstalled[bus] = false;

  for (int bus = 0; bus < 3; ++bus) {
    CInterruptGuard interrupts;
    if (bus == 2)
      sndSetAuxProcessingCallbacks(kStudios[bus], NoEffectCallback, &mContexts[bus], SND_MIDI_NONE,
                                   0, nullptr, nullptr, SND_MIDI_NONE, 0);
  }
}

void CAuxEffectManager::Shutdown() {
  mContexts.clear();
  CInterruptGuard interrupts;
  for (uint bus = 0; bus < 3; ++bus)
    sndSetAuxProcessingCallbacks(kStudios[bus], nullptr, nullptr, SND_MIDI_NONE, 0, nullptr,
                                 nullptr, SND_MIDI_NONE, 0);
  for (int bus = 0; bus < mBuses.size(); ++bus) {
    for (SEffectSlot* slot = mBuses[bus].begin(); slot != mBuses[bus].end(); ++slot) {
      if (slot->GetState() != kES_Free)
        slot->Shutdown();
    }
  }
  mBuses.clear();
}

void CAuxEffectManager::Cleanup() {
  CInterruptGuard interrupts;
  for (int bus = 0; bus < 3; ++bus) {
    if (!mCallbackInstalled[bus])
      continue;
    int active = 0;
    for (SEffectSlot* effect = mBuses[bus].begin(); effect != mBuses[bus].end(); ++effect) {
      if (effect->GetState() == kES_PendingCleanup)
        effect->Shutdown();
      else if (effect->GetState() != kES_Free)
        ++active;
    }
    if (active == 0) {
      if (bus == 2) {
        CInterruptGuard callbackInterrupts;
        sndSetAuxProcessingCallbacks(kStudios[bus], NoEffectCallback, nullptr, SND_MIDI_NONE, 0,
                                     nullptr, nullptr, SND_MIDI_NONE, 0);
      } else {
        CInterruptGuard callbackInterrupts;
        sndSetAuxProcessingCallbacks(kStudios[bus], nullptr, nullptr, SND_MIDI_NONE, 0, nullptr,
                                     nullptr, SND_MIDI_NONE, 0);
      }
      mCallbackInstalled[bus] = false;
    }
  }
}

void CAuxEffectManager::FadeOut(int bus, ECategory category) {
  for (int slot = 0; slot < mBuses[bus].size(); ++slot) {
    SEffectSlot& effect = mBuses[bus][slot];
    switch (effect.GetState()) {
    case kES_Parallel:
    case kES_ParallelFadeIn:
      if (category == kEC_Parallel)
        effect.SetState(kES_ParallelFadeOut);
      break;
    case kES_Serial:
    case kES_SerialFadeIn:
    case kES_SerialBypassFadeOut:
      if (category == kEC_Serial) {
        effect.SetState(kES_SerialFadeOut);
        SetHighestPrioritySerialState(bus, kES_SerialFadeIn);
      }
      break;
    }
  }
}

void CAuxEffectManager::SetHighestPrioritySerialState(int bus, EState state) {
  int highestPriority = -1;
  int selected = -1;
  for (int slot = 0; slot < mBuses[bus].size(); ++slot) {
    const SEffectSlot& effect = mBuses[bus][slot];
    if (effect.GetState() == kES_Serial || effect.GetState() == kES_SerialFadeIn ||
        effect.GetState() == kES_SerialBypassFadeOut) {
      if (effect.GetPriority() > highestPriority) {
        highestPriority = effect.GetPriority();
        selected = slot;
      }
    }
  }
  if (selected != -1) {
    CInterruptGuard interrupts;
    mBuses[bus][selected].SetState(state);
  }
}

int CAuxEffectManager::AddEffect(int bus, const CAuxEffect& effect, ECategory category,
                                 bool replace) {
  TBus& effects = mBuses[bus];
  bool primary = true;
  if (replace) {
    if (category == kEC_Parallel) {
      FadeOut(bus, category);
    } else {
      for (int slot = 0; slot < effects.size(); ++slot) {
        // Native priority comparison examines free slots, not active ones.
        if (effects[slot].GetState() == kES_Free &&
            effect.GetPriority() < effects[slot].GetPriority())
          primary = false;
      }
      if (primary)
        SetHighestPrioritySerialState(bus, kES_SerialBypassFadeOut);
    }
  }
  Cleanup();

  int id = 0;
  bool assigned = false;
  for (int slot = 0; slot < effects.size(); ++slot) {
    if (effects[slot].GetState() != kES_Free)
      continue;
    {
      CInterruptGuard interrupts;
      id = (++mNextId << 4) | (bus << 2) | (slot + 1);
      effects[slot].SetId(id);
      effects[slot].SetFade(0.f);
      effects[slot].SetEffect(effect);
      effects[slot].SetState(category == kEC_Parallel ? kES_ParallelFadeIn
                             : primary                ? kES_SerialFadeIn
                                                      : kES_SerialBypassFadeOut);
      effects[slot].Prepare();
    }
    assigned = true;
    break;
  }
  if (!assigned) {
    CInterruptGuard interrupts;
    if (effects.size() == 4) {
      return 0;
    }
    // New slots use a zero-based index; reused slots use one-based indices.
    id = (++mNextId << 4) | (bus << 2) | effects.size();
    effects.push_back(SEffectSlot(
        0.f, category == kEC_Parallel ? kES_ParallelFadeIn : kES_SerialFadeIn, id, effect));
    effects.back().Prepare();
  }
  if (!mCallbackInstalled[bus]) {
    sndSetAuxProcessingCallbacks(kStudios[bus], AuxCallback, &mContexts[bus], SND_MIDI_NONE, 0,
                                 nullptr, nullptr, SND_MIDI_NONE, 0);
    mCallbackInstalled[bus] = true;
  }
  return id;
}

void CAuxEffectManager::RemoveEffect(int id) {
  CInterruptGuard interrupts;
  for (int bus = 0; bus < 3; ++bus) {
    for (int slot = 0; slot < mBuses[bus].size(); ++slot) {
      SEffectSlot& effect = mBuses[bus][slot];
      if (effect.GetId() != id)
        continue;
      switch (effect.GetState()) {
      case kES_Parallel:
      case kES_ParallelFadeIn:
        effect.SetState(kES_ParallelFadeOut);
        break;
      case kES_Serial:
      case kES_SerialFadeIn:
      case kES_SerialBypassFadeOut:
        effect.SetState(kES_SerialFadeOut);
        break;
      }
      return;
    }
  }
}

void CAuxEffectManager::AuxCallback(uchar reason, SND_AUX_INFO* info, void* user) {
  if (reason != SND_AUX_REASON_BUFFERUPDATE)
    return;
  const SCallbackContext& context = *static_cast< SCallbackContext* >(user);
  TBus& effects = context.GetManager()->mBuses[context.GetBusIndex()];
  int parallel = 0;
  int parallelFading = 0;
  int serial = 0;
  for (int slot = 0; slot < effects.size(); ++slot) {
    switch (effects[slot].GetState()) {
    case kES_Parallel:
      ++parallel;
      break;
    case kES_ParallelFadeIn:
    case kES_ParallelFadeOut:
      ++parallelFading;
      break;
    case kES_Serial:
    case kES_SerialFadeIn:
    case kES_SerialFadeOut:
    case kES_SerialBypassFadeOut:
      ++serial;
      break;
    }
  }
  if (parallel == 0 && parallelFading == 0 && serial == 0)
    return;

  s32* buffers[3] = {info->data.bufferUpdate.left, info->data.bufferUpdate.right,
                     info->data.bufferUpdate.surround};
  if (parallel == 1 && parallelFading == 0) {
    for (int slot = 0; slot < effects.size(); ++slot) {
      if (effects[slot].GetState() == kES_Parallel)
        effects[slot].Process(reason, info);
    }
  } else if (parallel != 0 || parallelFading != 0) {
    s32 mixed[3][kBufferSamples];
    memset(mixed, 0, sizeof(mixed));
    for (int slot = 0; slot < effects.size(); ++slot) {
      SEffectSlot& effect = effects[slot];
      const EState state = effect.GetState();
      if (state != kES_Parallel && state != kES_ParallelFadeIn && state != kES_ParallelFadeOut)
        continue;
      s32 scratch[3][kBufferSamples];
      for (int channel = 0; channel < 3; ++channel)
        for (int sample = 0; sample < kBufferSamples; ++sample)
          scratch[channel][sample] = buffers[channel][sample];
      SND_AUX_INFO processed;
      processed.data.bufferUpdate.left = scratch[0];
      processed.data.bufferUpdate.right = scratch[1];
      processed.data.bufferUpdate.surround = scratch[2];
      effect.Process(reason, &processed);

      float fade = effect.GetFade();
      int sample = 0;
      for (; sample < kBufferSamples; ++sample) {
        for (int channel = 0; channel < 3; ++channel) {
          mixed[channel][sample] +=
              state == kES_Parallel
                  ? scratch[channel][sample]
                  : static_cast< s32 >(static_cast< float >(scratch[channel][sample]) * fade);
        }
        if (state == kES_ParallelFadeIn) {
          fade += kFadeStep;
          if (fade >= 1.f) {
            effect.SetState(kES_Parallel);
            fade = 1.f;
            ++sample;
            for (; sample < kBufferSamples; ++sample)
              for (int channel = 0; channel < 3; ++channel)
                mixed[channel][sample] += scratch[channel][sample];
            break;
          }
        } else if (state == kES_ParallelFadeOut) {
          fade -= kFadeStep;
          if (fade <= 0.f) {
            effect.SetState(kES_PendingCleanup);
            fade = 0.f;
            break;
          }
        }
      }
      if (state != kES_Parallel)
        effect.SetFade(fade);
    }
    for (int channel = 0; channel < 3; ++channel)
      for (int sample = 0; sample < kBufferSamples; ++sample)
        buffers[channel][sample] = mixed[channel][sample];
  }

  if (serial == 0)
    return;
  for (int slot = 0; slot < effects.size(); ++slot) {
    SEffectSlot& effect = effects[slot];
    const EState state = effect.GetState();
    if (state == kES_Serial) {
      effect.Process(reason, info);
      continue;
    }
    if (state != kES_SerialFadeIn && state != kES_SerialFadeOut && state != kES_SerialBypassFadeOut)
      continue;
    float fade = effect.GetFade();
    if (state == kES_SerialBypassFadeOut && fade == 0.f)
      continue;
    s32 scratch[3][kBufferSamples];
    for (int channel = 0; channel < 3; ++channel)
      for (int sample = 0; sample < kBufferSamples; ++sample)
        scratch[channel][sample] = buffers[channel][sample];
    SND_AUX_INFO processed;
    processed.data.bufferUpdate.left = scratch[0];
    processed.data.bufferUpdate.right = scratch[1];
    processed.data.bufferUpdate.surround = scratch[2];
    effect.Process(reason, &processed);

    for (int sample = 0; sample < kBufferSamples; ++sample) {
      const float dry = 1.f - fade;
      for (int channel = 0; channel < 3; ++channel) {
        const float wet = static_cast< float >(scratch[channel][sample]) * fade;
        buffers[channel][sample] =
            static_cast< s32 >(dry * static_cast< float >(buffers[channel][sample]) + wet);
      }
      if (state == kES_SerialFadeIn) {
        fade = rstl::min_val(1.f, fade + kFadeStep);
        if (fade == 1.f) {
          effect.SetState(kES_Serial);
          // Native full-wet copy includes the current sample.
          for (; sample < kBufferSamples; ++sample)
            for (int channel = 0; channel < 3; ++channel)
              buffers[channel][sample] = scratch[channel][sample];
          break;
        }
      } else {
        fade = rstl::max_val(0.f, fade - kFadeStep);
        if (fade == 0.f) {
          if (state == kES_SerialFadeOut)
            effect.SetState(kES_PendingCleanup);
          break;
        }
      }
    }
    effect.SetFade(fade);
  }
}
