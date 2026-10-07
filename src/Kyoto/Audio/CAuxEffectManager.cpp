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
  for (SEffectSlot* effect = mBuses[bus].begin(); effect != mBuses[bus].end(); ++effect) {
    switch (effect->GetState()) {
    case kES_Parallel:
    case kES_ParallelFadeIn:
      if (category == kEC_Parallel)
        effect->SetState(kES_ParallelFadeOut);
      break;
    case kES_Serial:
    case kES_SerialFadeIn:
    case kES_SerialBypassFadeOut:
      if (category == kEC_Serial) {
        effect->SetState(kES_SerialFadeOut);
        SetHighestPrioritySerialState(bus, kES_SerialFadeIn);
      }
      break;
    }
  }
}

void CAuxEffectManager::SetHighestPrioritySerialState(int bus, EState state) {
  int highestPriority = -1;
  int selected = -1;
  int slot = 0;
  for (SEffectSlot* effect = mBuses[bus].begin(); effect != mBuses[bus].end(); ++effect, ++slot) {
    if (effect->GetState() == kES_SerialBypassFadeOut || effect->GetState() == kES_SerialFadeIn ||
        effect->GetState() == kES_Serial) {
      if (effect->GetPriority() > highestPriority) {
        selected = slot;
        highestPriority = effect->GetPriority();
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
      for (SEffectSlot* slot = effects.begin(); slot != effects.end(); ++slot) {
        // Native priority comparison examines free slots, not active ones.
        if (slot->GetState() == kES_Free && slot->GetPriority() > effect.GetPriority())
          primary = false;
      }
      if (primary)
        SetHighestPrioritySerialState(bus, kES_SerialBypassFadeOut);
    }
  }
  Cleanup();

  int id = 0;
  bool assigned = false;
  for (SEffectSlot* slot = effects.begin(); slot != effects.end(); ++slot) {
    ++id;
    if (slot->GetState() != kES_Free)
      continue;
    {
      CInterruptGuard interrupts;
      id = (++mNextId << 4) | ((bus << 2) | id);
      slot->SetId(id);
      slot->SetFade(0.f);
      slot->SetEffect(effect);
      slot->SetState(category == kEC_Parallel ? kES_ParallelFadeIn
                     : primary                ? kES_SerialFadeIn
                                              : kES_SerialBypassFadeOut);
      slot->Prepare();
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
    id = (++mNextId << 4) | ((bus << 2) | effects.size());
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
    for (SEffectSlot* effect = mBuses[bus].begin(); effect != mBuses[bus].end(); ++effect) {
      if (effect->GetId() != id)
        continue;
      switch (effect->GetState()) {
      case kES_Parallel:
      case kES_ParallelFadeIn:
        effect->SetState(kES_ParallelFadeOut);
        break;
      case kES_Serial:
      case kES_SerialFadeIn:
      case kES_SerialBypassFadeOut:
        effect->SetState(kES_SerialFadeOut);
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
  for (SEffectSlot* slot = effects.begin(); slot != effects.end(); ++slot) {
    switch (slot->GetState()) {
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

  s32 scratch[3][kBufferSamples];
  if (parallel == 1 && parallelFading == 0) {
    for (SEffectSlot* slot = effects.begin(); slot != effects.end(); ++slot) {
      if (slot->GetState() == kES_Parallel)
        slot->Process(reason, info);
    }
  } else if (parallel != 0 || parallelFading != 0) {
    s32 mixed[3][kBufferSamples];
    memset(mixed, 0, sizeof(mixed));
    for (SEffectSlot* slot = effects.begin(); slot != effects.end(); ++slot) {
      switch (slot->GetState()) {
      case kES_Parallel:
      case kES_ParallelFadeIn:
      case kES_ParallelFadeOut: {
        for (int sample = 0; sample < kBufferSamples; ++sample) {
          scratch[0][sample] = info->data.bufferUpdate.left[sample];
          scratch[1][sample] = info->data.bufferUpdate.right[sample];
          scratch[2][sample] = info->data.bufferUpdate.surround[sample];
        }
        SND_AUX_INFO processed;
        processed.data.bufferUpdate.left = scratch[0];
        processed.data.bufferUpdate.right = scratch[1];
        processed.data.bufferUpdate.surround = scratch[2];
        slot->Process(reason, &processed);

        switch (slot->GetState()) {
        case kES_Parallel:
          for (int sample = 0; sample < kBufferSamples; ++sample) {
            mixed[0][sample] += scratch[0][sample];
            mixed[1][sample] += scratch[1][sample];
            mixed[2][sample] += scratch[2][sample];
          }
          break;
        case kES_ParallelFadeIn: {
          float fade = slot->GetFade();
          int sample = 0;
          for (; sample < kBufferSamples; ++sample) {
            mixed[0][sample] += static_cast< s32 >(scratch[0][sample] * fade);
            mixed[1][sample] += static_cast< s32 >(scratch[1][sample] * fade);
            mixed[2][sample] += static_cast< s32 >(scratch[2][sample] * fade);
            fade += kFadeStep;
            if (fade >= 1.f) {
              slot->SetState(kES_Parallel);
              fade = 1.f;
              ++sample;
              for (; sample < kBufferSamples; ++sample)
                mixed[0][sample] += scratch[0][sample];
              mixed[1][sample] += scratch[1][sample];
              mixed[2][sample] += scratch[2][sample];
              break;
            }
          }
          slot->SetFade(fade);
          break;
        }
        case kES_ParallelFadeOut: {
          float fade = slot->GetFade();
          for (int sample = 0; sample < kBufferSamples; ++sample) {
            mixed[0][sample] += static_cast< s32 >(scratch[0][sample] * fade);
            mixed[1][sample] += static_cast< s32 >(scratch[1][sample] * fade);
            mixed[2][sample] += static_cast< s32 >(scratch[2][sample] * fade);
            fade -= kFadeStep;
            if (fade <= 0.f) {
              slot->SetState(kES_PendingCleanup);
              fade = 0.f;
              break;
            }
          }
          slot->SetFade(fade);
          break;
        }
        }
        break;
      }
      }
    }
    for (int sample = 0; sample < kBufferSamples; ++sample) {
      info->data.bufferUpdate.left[sample] = mixed[0][sample];
      info->data.bufferUpdate.right[sample] = mixed[1][sample];
      info->data.bufferUpdate.surround[sample] = mixed[2][sample];
    }
  }

  if (serial == 0)
    return;
  for (SEffectSlot* slot = effects.begin(); slot != effects.end(); ++slot) {
    SEffectSlot& effect = *slot;
    const EState state = effect.GetState();
    switch (state) {
    case kES_Serial:
      effect.Process(reason, info);
      continue;
    case kES_SerialBypassFadeOut:
      if (effect.GetFade() == 0.f)
        continue;
      // Fall through.
    case kES_SerialFadeIn:
    case kES_SerialFadeOut:
      break;
    default:
      continue;
    }
    s32 scratch[3][kBufferSamples];
    for (int sample = 0; sample < kBufferSamples; ++sample) {
      scratch[0][sample] = info->data.bufferUpdate.left[sample];
      scratch[1][sample] = info->data.bufferUpdate.right[sample];
      scratch[2][sample] = info->data.bufferUpdate.surround[sample];
    }
    SND_AUX_INFO processed;
    processed.data.bufferUpdate.left = scratch[0];
    processed.data.bufferUpdate.right = scratch[1];
    processed.data.bufferUpdate.surround = scratch[2];
    effect.Process(reason, &processed);

    float fade = effect.GetFade();
    for (int sample = 0; sample < kBufferSamples; ++sample) {
      const float dry = 1.f - fade;
      {
        const float wet = static_cast< float >(scratch[0][sample]) * fade;
        info->data.bufferUpdate.left[sample] = static_cast< s32 >(
            dry * static_cast< float >(info->data.bufferUpdate.left[sample]) + wet);
      }
      {
        const float wet = static_cast< float >(scratch[1][sample]) * fade;
        info->data.bufferUpdate.right[sample] = static_cast< s32 >(
            dry * static_cast< float >(info->data.bufferUpdate.right[sample]) + wet);
      }
      {
        const float wet = static_cast< float >(scratch[2][sample]) * fade;
        info->data.bufferUpdate.surround[sample] = static_cast< s32 >(
            dry * static_cast< float >(info->data.bufferUpdate.surround[sample]) + wet);
      }
      if (state == kES_SerialFadeIn) {
        fade = rstl::min_val(1.f, fade + kFadeStep);
        if (fade == 1.f) {
          effect.SetState(kES_Serial);
          // Native full-wet copy includes the current sample.
          for (; sample < kBufferSamples; ++sample) {
            info->data.bufferUpdate.left[sample] = scratch[0][sample];
            info->data.bufferUpdate.right[sample] = scratch[1][sample];
            info->data.bufferUpdate.surround[sample] = scratch[2][sample];
          }
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
