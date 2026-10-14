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
#include "samp/net/netgame.h"
#include "samp/gui/d3d_device_hook.h"
#include "samp/gui/chat.h"
#include "samp/gui/colors.h"
#include "samp/gui/deathwindow.h"
#include "samp/gui/scoreboard.h"
#include "samp/net/death_codec.h"
#include "samp/net/compressed_vector.h"
#include "samp/input/cmd_window.h"
#include "samp/input/wndproc.h"
#include "samp/audio/audio.h"
#include "samp/game/models.h"
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

  // vehicle playback blend (PACKET200 constants)
  {
    float cur[3] = {0, 0, 0}, out[3];
    game::RemoteFrame snap;
    snap.pos[0] = 100; snap.pos[1] = 0; snap.pos[2] = 0; snap.airOrBoat = false;
    game::BlendVehiclePos(cur, snap, out); // |err|>8 -> snap
    assert(out[0] == 100 && out[1] == 0 && out[2] == 0);
    game::RemoteFrame small;
    small.pos[0] = 0.03f; small.pos[1] = 0; small.pos[2] = 0; small.airOrBoat = false;
    game::BlendVehiclePos(cur, small, out); // within deadband -> hold
    assert(out[0] == 0 && out[1] == 0 && out[2] == 0);
    game::RemoteFrame mid;
    mid.pos[0] = 1.0f; mid.pos[1] = 0; mid.pos[2] = 0; mid.airOrBoat = false;
    game::BlendVehiclePos(cur, mid, out); // 1.0*0.06=0.06 > 0.01 -> apply
    assert(out[0] > 0.05f && out[0] < 0.07f);
  }

  // sync router: synthetic vehicle packet (u16 id + xyz)
  {
    net::CNetGame g;
    assert(game::Vehicles().Create(7, 411, 0, 0, 0, 0));
    uint8_t pkt[14] = {};
    pkt[0] = 7; pkt[1] = 0;
    float x = 50.0f;
    memcpy(pkt + 2, &x, 4);
    assert(g.RouteSync(200, pkt, sizeof pkt));
    assert(g.syncCount(200) == 1);
    auto* v = game::Vehicles().Get(7);
    assert(v && v->pos[0] == 50.0f); // snapped (|50|>8)
    assert(!g.RouteSync(200, pkt, 5)); // too short
    game::DeleteVehicle(7);
  }

  // D3D hook on a fake vtable (slot routing without D3D)
  {
    struct FakeDev { void** vtbl; };
    using PFn = long(__stdcall*)(void*, const void*, const void*, void*, const void*);
    using RFn = long(__stdcall*)(void*, void*);
    using OFn = long(__stdcall*)(void*);
    PFn fakePresent = [](void*, const void*, const void*, void*, const void*) -> long { return 7; };
    RFn fakeReset = [](void*, void*) -> long { return 9; };
    OFn fakeOther = [](void*) -> long { return 3; };
    void* table[20];
    for (int i = 0; i < 20; ++i) table[i] = (void*)fakeOther;
    table[17] = (void*)fakePresent;
    table[16] = (void*)fakeReset;
    FakeDev dev{table};
    auto* h = gui::HookDevice(&dev, 20);
    assert(h && gui::HookVTable(h)[17] != table[17]);
    assert(gui::HookVTable(h)[5] == table[5]); // untouched slots forward
    // Override resolves the hook by device and forwards to the original.
    PFn hookedPresent = (PFn)gui::HookVTable(h)[17];
    assert(hookedPresent(&dev, nullptr, nullptr, nullptr, nullptr) == 7);
    RFn hookedReset = (RFn)gui::HookVTable(h)[16];
    assert(hookedReset(&dev, nullptr) == 9);
    gui::UnhookDevice(h);
    gui::UnhookDevice(nullptr); // safe no-op
  }

  // fonts/config/strings
  assert(gui::FontSizeFor(10, 2) == 14);
  assert(gui::FontWeightFor(0) == 700);
  assert(strcmp(gui::FontFaceFor(""), "Arial") == 0);

  // color tags (runutil.cpp exact rule)
  assert(gui::ParseColorTag("{FF0000}hi") == 0xFF0000);
  assert(gui::ParseColorTag("{ff0000}hi") == 0xFF0000);
  assert(gui::ParseColorTag("{GG0000}") == gui::kNoColor);
  assert(gui::ParseColorTag("{FF00}") == gui::kNoColor);
  assert(gui::ParseColorTag("hi") == gui::kNoColor);
  assert(gui::StripColorTags("A{FF0000}B{x}C") == "AB{x}C");

  // chat store (100-entry shift, wrap at 72)
  {
    gui::ChatWindow cw;
    cw.AddMessage(std::string(200, 'x'), 0);
    assert(cw.Latest().text.size() <= 72u);
    cw.AddMessage("short", 1);
    assert(cw.Latest().text == "short" && cw.Total() == 2);
    cw.SetPageSize(5);
    assert(cw.PageSize() == 5);
  }

  // death codec + window ring
  {
    uint8_t raw[5] = {0x05, 0x00, 0x0A, 0x00, 31};
    net::DeathMessage dm;
    assert(net::DecodeDeathMessage(raw, 5, &dm));
    assert(dm.killer == 5 && dm.killee == 10 && dm.reason == 31);
    assert(!net::DecodeDeathMessage(raw, 4, &dm));
    gui::DeathWindow dw;
    for (int i = 0; i < 7; ++i) dw.Add("a", "b", i);
    assert(dw.Count() == 5 && dw.At(4).reason == 6);
    dw.AddSpecial("joiner", gui::kSpecialConnect);
    assert(dw.At(4).reason == gui::kSpecialConnect);
  }

  // compressed-vector gate math
  {
    float o[3];
    assert(net::ScaleCompressedVector(2.0f, 0.5f, 0.0f, -0.5f, o));
    assert(o[0] == 1.0f && o[2] == -1.0f);
    assert(net::ScaleCompressedVector(0.0f, 1, 1, 1, o) && o[0] == 0 && o[1] == 0 && o[2] == 0);
    assert(!net::ScaleCompressedVector(-1.0f, 0, 0, 0, o));
  }

  // cmd recall ring (10x128)
  {
    input::CmdHistory h;
    h.Push("first");
    h.Push(std::string(200, 'y'));
    assert(h.Size() == 2);
    assert(h.RecallUp() == std::string(128, 'y')); // truncated
    assert(h.RecallUp() == "first");
    assert(h.RecallDown() == std::string(128, 'y'));
    assert(h.RecallDown() == "");
  }

  // scoreboard sort
  {
    std::vector<gui::Scoreboard::Row> rows = {{"b", 5, 50}, {"a", 5, 30}, {"c", 9, 10}};
    gui::Scoreboard::SortRows(rows);
    assert(rows[0].name == "c" && rows[1].name == "a" && rows[2].name == "b");
  }

  // audio: volume law + meta parse + tick
  {
    assert(audio::VolumeFor(-1, 60, 1.0f) == 8000); // 2D
    assert(audio::VolumeFor(30, 60, 1.0f) == 4000); // linear half
    assert(audio::VolumeFor(60, 60, 0.5f) == 0);    // out of range
    assert(audio::VolumeFor(0, 60, 0.5f) == 4000);
    assert(audio::ParseMetaTitle("StreamTitle='Cool FM';", "") == "Cool FM");
    assert(audio::ParseMetaTitle("artist=DJ", "artist=") == "DJ");
    assert(audio::ParseMetaTitle("nothing", "") == "");
    audio::StreamState st;
    st.playing = true; st.maxDist = -1; st.lastTick = 0;
    audio::TickCtx ctx;
    ctx.nowMs = 200; ctx.radioVol = 0.5f;
    assert(audio::ProcessTick(st, ctx) == 4000);
    ctx.menuOpen = true;
    assert(audio::ProcessTick(st, ctx) == 0);
  }

  // wndproc router (subclass.cpp flow)
  {
    input::WndState s;
    assert(input::RouteMessage(s, {input::Msg::KeyUp, input::VK_F6_})); // open cmd
    assert(s.cmdActive && s.chatMode);
    assert(input::RouteMessage(s, {input::Msg::KeyUp, 0x09})); // TAB score
    assert(s.scoreVisible);
    assert(!input::RouteMessage(s, {input::Msg::KeyDown, 0x41})); // passthrough
    assert(input::RouteMessage(s, {input::Msg::KeyUp, input::VK_ESC_})); // ESC closes cmd first
    assert(!s.cmdActive && s.scoreVisible); // score still up
    assert(input::RouteMessage(s, {input::Msg::Char, input::VK_ESC_})); // ESC hides score
    assert(!s.scoreVisible);
    assert(!input::RouteMessage(s, {input::Msg::KeyUp, input::VK_F8_})); // flag set, msg continues to GTA
    assert(s.takeScreenshot);
    assert(input::RouteMessage(s, {input::Msg::SetCursor, 0})); // swallowed
    s.scoreVisible = true;
    assert(input::RouteMessage(s, {input::Msg::LButtonDown, 0, false})); // outside click
    assert(!s.scoreVisible && s.outsideScoreboard);
    assert(input::RouteMessage(s, {input::Msg::SysKeyUp, input::VK_RET_})); // windowed toggle
    input::WndState c;
    assert(input::RouteMessage(c, {input::Msg::Char, 't'})); // quick-talk opens cmd
    assert(c.cmdActive);
  }

  // DL-R1 model queue
  {
    game::ModelQueue q;
    assert(q.Request(0x1234));
    assert(!q.Request(0x1234)); // dup
    assert(q.MarkDownloading(0x1234));
    assert(q.MarkReady(0x1234));
    assert(q.Find(0x1234)->dff == game::ModelReqState::Ready);
    assert(game::DffName("samp", 0xAB) == "samp/0xAB.dff");
    assert(game::TxdName("samp", 0xAB) == "samp/0xAB.txd");
  }
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
