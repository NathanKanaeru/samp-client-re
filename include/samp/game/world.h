#pragma once
// Actor + object pools. Caps: actors 1000 (actorpool.h), objects 255.
// Reference RPCs: WorldAddActor 42 / WorldRemoveActor 43 / ActorDamage 44,
// CreateObject 137 + pos/rot/scale/move/stop, SetActorPos/Facing/Health.
// Pure lifecycle here; GTA spawn calls are callbacks. Tested.
#include <cstdint>
#include <string>
#include "samp/net/limits.h"
#include "samp/game/slots.h"

namespace samp { namespace game {

struct Actor {
  int model = 0;
  float x = 0, y = 0, z = 0, facing = 0, health = 100;
  bool invulnerable = false;
};

struct WorldObject {
  int model = 0;
  float x = 0, y = 0, z = 0, rx = 0, ry = 0, rz = 0, scale = 1.0f;
  bool moving = false;
  float tx = 0, ty = 0, tz = 0, speed = 0; // MoveObject target
};

struct MapIcon {
  uint8_t id = 0;
  float x = 0, y = 0, z = 0;
  uint8_t type = 0, color = 0;
};

using ActorPool = SlotPool<Actor, net::kMaxActors>;
using ObjectPool = SlotPool<WorldObject, net::kMaxObjects>;
using IconPool = SlotPool<MapIcon, 200>; // MAX_MAP_ICON (shared.h:26)

}} // namespace
