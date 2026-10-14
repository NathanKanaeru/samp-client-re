// Sync codecs. See sync.h for provenance.
#include "samp/net/sync.h"

namespace samp { namespace net {

void EncodeOnFoot(BitStream& bs, const OnFootSync& s) {
  bs.WriteU16(s.lr); bs.WriteU16(s.ud); bs.WriteU16(s.keys);
  bs.WriteBytes(&s.px, 4); bs.WriteBytes(&s.py, 4); bs.WriteBytes(&s.pz, 4);
  bs.WriteBytes(&s.rot, 4);
  bs.WriteU8(s.health); bs.WriteU8(s.armour); bs.WriteU8(s.weapon); bs.WriteU8(s.special);
  bs.WriteBytes(&s.mx, 4); bs.WriteBytes(&s.my, 4); bs.WriteBytes(&s.mz, 4);
  bs.WriteBytes(&s.sx, 4); bs.WriteBytes(&s.sy, 4); bs.WriteBytes(&s.sz, 4);
  bs.WriteU16(s.surfVehicle);
}

bool DecodeOnFoot(BitStream& bs, OnFootSync& s) {
  return bs.ReadU16(s.lr) && bs.ReadU16(s.ud) && bs.ReadU16(s.keys) &&
         bs.ReadBytes(&s.px, 4) && bs.ReadBytes(&s.py, 4) && bs.ReadBytes(&s.pz, 4) &&
         bs.ReadBytes(&s.rot, 4) &&
         bs.ReadU8(s.health) && bs.ReadU8(s.armour) && bs.ReadU8(s.weapon) && bs.ReadU8(s.special) &&
         bs.ReadBytes(&s.mx, 4) && bs.ReadBytes(&s.my, 4) && bs.ReadBytes(&s.mz, 4) &&
         bs.ReadBytes(&s.sx, 4) && bs.ReadBytes(&s.sy, 4) && bs.ReadBytes(&s.sz, 4) &&
         bs.ReadU16(s.surfVehicle);
}

void EncodeAim(BitStream& bs, const AimSync& s) {
  bs.WriteU8(s.camMode);
  bs.WriteBytes(&s.fx, 4); bs.WriteBytes(&s.fy, 4); bs.WriteBytes(&s.fz, 4);
  bs.WriteBytes(&s.ax, 4); bs.WriteBytes(&s.ay, 4); bs.WriteBytes(&s.az, 4);
  bs.WriteBytes(&s.aimZ, 4);
  bs.WriteU8((uint8_t)(((s.weaponState & 3u) << 6) | (s.camExtZoom & 63u)));
  bs.WriteU8(s.aspect);
}

bool DecodeAim(BitStream& bs, AimSync& s) {
  uint8_t packed = 0;
  if (!(bs.ReadU8(s.camMode) &&
        bs.ReadBytes(&s.fx, 4) && bs.ReadBytes(&s.fy, 4) && bs.ReadBytes(&s.fz, 4) &&
        bs.ReadBytes(&s.ax, 4) && bs.ReadBytes(&s.ay, 4) && bs.ReadBytes(&s.az, 4) &&
        bs.ReadBytes(&s.aimZ, 4) && bs.ReadU8(packed) && bs.ReadU8(s.aspect)))
    return false;
  s.camExtZoom = packed & 63u;
  s.weaponState = (packed >> 6) & 3u;
  return true;
}

void EncodeInCar(BitStream& bs, const InCarSync& s) {
  bs.WriteU16(s.vehicle);
  bs.WriteU16(s.lr); bs.WriteU16(s.ud); bs.WriteU16(s.keys);
  bs.WriteCompressedUnit(s.roll[0]); bs.WriteCompressedUnit(s.roll[1]); bs.WriteCompressedUnit(s.roll[2]);
  bs.WriteCompressedUnit(s.dir[0]); bs.WriteCompressedUnit(s.dir[1]); bs.WriteCompressedUnit(s.dir[2]);
  bs.WriteBytes(&s.px, 4); bs.WriteBytes(&s.py, 4); bs.WriteBytes(&s.pz, 4);
  bs.WriteBytes(&s.mx, 4); bs.WriteBytes(&s.my, 4); bs.WriteBytes(&s.mz, 4);
  bs.WriteBytes(&s.carHealth, 4);
  bs.WriteU8(s.pHealth); bs.WriteU8(s.pArmour); bs.WriteU8(s.weapon);
  bs.WriteU8(s.siren); bs.WriteU8(s.gear);
  bs.WriteBytes(s.tires, 4);
  bs.WriteU16(s.trailer);
  bs.WriteU32(s.hydraAngle);
  bs.WriteBytes(&s.trainSpeed, 4);
  bs.WriteU8(s.info);
}

bool DecodeInCar(BitStream& bs, InCarSync& s) {
  return bs.ReadU16(s.vehicle) &&
         bs.ReadU16(s.lr) && bs.ReadU16(s.ud) && bs.ReadU16(s.keys) &&
         bs.ReadCompressedUnit(s.roll[0]) && bs.ReadCompressedUnit(s.roll[1]) && bs.ReadCompressedUnit(s.roll[2]) &&
         bs.ReadCompressedUnit(s.dir[0]) && bs.ReadCompressedUnit(s.dir[1]) && bs.ReadCompressedUnit(s.dir[2]) &&
         bs.ReadBytes(&s.px, 4) && bs.ReadBytes(&s.py, 4) && bs.ReadBytes(&s.pz, 4) &&
         bs.ReadBytes(&s.mx, 4) && bs.ReadBytes(&s.my, 4) && bs.ReadBytes(&s.mz, 4) &&
         bs.ReadBytes(&s.carHealth, 4) &&
         bs.ReadU8(s.pHealth) && bs.ReadU8(s.pArmour) && bs.ReadU8(s.weapon) &&
         bs.ReadU8(s.siren) && bs.ReadU8(s.gear) &&
         bs.ReadBytes(s.tires, 4) && bs.ReadU16(s.trailer) &&
         bs.ReadU32(s.hydraAngle) && bs.ReadBytes(&s.trainSpeed, 4) && bs.ReadU8(s.info);
}

void EncodePassenger(BitStream& bs, const PassengerSync& s) {
  bs.WriteU16(s.vehicle);
  bs.WriteU8((uint8_t)((s.driveBy & 1u) << 7 | (s.seats & 127u)));
  bs.WriteU8(s.weapon); bs.WriteU8(s.health); bs.WriteU8(s.armour);
  bs.WriteU16(s.lr); bs.WriteU16(s.ud); bs.WriteU16(s.keys);
  bs.WriteBytes(&s.px, 4); bs.WriteBytes(&s.py, 4); bs.WriteBytes(&s.pz, 4);
}

bool DecodePassenger(BitStream& bs, PassengerSync& s) {
  uint8_t packed = 0;
  if (!(bs.ReadU16(s.vehicle) && bs.ReadU8(packed) &&
        bs.ReadU8(s.weapon) && bs.ReadU8(s.health) && bs.ReadU8(s.armour) &&
        bs.ReadU16(s.lr) && bs.ReadU16(s.ud) && bs.ReadU16(s.keys) &&
        bs.ReadBytes(&s.px, 4) && bs.ReadBytes(&s.py, 4) && bs.ReadBytes(&s.pz, 4)))
    return false;
  s.seats = packed & 127u;
  s.driveBy = (packed >> 7) & 1u;
  return true;
}

void EncodeSpectator(BitStream& bs, const SpectatorSync& s) {
  bs.WriteU16(s.lr); bs.WriteU16(s.ud); bs.WriteU16(s.keys);
  bs.WriteBytes(&s.px, 4); bs.WriteBytes(&s.py, 4); bs.WriteBytes(&s.pz, 4);
}

bool DecodeSpectator(BitStream& bs, SpectatorSync& s) {
  return bs.ReadU16(s.lr) && bs.ReadU16(s.ud) && bs.ReadU16(s.keys) &&
         bs.ReadBytes(&s.px, 4) && bs.ReadBytes(&s.py, 4) && bs.ReadBytes(&s.pz, 4);
}

void EncodeTrailer(BitStream& bs, const TrailerSync& s) {
  bs.WriteCompressedUnit(s.roll[0]); bs.WriteCompressedUnit(s.roll[1]); bs.WriteCompressedUnit(s.roll[2]);
  bs.WriteCompressedUnit(s.dir[0]); bs.WriteCompressedUnit(s.dir[1]); bs.WriteCompressedUnit(s.dir[2]);
  bs.WriteBytes(&s.px, 4); bs.WriteBytes(&s.py, 4); bs.WriteBytes(&s.pz, 4);
  bs.WriteBytes(&s.mx, 4); bs.WriteBytes(&s.my, 4); bs.WriteBytes(&s.mz, 4);
}

bool DecodeTrailer(BitStream& bs, TrailerSync& s) {
  return bs.ReadCompressedUnit(s.roll[0]) && bs.ReadCompressedUnit(s.roll[1]) && bs.ReadCompressedUnit(s.roll[2]) &&
         bs.ReadCompressedUnit(s.dir[0]) && bs.ReadCompressedUnit(s.dir[1]) && bs.ReadCompressedUnit(s.dir[2]) &&
         bs.ReadBytes(&s.px, 4) && bs.ReadBytes(&s.py, 4) && bs.ReadBytes(&s.pz, 4) &&
         bs.ReadBytes(&s.mx, 4) && bs.ReadBytes(&s.my, 4) && bs.ReadBytes(&s.mz, 4);
}

}} // namespace
