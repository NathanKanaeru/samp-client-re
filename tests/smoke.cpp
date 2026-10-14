// Smoke test: exercises BitStream, packets, pools, config, crash format.
// No GTA / network / D3D needed.
#include <cstdio>
#include <cassert>
#include <cstring>
#include "samp/common.h"
#include "samp/net/bitstream.h"
#include "samp/net/packets.h"
#include "samp/net/rpc.h"
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
