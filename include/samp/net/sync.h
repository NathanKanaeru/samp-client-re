#pragma once
// Sync struct codecs. Field order ported from samp-source sdk/shared.h
// ONFOOT/AIM/INCAR/PASSENGER/SPECTATOR_SYNC_DATA (verified by read).
// Encode/decode over our BitStream with RakNet-compatible primitive order
// (u16LE, float LE, packed bytes). InCar roll/direction use compressed-unit
// norm vectors (C_VECTOR1). Round-trip tested.
// Wire parity vs a live DL-R1 server is TODO_VERIFY (no live server here);
// struct shapes match the 0.3.7 reference both versions derive from.
#include <cstdint>
#include "samp/net/bitstream.h"

namespace samp { namespace net {

struct OnFootSync {
  uint16_t lr = 0, ud = 0, keys = 0;
  float px = 0, py = 0, pz = 0, rot = 0;
  uint8_t health = 100, armour = 0, weapon = 0, special = 0;
  float mx = 0, my = 0, mz = 0;       // move speed
  float sx = 0, sy = 0, sz = 0;       // surf offsets
  uint16_t surfVehicle = 0;
};

struct AimSync { // size 31
  uint8_t camMode = 0;
  float fx = 0, fy = 0, fz = 0;       // aimf1
  float ax = 0, ay = 0, az = 0;       // aim pos
  float aimZ = 0;
  uint8_t camExtZoom = 0;             // 6 bits
  uint8_t weaponState = 0;            // 2 bits
  uint8_t aspect = 0;
};

struct InCarSync {
  uint16_t vehicle = 0;
  uint16_t lr = 0, ud = 0, keys = 0;
  float roll[3] = {};                 // compressed unit norm vector
  float dir[3] = {};                  // compressed unit norm vector
  float px = 0, py = 0, pz = 0;
  float mx = 0, my = 0, mz = 0;
  float carHealth = 1000;
  uint8_t pHealth = 100, pArmour = 0, weapon = 0, siren = 0, gear = 0;
  uint8_t tires[4] = {};
  uint16_t trailer = 0;
  uint32_t hydraAngle = 0;
  float trainSpeed = 0;
  uint8_t info = 0;
};

struct PassengerSync {
  uint16_t vehicle = 0;
  uint8_t seats = 0;      // seatFlags:7
  uint8_t driveBy = 0;    // :1
  uint8_t weapon = 0, health = 100, armour = 0;
  uint16_t lr = 0, ud = 0, keys = 0;
  float px = 0, py = 0, pz = 0;
};

struct SpectatorSync {
  uint16_t lr = 0, ud = 0, keys = 0;
  float px = 0, py = 0, pz = 0;
};

struct TrailerSync {
  float roll[3] = {};                 // compressed unit norm vectors
  float dir[3] = {};
  float px = 0, py = 0, pz = 0;
  float mx = 0, my = 0, mz = 0;
};

void EncodeOnFoot(BitStream& bs, const OnFootSync& s);
bool DecodeOnFoot(BitStream& bs, OnFootSync& s);
void EncodeAim(BitStream& bs, const AimSync& s);
bool DecodeAim(BitStream& bs, AimSync& s);
void EncodeInCar(BitStream& bs, const InCarSync& s);
bool DecodeInCar(BitStream& bs, InCarSync& s);
void EncodePassenger(BitStream& bs, const PassengerSync& s);
bool DecodePassenger(BitStream& bs, PassengerSync& s);
void EncodeSpectator(BitStream& bs, const SpectatorSync& s);
bool DecodeSpectator(BitStream& bs, SpectatorSync& s);
void EncodeTrailer(BitStream& bs, const TrailerSync& s);
bool DecodeTrailer(BitStream& bs, TrailerSync& s);

}} // namespace
