// PacketIdToString. Observed: 0x10033650 jump-table, IDs 200-212,
// default "Unknown" @0x1011936c.
#include "samp/net/packets.h"

namespace samp { namespace net {

const char* PacketIdToString(uint8_t id) {
  switch ((int)id - 200) {
    case 0: return "ID_VEHICLE_SYNC";
    case 1: return "ID_RCON_COMMAND";
    case 2: return "ID_RCON_RESPONCE";
    case 3: return "ID_AIM_SYNC";
    case 4: return "ID_WEAPONS_UPDATE";
    case 5: return "ID_STATS_UPDATE";
    case 7: return "ID_PLAYER_SYNC";
    case 9: return "ID_PASSENGER_SYNC";
    case 10: return "ID_SPECTATOR_SYNC";
    case 11: return "ID_TRAILER_SYNC";
    case 12: return "ID_UNOCCUPIED_SYNC";
    default: return "Unknown";
  }
}

}} // namespace
