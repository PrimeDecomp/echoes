#include "MetroidPrime/Player/CStaticInterference.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "rstl/math.hpp"

CStaticInterference::CStaticInterference(int sourceCount) { sources.reserve(sourceCount); }

void CStaticInterference::AddSource(TUniqueId id, float magnitude, float duration) {
  float clampedMagnitude = CMath::Clamp(0.f, magnitude, 1.f);

  rstl::vector< CStaticInterferenceSource >::iterator search = sources.begin();
  for (; search != sources.end(); ++search) {
    if (search->GetSourceId() == id) {
      break;
    }
  }

  if (search != sources.end()) {
    search->SetIntensity(clampedMagnitude);
    search->SetTime(duration);
  } else {
    if (sources.size() < sources.capacity()) {
      sources.push_back_unsafe(CStaticInterferenceSource(id, clampedMagnitude, duration));
    }
  }
}

void CStaticInterference::RemoveSource(const TUniqueId id) {
  rstl::vector< CStaticInterferenceSource >::iterator search = sources.begin();
  for (; search != sources.end(); ++search) {
    if ((*search).GetSourceId() == id) {
      break;
    }
  }

  if (search != sources.end()) {
    sources.erase(search);
  }
}

float CStaticInterference::GetTotalInterference() const {
    const CStaticInterferenceSource* staticInterferenceSource;
    float f = 0.0f;
    CStaticInterferenceSource* items = sources.mItems;
    float a = f;
    staticInterferenceSource = &items[sources.mCount];
    while (items != staticInterferenceSource) {
        float intensity = items->GetIntensity();
        if (items->GetSourceId() == kInvalidUniqueId) {
            a += intensity;
        }
        if (((const __typeof__(*items)*)items)->GetSourceId() != kInvalidUniqueId) {
            f += intensity;
        }
        items++;
    }
    if (f > 0.8f) {
        f = 0.8f;
    }
    return rstl::min_val(f + a, 1.0f);
}

void CStaticInterference::Update(const CStateManager&, float dt) {
  rstl::vector< CStaticInterferenceSource >::iterator it = sources.begin();
  rstl::vector< CStaticInterferenceSource > toRemove;
  toRemove.reserve(sources.size());
  for (; it != sources.end(); ++it) {
    if (it->GetTime() < 0.f) {
      toRemove.push_back_unsafe(*it);
    } else {
      it->SetTime(it->GetTime() - dt);
    }
  }

  for (rstl::vector< CStaticInterferenceSource >::iterator it = toRemove.begin();
       it != toRemove.end(); ++it) {
    RemoveSource(it->GetSourceId());
  }
}
