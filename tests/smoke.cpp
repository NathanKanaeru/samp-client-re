// Smoke test: exercises BitStream, packets, pools, config, crash format.
// No GTA / network / D3D needed.
#include <cstdio>
#include <cassert>
#include <cstring>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#endif
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
#include "samp/core/saa.h"
#include "samp/net/endpoint.h"
#include "samp/net/socket_compat.h"
#include "samp/net/bootstrap.h"
#include "samp/net/raknet_adapter.h"
#include <vector>

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

  // endpoint parsing (SPEC-NET-002 rules)
  {
    net::Endpoint ep;
    assert(net::ParseHostPort("example.com:7777", 7777, &ep) == 0);
    assert(strcmp(ep.host, "example.com") == 0 && ep.port == 7777);
    assert(net::ParseHostPort("203.0.113.10", 7777, &ep) == 0 && ep.port == 7777);
    assert(net::ParseHostPort("[2001:db8::10]:7777", 80, &ep) == 0 && ep.port == 7777);
    assert(net::ParseHostPort("2001:db8::10", 7777, &ep) == 0 && ep.port == 7777);
    assert(net::ParseHostPort("[2001:db8::10", 7777, &ep) == -1);
    assert(net::ParseHostPort("example.com:", 7777, &ep) == -1);
    assert(net::ParseHostPort(":7777", 7777, &ep) == -1);
    assert(net::ParseHostPort("example.com:99999", 7777, &ep) == -1);
    assert(net::ParseHostPort("", 7777, &ep) == -1);
  }

  // SAA hash vectors (libsamp test_saa.c anchors)
  assert(core::SaaHashFilename("main.scm") == core::saa::kHashMainScm);
  assert(core::SaaHashFilename("SCRIPT.IMG") == core::saa::kHashScriptImg);
  assert(core::SaaHashFilename("main.scm ") != core::saa::kHashMainScm); // no trim
  {
    uint32_t h = 0;
    assert(core::SaaHashPath("data\\script\\MAIN.SCM", &h) && h == core::saa::kHashMainScm);
    assert(core::SaaHashPath("data/script/script.img", &h) && h == core::saa::kHashScriptImg);
  }

  // SAA layout on the real samp.saa (../../samp.saa from build dir); skip if absent.
  {
    FILE* f = nullptr;
    fopen_s(&f, "..\\..\\samp.saa", "rb");
    if (f) {
      fseek(f, 0, SEEK_END);
      long sz = ftell(f);
      fseek(f, 0, SEEK_SET);
      std::vector<uint8_t> data((size_t)sz);
      assert(fread(data.data(), 1, (size_t)sz, f) == (size_t)sz);
      fclose(f);
      core::SaaLayout lay;
      assert(core::SaaParseLayout(data.data(), data.size(), &lay));
      assert(lay.header_size == 256 && lay.signature_size == 128);
      printf("[saa] real samp.saa: size=%ld entry_off=%zu sig_off=%zu invalid=%u\n",
             sz, lay.entry_offset, lay.signature_offset, lay.invalid_index);
      // Decrypt table + find/extract (keys documented in saa.cpp).
      std::vector<uint8_t> table(data.begin() + (ptrdiff_t)lay.entry_offset,
                                 data.begin() + (ptrdiff_t)lay.entry_offset + 2048);
      core::SaaDecryptTable(table.data());
      for (const char* name : {"main.scm", "script.img"}) {
        uint32_t h = core::SaaHashFilename(name);
        core::SaaFilePos fp;
        if (core::SaaFind(table.data(), lay.invalid_index, lay.header_size,
                          lay.entry_offset, h, &fp)) {
          std::vector<uint8_t> payload(fp.size);
          assert(core::SaaExtract(data.data(), table.data(), fp, payload.data()));
          printf("[saa] found %s: size=%u off=%zu magic=%02x%02x%02x%02x\n", name,
                 fp.size, fp.data_offset, payload[0], payload[1], payload[2], payload[3]);
        } else {
          printf("[saa] %s not in this archive (DL-R1 set differs from R5)\n", name);
        }
      }
    } else {
      printf("[saa] samp.saa not found, layout test skipped\n");
    }
  }

  // sockets: Winsock init, UDP loopback self-send, bootstrap lifecycle.
  {
    WSADATA wd{};
    assert(WSAStartup(MAKEWORD(2, 2), &wd) == 0);
    net::Socket s = net::kInvalidSocket;
    assert(net::UdpBind(0, nullptr, true, &s) == 0);
    uint16_t port = 0;
    assert(net::GetsockPort(s, &port) == 0 && port != 0);
    uint32_t lo = 0;
    assert(net::ResolveFirst("localhost", &lo) == 0);
    const char* hello = "samp-loopback";
    int err = 0;
    assert(net::UdpSendTo(s, hello, strlen(hello), lo, port, &err) > 0);
    char buf[64] = {};
    size_t got = 0;
    int tri = -2;
    for (int i = 0; i < 100 && tri == -2; ++i) {
      tri = net::UdpRecvFrom(s, buf, sizeof buf, &got, nullptr, nullptr);
      if (tri == 0) Sleep(10);
    }
    assert(tri == 1 && got == strlen(hello) && memcmp(buf, hello, got) == 0);
    net::CloseSocket(s);

    net::BootstrapManager bm;
    assert(net::BootstrapInit(&bm) == 0 && bm.initialized);
    assert(net::BootstrapInit(&bm) == 0); // idempotent
    assert(net::BootstrapStartListener(&bm, 0, 0) == 0 && bm.running);
    assert(net::BootstrapStartListener(&bm, 0, 0) == -1); // duplicate reject
    assert(net::BootstrapStop(&bm) == 0 && !bm.running);
    assert(net::BootstrapStop(&bm) == 0); // idempotent
    assert(net::BootstrapDestroy(&bm) == 0 && !bm.initialized);
    assert(net::RakAdapterAvailable() == false); // no submodule yet
    WSACleanup();
  }

  printf("samp_smoke: all %s checks passed\n", kVersion);
  return 0;
}
