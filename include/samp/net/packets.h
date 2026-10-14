#pragma once
// Packet IDs. Observed: name table 0x10033650 (13 entries, IDs 200-212),
// strings ID_VEHICLE_SYNC @0x10119414 etc. NOTE: table is stringify-only
// (each case RETs) — NOT the dispatcher. Dispatcher is Net_DispatchIncoming.
#include <cstdint>

namespace samp { namespace net {

enum class SampPacketId : uint8_t {
  VehicleSync    = 200,
  RconCommand    = 201,
  RconResponse   = 202, // NOTE original typo in binary: "ID_RCON_RESPONCE"
  AimSync        = 203,
  WeaponsUpdate  = 204,
  StatsUpdate    = 205,
  // 206 = Unknown (default case)
  PlayerSync     = 207,
  // 208 = Unknown (default case)
  PassengerSync  = 209,
  SpectatorSync  = 210,
  TrailerSync    = 211,
  UnoccupiedSync = 212,
};

// Connection lifecycle IDs observed in FUN_1000af40 switch.
enum class ConnEvent : uint8_t {
  ConnectedChallenge = 0x0C,
  NoResponse   = 0x1D,
  ServerFull   = 0x1F,
  ServerClosed = 0x20,
  Lost         = 0x21,
  Accepted     = 0x22,
  EncryptFail  = 0x23,
  Banned       = 0x24,
  BadPassword  = 0x25,
};

const char* PacketIdToString(uint8_t id); // mirrors 0x10033650, default "Unknown"

}} // namespace
