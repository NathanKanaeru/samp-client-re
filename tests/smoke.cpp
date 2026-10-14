// Smoke test: exercises BitStream, packets, pools, config, crash format.
// No GTA / network / D3D needed.
#include <cstdio>
#include <cassert>
#include <cstring>
#include "samp/common.h"
#include "samp/net/bitstream.h"
#include "samp/net/packets.h"
#include "samp/net/rpc.h"
#include "samp/net/rpc_ids.h"
#include "samp/net/limits.h"
#include "samp/game/gta_addrs.h"
#include "samp/game/pools.h"
#include "samp/game/player.h"
#include "samp/game/vehicle.h"
#include "samp/gui/fonts.h"
#include "samp/util/config.h"
#include "samp/util/string_utils.h"
#include "samp/util/crash.h"

int main() {
  using namespace samp;
  // version
  assert(strcmp(net::PacketIdToString(200), "ID_VEHICLE_SYNC") == 0);
  assert(strcmp(net::PacketIdToString(206), "Unknown") == 0);
  assert(strcmp(net::PacketIdToString(207), "ID_PLAYER_SYNC") == 0);
  assert(strcmp(net::PacketIdToString(212), "ID_UNOCCUPIED_SYNC") == 0);

  // bitstream round-trip
  net::BitStream bs;
  bs.WriteU32(0xFDE);
  bs.WriteU16(7777);
  bs.WriteBytes("AB", 2);
  net::BitStream rd;
  rd.SetData(bs.Data(), bs.BitsUsed() / 8, true);
  uint32_t m = 0; uint16_t p = 0; char s[2] = {};
  assert(rd.ReadU32(m) && m == 0xFDE);
  assert(rd.ReadU16(p) && p == 7777);
  assert(rd.ReadBytes(s, 2) && s[0] == 'A' && s[1] == 'B');

  // api registry
  net::RegisterRakPeerApi();
  assert(!net::ApiRegistry().empty());

  // reference limits (sdk/shared.h) + RPC IDs (SAMPRPC.h) + x86 map (address.h)
  static_assert(net::kMaxPlayers == 200, "net players");
  static_assert(net::kMaxVehicles == 700, "net vehicles");
  static_assert(net::kMaxObjects == 255, "net objects");
  static_assert(net::kMaxActors == 1000, "net actors");
  static_assert(net::kMaxTextDrawSlots == 2304, "textdraw slots");
  static_assert(net::kNetGameVersion == 4242, "netgame version");
  static_assert((int)net::GameRpc::ClientJoin == 25, "ClientJoin");
  static_assert((int)net::ScriptRpc::AudioStream == 182, "AudioStream");
  static_assert((int)net::ConnPacket::Timestamp == 133, "timestamp pkt");
  static_assert(gta::kEntryState == 0x00C8D4C0u, "entry state");
  static_assert(gta::kD3DDevicePtr == 0x00C97C28u, "device ptr");
  static_assert(gta::kRwGetD3D_US == 0x007F9D50u, "rw us");
  static_assert(game::kOverlayMaxVehicle == 2000, "overlay vehicles");

  // pools
  assert(game::Vehicles().Create(0, 411, 0, 0, 3, 0));
  assert(game::Vehicles().Get(0) != nullptr);
  game::DeleteVehicle(0);
  int pid = game::Players().Alloc();
  assert(pid >= 0);
  assert(game::SpawnPlayer(pid, 0, 100, 200, 3, 90));

  // fonts/config/strings
  assert(gui::FontSizeFor(10, 2) == 14);
  assert(gui::FontWeightFor(0) == 700);
  assert(strcmp(gui::FontFaceFor(""), "Arial") == 0);
  util::SetCfg("fontsize", "2");
  assert(util::GetCfgInt("fontsize", 0) == 2);
  assert(util::BasenameOf("C:\\GTASA\\samp.saa  ") == "samp.saa");
  assert(IsVirtualHandle(0xFF000005));
  assert(!IsVirtualHandle(0x100));

  std::string c = util::FormatCrash(nullptr, kVersion);
  assert(c.find("SA-MP") != std::string::npos);

  printf("samp_smoke: all %s checks passed\n", kVersion);
  return 0;
}
