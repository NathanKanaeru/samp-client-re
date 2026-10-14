// Smoke test: exercises BitStream, packets, pools, config, crash format.
// No GTA / network / D3D needed.
#include <cstdio>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
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
#include "samp/gui/dialog.h"
#include "samp/gui/colors.h"
#include "samp/gui/deathwindow.h"
#include "samp/gui/scoreboard.h"
#include "samp/net/death_codec.h"
#include "samp/net/compressed_vector.h"
#include "samp/input/cmd_window.h"
#include "samp/input/wndproc.h"
#include "samp/audio/audio.h"
#include "samp/game/models.h"
#include "samp/game/spawn.h"
#include "samp/game/slots.h"
#include "samp/gui/textdraw_pool.h"
#include "samp/gui/bubbles.h"
#include "samp/util/pacer.h"
#include "samp/gui/fonts.h"
#include "samp/util/config.h"
#include "samp/util/string_utils.h"
#include "samp/util/crash.h"
#include "samp/core/saa.h"
#include "samp/net/endpoint.h"
#include "samp/net/socket_compat.h"
#include "samp/net/bootstrap.h"
#include "samp/net/raknet_adapter.h"
#include "samp/net/sync.h"
#include "samp/net/query.h"
#include "samp/net/join.h"
#include "samp/net/menu_rpc.h"
#include "samp/net/rcon.h"
#include "samp/net/rpc_dispatch.h"
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

  // spawn FSM (localplayer flow)
  {
    game::SpawnManager sm;
    assert(!sm.RequestClass(0)); // no classes yet
    game::PlayerClass c;
    c.skin = 7; c.x = 1; c.y = 2; c.z = 3; c.rot = 90;
    sm.AddClass(c);
    assert(sm.RequestClass(0) && sm.state() == game::SpawnState::ClassRequested);
    assert(!sm.RequestSpawn()); // not settled
    sm.OnClassSettled();
    assert(sm.RequestSpawn() && sm.state() == game::SpawnState::WaitSpawnReply);
    game::SpawnInfo si;
    si.team = 1; si.skin = 7; si.x = 1; si.y = 2; si.z = 3; si.rot = 90;
    si.weapons[0] = 24; si.ammo[0] = 100; si.weapons[2] = 30; si.ammo[2] = 200;
    sm.SetSpawnInfo(si);
    static int gave[4];
    static int ngave;
    ngave = 0;
    game::SpawnEvents ev;
    ev.giveWeapon = [](int w, int a) { gave[ngave++] = w * 1000 + a; };
    assert(sm.Spawn(ev) && sm.state() == game::SpawnState::Spawned);
    assert(ngave == 2 && gave[0] == 30 * 1000 + 200 && gave[1] == 24 * 1000 + 100); // 2,1,0 order
  }

  // dialog state (styles 0-5, tab parse, caption cap)
  {
    gui::Dialog& d = gui::GetDialog();
    assert(!d.ShowDialog(1, 9, "c", "t", "a", "b")); // bad style
    assert(d.ShowDialog(3, 4, std::string(100, 'C'), "h1\th2\nr1c1\tr1c2\nr2c1\tr2c2", "OK", "Cancel"));
    assert(d.state().visible && d.state().caption.size() == 64);
    assert(d.state().rows.size() == 3 && d.state().rows[1].size() == 2);
    assert(d.state().rows[1][0] == "r1c1");
    assert(d.SelectRow(2) && d.state().selected == 2);
    assert(!d.SelectRow(9));
    d.Hide();
    assert(!d.state().visible && !d.SelectRow(0));
    assert(d.ShowDialog(4, 0, "hi", "msg", "OK", ""));
    assert(d.state().rows.empty()); // msgbox: no rows
    d.Hide();
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

  // compressed unit wire format (Knogle/RakNet BitStream.h:907-915,1124-1129)
  {
    net::BitStream b1;
    b1.WriteCompressedUnit(-1.0f);
    b1.WriteCompressedUnit(1.0f);
    b1.WriteCompressedUnit(0.0f);
    b1.WriteCompressedUnit(5.0f); // clamped to 1
    net::BitStream r1;
    r1.SetData(b1.Data(), b1.BitsUsed() / 8, true);
    float v = 99;
    assert(r1.ReadCompressedUnit(v) && v > -1.001f && v < -0.999f);
    assert(r1.ReadCompressedUnit(v) && v > 0.999f && v < 1.001f);
    assert(r1.ReadCompressedUnit(v) && v > -0.001f && v < 0.001f);
    assert(r1.ReadCompressedUnit(v) && v > 0.999f); // clamped
    // vector gate: zero-mag writes no direction bytes
    net::BitStream b2;
    b2.WriteCompVector(0.0f, 9, 9, 9);
    assert(b2.BitsUsed() == 32u);
    net::BitStream r2;
    r2.SetData(b2.Data(), 4, true);
    float m = -1, o[3] = {9, 9, 9};
    assert(r2.ReadCompVector(m, o) && m == 0.0f && o[0] == 0 && o[1] == 0 && o[2] == 0);
  }

  // sync codecs round-trip (field order per shared.h)
  {
    net::OnFootSync a;
    a.lr = 128; a.keys = 0xFF; a.px = 1.5f; a.rot = 90; a.health = 88;
    a.mx = 0.5f; a.surfVehicle = 12;
    net::BitStream bs;
    net::EncodeOnFoot(bs, a);
    net::BitStream rd;
    rd.SetData(bs.Data(), bs.BitsUsed() / 8, true);
    net::OnFootSync b;
    assert(net::DecodeOnFoot(rd, b));
    assert(b.lr == 128 && b.keys == 0xFF && b.px == 1.5f && b.rot == 90);
    assert(b.health == 88 && b.mx == 0.5f && b.surfVehicle == 12);

    net::AimSync am;
    am.camMode = 4; am.fx = 1; am.aimZ = 0.5f; am.camExtZoom = 63; am.weaponState = 2; am.aspect = 1;
    net::BitStream bs2;
    net::EncodeAim(bs2, am);
    // 1+24+4+1+1 = 31 bytes (AIM_SYNC_DATA size=31)
    assert(bs2.BitsUsed() == 31u * 8u);
    net::BitStream rd2;
    rd2.SetData(bs2.Data(), 31, true);
    net::AimSync am2;
    assert(net::DecodeAim(rd2, am2));
    assert(am2.camMode == 4 && am2.fx == 1 && am2.aimZ == 0.5f);
    assert(am2.camExtZoom == 63 && am2.weaponState == 2 && am2.aspect == 1);

    net::InCarSync ic;
    ic.vehicle = 7; ic.keys = 32; ic.dir[0] = 1.0f; ic.px = 10; ic.carHealth = 900;
    ic.tires[0] = 1; ic.trailer = 3; ic.hydraAngle = 45; ic.trainSpeed = 1.5f; ic.info = 2;
    net::BitStream bs3;
    net::EncodeInCar(bs3, ic);
    net::BitStream rd3;
    rd3.SetData(bs3.Data(), bs3.BitsUsed() / 8, true);
    net::InCarSync ic2;
    assert(net::DecodeInCar(rd3, ic2));
    assert(ic2.vehicle == 7 && ic2.keys == 32 && ic2.px == 10 && ic2.carHealth == 900);
    assert(ic2.dir[0] > 0.999f && ic2.tires[0] == 1 && ic2.trailer == 3);
    assert(ic2.hydraAngle == 45 && ic2.trainSpeed == 1.5f && ic2.info == 2);
  }

  // query protocol build/parse
  {
    std::vector<uint8_t> q;
    net::BuildQuery('i', 0x7F000001u, 7777, q);
    assert(q.size() == 11u && q[0] == 'S' && q[4] == 127 && q[5] == 0);
    assert(q[8] == 0x61 && q[9] == 0x1E && q[10] == 'i'); // port 7777 LE
    std::vector<uint8_t> rc;
    net::BuildRcon("cmdlist", 0x7F000001u, 7777, rc);
    assert(rc.size() == 11u + 2u + 7u && rc[10] == 'c');
    // synthetic 'i' reply
    std::vector<uint8_t> rep = {'S', 'A', 'M', 'P', 'i', 0, 3, 0, 20, 0};
    auto push = [&](const char* s) {
      uint32_t n = (uint32_t)strlen(s);
      rep.push_back((uint8_t)n); rep.push_back(0); rep.push_back(0); rep.push_back(0);
      rep.insert(rep.end(), s, s + n);
    };
    push("HN"); push("GM"); push("EN");
    net::QueryInfo info;
    assert(net::ParseInfoReply(rep.data(), rep.size(), info));
    assert(!info.locked && info.players == 3 && info.maxPlayers == 20);
    assert(info.hostname == "HN" && info.gamemode == "GM" && info.language == "EN");
    assert(!net::ParseInfoReply(rep.data(), 5, info)); // truncated
    // 'd' reply with one player
    std::vector<uint8_t> det = {'S', 'A', 'M', 'P', 'd', 1, 0, 5, 3, 'B', 'o', 'b'};
    det.insert(det.end(), {10, 0, 0, 0, 20, 0, 0, 0}); // score 10, ping 20
    std::vector<net::QueryPlayer> pls;
    assert(net::ParseDetailReply(det.data(), det.size(), pls));
    assert(pls.size() == 1 && pls[0].id == 5 && pls[0].name == "Bob");
    assert(pls[0].score == 10 && pls[0].ping == 20);
  }

  // interop golden vectors from REAL RakNet (temp clone, MIT-hygiene: facts only)
  {
    const uint8_t golden[18] = {0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xBF,
                                0x00, 0x00, 0x00, 0x40, 0xFF, 0xBF, 0xFF, 0x7F, 0xFF, 0x3F};
    net::BitStream ours;
    ours.WriteCompressedUnit(-1.0f);
    ours.WriteCompressedUnit(1.0f);
    ours.WriteCompressedUnit(0.0f);
    ours.WriteCompressedUnit(0.5f);
    float two = 2.0f;
    ours.WriteBytes(&two, 4);
    ours.WriteCompressedUnit(0.5f);
    ours.WriteCompressedUnit(0.0f);
    ours.WriteCompressedUnit(-0.5f);
    assert(ours.BitsUsed() == 18u * 8u);
    assert(memcmp(ours.Data(), golden, 18) == 0); // byte-identical to RakNet
    net::BitStream rd;
    rd.SetData(golden, 18, true);
    float v = 0;
    assert(rd.ReadCompressedUnit(v) && v == -1.0f);
    assert(rd.ReadCompressedUnit(v) && v > 0.999f && v < 1.001f);
    assert(rd.ReadCompressedUnit(v) && v > -0.001f && v < 0.001f);
  }

  // ClientJoin 0.3.7-shape exact bytes (netgame.cpp:758-786 reference)
  {
    net::BitStream bs;
    net::BuildClientJoin(bs, "Bob", 0x12345678u, "CID", "1.0-alpha", 0x01);
    const uint8_t* d = bs.Data();
    size_t n = bs.BitsUsed() / 8;
    assert(n == 4u + 1u + 1u + 3u + 4u + 1u + 3u + 1u + 10u);
    assert(d[0] == 0x92 && d[1] == 0x10 && d[2] == 0 && d[3] == 0); // 4242 LE
    assert(d[4] == 0x01 && d[5] == 3);                             // mod + nameLen
    assert(memcmp(d + 6, "Bob", 3) == 0);
    assert(d[9] == 0x78 && d[10] == 0x56 && d[11] == 0x34 && d[12] == 0x12);
    assert(d[13] == 3 && memcmp(d + 14, "CID", 3) == 0);
    assert(d[17] == 10 && memcmp(d + 18, "1.0-alpha", 9) == 0 && d[27] == 0);
  }

  // crash full original shape (@0x1011b610)
  {
    util::CrashRegs r;
    r.eip = 0x10001234; r.base = 0x10000000; r.eax = 1; r.esp = 0x10FF00;
    uint32_t st[2] = {0xAAAAAAAAu, 0xBBBBBBBBu};
    std::string s = util::FormatCrashFull(r, "0.3.DL-R1", st, 2);
    assert(s.find("Exception At Address: 0x10001234") != std::string::npos);
    assert(s.find("EAX: 0x00000001") != std::string::npos);
    assert(s.find("0xAAAAAAAA") != std::string::npos);
  }

  // config: case-insensitive, caps, parse text
  {
    util::ClearCfg();
    util::ParseCfgText("# comment\nFontSize = 2\nfontsize=5\n新key=1\n");
    assert(util::GetCfgInt("FONTSIZE", 0) == 5); // last wins, any case
    for (int i = 0; i < 40; ++i) util::SetCfg("k" + std::to_string(i), "v");
    util::SetCfg("fontsize", "9"); // existing key still writable when full
    assert(util::GetCfgInt("fontsize", 0) == 9);
  }

  // slot pools (caps per limits.h)
  {
    static_assert(game::GangZonePool::kCap == 1024, "gz");
    static_assert(game::LabelPool::kCap == 2048, "labels");
    static_assert(game::PickupPool::kCap == 400, "pickups");
    static_assert(game::MenuPool::kCap == 128, "menus");
    game::GangZonePool gz;
    int a = gz.Alloc(), b = gz.Alloc();
    assert(a == 0 && b == 1 && gz.Count() == 2);
    gz.Get(a)->color = 0xFF0000;
    assert(gz.Free(a) && !gz.Free(a) && gz.Count() == 1 && gz.Get(a) == nullptr);
    game::LabelPool lb;
    int l = lb.Alloc();
    assert(l == 0);
    lb.Get(l)->text = "base";
    assert(lb.Get(l)->text == "base");
    game::PickupPool pk;
    assert(pk.Alloc() == 0 && pk.Count() == 1);
    game::MenuPool mn;
    int m = mn.Alloc();
    assert(m == 0);
    mn.Get(m)->title = "Weapons";
    mn.Get(m)->items[0] = "Desert Eagle";
    mn.Get(m)->itemCount = 1;
    assert(mn.Get(m)->itemCount == 1);
  }

  // end-to-end sync pipeline over real UDP loopback (no server needed)
  {
    WSADATA wd{};
    assert(WSAStartup(MAKEWORD(2, 2), &wd) == 0);
    net::Socket tx = net::kInvalidSocket, rx = net::kInvalidSocket;
    assert(net::UdpBind(0, nullptr, true, &tx) == 0);
    assert(net::UdpBind(0, nullptr, true, &rx) == 0);
    uint16_t rxPort = 0;
    assert(net::GetsockPort(rx, &rxPort) == 0);
    uint32_t lo = 0;
    assert(net::ResolveFirst("localhost", &lo) == 0);
    assert(game::Vehicles().Create(21, 411, 0, 0, 0, 0));
    uint8_t wire[14] = {};
    wire[0] = 21;
    float tx3[3] = {30.0f, 0, 0};
    memcpy(wire + 2, tx3, 12);
    net::CNetGame g;
    int err = 0;
    assert(net::UdpSendTo(tx, wire, sizeof wire, lo, rxPort, &err) == (long)sizeof wire);
    g.NoteSent(sizeof wire);
    uint8_t got[64] = {};
    size_t gotN = 0;
    int tri = -2;
    for (int i = 0; i < 200 && tri != 1; ++i) {
      tri = net::UdpRecvFrom(rx, got, sizeof got, &gotN, nullptr, nullptr);
      if (tri == 0) Sleep(5);
    }
    assert(tri == 1 && gotN == sizeof wire);
    assert(g.RouteSync(200, got, gotN));
    auto* v = game::Vehicles().Get(21);
    assert(v && v->pos[0] == 30.0f); // snapped through the real socket path
    assert(g.syncCount(200) == 1);
    std::string fs = net::CNetGame::FormatStats(g.stats());
    assert(fs.find("Up: 1 pkts") != std::string::npos);
    assert(fs.find("Down: 1 pkts") != std::string::npos);
    game::DeleteVehicle(21);
    net::CloseSocket(tx);
    net::CloseSocket(rx);
    WSACleanup();
  }

  // extended sync routes (207/203/209/211)
  {
    net::CNetGame g;
    uint8_t onf[18] = {};
    onf[4] = 0x34; onf[5] = 0x12; // keys 0x1234
    float ox = 5.0f;
    memcpy(onf + 6, &ox, 4);
    assert(g.RouteSync(207, onf, sizeof onf));
    assert(g.lastFrames().hasOnFoot && g.lastFrames().keys == 0x1234);
    assert(g.lastFrames().x == 5.0f);
    assert(!g.RouteSync(207, onf, 10)); // too short
    net::BitStream ab;
    net::AimSync a2;
    a2.camMode = 7; a2.aimZ = 1.25f;
    net::EncodeAim(ab, a2);
    std::vector<uint8_t> av(ab.BitsUsed() / 8);
    memcpy(av.data(), ab.Data(), av.size());
    assert(g.RouteSync(203, av.data(), av.size()));
    assert(g.lastFrames().hasAim && g.lastFrames().camMode == 7);
    assert(g.lastFrames().aimZ == 1.25f);
    uint8_t px[14] = {};
    assert(g.RouteSync(209, px, sizeof px));
    assert(g.RouteSync(211, px, sizeof px));
    assert(!g.RouteSync(209, px, 3));
    assert(g.syncCount(207) == 2 && g.syncCount(203) == 1);
  }

  // textdraw pool (2304) + bubbles (200x144)
  {
    gui::TextDrawPool& tp = gui::TextDraws();
    int id = tp.AllocId();
    assert(id >= 0);
    assert(tp.SetText((uint32_t)id, "hello", 5));
    assert(!tp.SetText((uint32_t)id, "x", 0x2000)); // size guard
    assert(tp.Show(id) && tp.Shown() == 1);
    assert(tp.Hide(id) && tp.Shown() == 0);
    assert(tp.Free(id) && !tp.Free(id));
    gui::ChatBubbles bb;
    assert(bb.Set(3, std::string(200, 'z'), 1000));
    assert(bb.Get(3) && bb.Get(3)->text.size() == 144); // truncated
    assert(bb.Active() == 1);
    bb.Expire(1000 + 4999, 5000);
    assert(bb.Active() == 1);
    bb.Expire(1000 + 5000, 5000);
    assert(bb.Active() == 0 && bb.Get(3) == nullptr);
    assert(!bb.Set(500, "x", 0)); // out of range
  }

  // class navigator debounce + frame pacer
  {
    game::ClassNavigator nav;
    nav.SetCount(3);
    assert(nav.Next(100) && nav.Current() == 1);
    assert(!nav.Next(200)); // debounced
    assert(nav.Next(100 + 250) && nav.Current() == 2);
    assert(nav.Prev(1000) && nav.Current() == 1);
    util::FramePacer fp;
    fp.SetFps(90);
    assert(fp.FrameMs() == 11u);
    uint32_t last = 0;
    assert(fp.Due(1000, last));
    assert(!fp.Due(1005, last));
    assert(fp.Due(1011, last));
  }

  // passenger/spectator/trailer codecs round-trip
  {
    net::PassengerSync p;
    p.vehicle = 9; p.seats = 5; p.driveBy = 1; p.weapon = 29; p.px = 3.0f;
    net::BitStream bs;
    net::EncodePassenger(bs, p);
    net::BitStream rd;
    rd.SetData(bs.Data(), bs.BitsUsed() / 8, true);
    net::PassengerSync p2;
    assert(net::DecodePassenger(rd, p2));
    assert(p2.vehicle == 9 && p2.seats == 5 && p2.driveBy == 1);
    assert(p2.weapon == 29 && p2.px == 3.0f);

    net::SpectatorSync s;
    s.keys = 0x10; s.pz = -2.0f;
    net::BitStream bs2;
    net::EncodeSpectator(bs2, s);
    net::BitStream rd2;
    rd2.SetData(bs2.Data(), bs2.BitsUsed() / 8, true);
    net::SpectatorSync s2;
    assert(net::DecodeSpectator(rd2, s2));
    assert(s2.keys == 0x10 && s2.pz == -2.0f);

    net::TrailerSync t;
    t.dir[1] = 1.0f; t.px = 7.0f; t.mz = -1.0f;
    net::BitStream bs3;
    net::EncodeTrailer(bs3, t);
    net::BitStream rd3;
    rd3.SetData(bs3.Data(), bs3.BitsUsed() / 8, true);
    net::TrailerSync t2;
    assert(net::DecodeTrailer(rd3, t2));
    assert(t2.dir[1] > 0.999f && t2.px == 7.0f && t2.mz == -1.0f);
  }

  // RPC dispatch with recording callbacks
  {
    static std::string gotChat;
    static std::string gotDeath;
    net::RpcDispatcher disp;
    disp.RegisterBuiltins();
    net::WorldState w;
    net::RpcCtx ctx;
    ctx.world = &w;
    ctx.chat = (void*)1;
    ctx.chatAdd = [](void*, const char* t, int) { gotChat = t; };
    ctx.deaths = (void*)2;
    ctx.deathAdd = [](void*, const char* a, const char*, int) { gotDeath = a; };
    net::BitStream bc;
    bc.WriteU8(5); bc.WriteBytes("hello", 5);
    net::BitStream rc;
    rc.SetData(bc.Data(), bc.BitsUsed() / 8, true);
    assert(disp.Dispatch(21, rc, ctx) && gotChat == "hello");
    net::BitStream bw;
    bw.WriteU8(20); bw.WriteU8(30);
    net::BitStream rw;
    rw.SetData(bw.Data(), bw.BitsUsed() / 8, true);
    assert(disp.Dispatch(13, rw, ctx) && w.hour == 20 && w.minute == 30);
    net::BitStream bd;
    uint8_t draw[5] = {1, 0, 2, 0, 31};
    bd.WriteBytes(draw, 5);
    net::BitStream rd;
    rd.SetData(bd.Data(), 5, true);
    assert(disp.Dispatch(32, rd, ctx) && gotDeath == "player1");
    net::BitStream be;
    assert(!disp.Dispatch(99, be, ctx)); // unknown
    assert(disp.handled() == 3 && disp.unhandled() == 1);
  }

  // scoreboard update + chat slice/timestamp + menu responses
  {
    gui::Scoreboard sb;
    std::string names[3] = {"b", "a", "c"};
    int scores[3] = {5, 5, 9};
    int pings[3] = {50, 30, 10};
    assert(sb.UpdateList(names, scores, pings, 3));
    assert(sb.rows()[0].name == "c" && sb.rows()[1].name == "a");
    assert(!sb.UpdateList(nullptr, scores, pings, 3));

    gui::ChatWindow cw2;
    cw2.SetPageSize(2);
    cw2.AddMessage("one", 1);
    cw2.AddMessage("two", 1);
    cw2.AddMessage("three", 1);
    auto vis = cw2.VisibleSlice(0);
    assert(vis.size() == 2 && vis[0] == "two" && vis[1] == "three");
    auto vis1 = cw2.VisibleSlice(1);
    assert(vis1.size() == 2 && vis1[0] == "one" && vis1[1] == "two");
    cw2.SetTimestamp(true);
    auto vist = cw2.VisibleSlice(0);
    assert(vist[0] == "[12:00] two");
    assert(gui::ChatWindow::WithTimestamp("x", 3, 7) == "[03:07] x");

    net::BitStream ms;
    net::BuildMenuSelect(ms, 4, 2);
    assert(ms.BitsUsed() == 16u && ms.Data()[0] == 4 && ms.Data()[1] == 2);
    net::BitStream mq;
    net::BuildMenuQuit(mq, 4);
    assert(mq.BitsUsed() == 8u && mq.Data()[0] == 4);
  }

  // RCON session
  {
    net::RconSession rs;
    assert(!rs.Enqueue(""));
    for (int i = 0; i < net::RconSession::kMaxQueue; ++i)
      assert(rs.Enqueue("cmd" + std::to_string(i)));
    assert(!rs.Enqueue("overflow"));
    std::string cmd;
    assert(rs.NextToSend(cmd) && cmd == "cmd0");
    rs.MarkSent(1000);
    rs.OnReplyLine("line1", 1500);
    rs.OnReplyLine("late", 1000 + 2000); // window edge: dropped
    assert(rs.reply == "line1" && !rs.Expired(2999) && rs.Expired(3000));
    auto lines = net::SplitReplyLines("a\r\nb\nc");
    assert(lines.size() == 3 && lines[0] == "a" && lines[1] == "b" && lines[2] == "c");
  }

  printf("samp_smoke: all %s checks passed\n", kVersion);
  return 0;
}
