// Model request queue. See models.h (REA strings + FUN_1000BB10/100b5900).
#include "samp/game/models.h"

namespace samp { namespace game {

bool ModelQueue::Request(uint32_t id) {
  if (Find(id)) return false;
  if (count_ >= kMax) return false; // "Too many textures…" family
  slots_[count_].id = id;
  slots_[count_].dff = ModelReqState::Requested;
  slots_[count_].txd = ModelReqState::Requested;
  ++count_;
  return true;
}

bool ModelQueue::MarkDownloading(uint32_t id) {
  auto* r = const_cast<ModelRequest*>(Find(id));
  if (!r) return false;
  r->dff = r->txd = ModelReqState::Downloading;
  return true;
}

bool ModelQueue::MarkReady(uint32_t id) {
  auto* r = const_cast<ModelRequest*>(Find(id));
  if (!r) return false;
  r->dff = r->txd = ModelReqState::Ready;
  return true;
}

bool ModelQueue::MarkFailed(uint32_t id) {
  auto* r = const_cast<ModelRequest*>(Find(id));
  if (!r) return false; // "[warning] Could not validate…"
  r->dff = r->txd = ModelReqState::Failed;
  return true;
}

const ModelRequest* ModelQueue::Find(uint32_t id) const {
  for (int i = 0; i < count_; ++i)
    if (slots_[i].id == id) return &slots_[i];
  return nullptr;
}

}} // namespace
