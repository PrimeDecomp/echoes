#include "MetroidPrime/CStateManager.hpp"

// Reconstructed operation names; original spellings are unknown.
void CStateManager::ScriptMsgArray::Append(const CScriptMsg& msg) {
  mMessages[mWriteIndex] = msg;
  mWriteIndex = (mWriteIndex + 1) % kCapacity;
}

CScriptMsg CStateManager::ScriptMsgArray::Dequeue() {
  const uint readIndex = mReadIndex;
  mReadIndex = (mReadIndex + 1) % kCapacity;
  return mMessages[readIndex];
}

int CStateManager::ScriptMsgArray::GetCount() const {
  if (mWriteIndex >= mReadIndex) {
    return mWriteIndex - mReadIndex;
  }
  return kCapacity - mReadIndex + mWriteIndex;
}
