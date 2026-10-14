#pragma once
// SA-MP server query protocol (browser/rcon-console). Public protocol:
// request = "SAMP" + u32 BE ip + u16 port + opcode; reply echoes "SAMP" +
// opcode-specific payload. Opcodes: 'p'ing, 'i'nfo, 'r'ules, 'd'etail players,
// 'c' rcon command, 'x'/'o' legacy. Info reply: password u8, players u16,
// maxplayers u16, hostname/var strings (u32 len + bytes), gamemode, language.
// Rules reply: u16 count + (u8 klen+key, u8 vlen+value)*.
// Detail reply: u16 count + (u8 id, u8 namelen+name, score u32, ping u32)*.
// Pure builder/parser here; socket IO stays in socket_compat.*. Tested.
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace samp { namespace net {

struct QueryInfo {
  bool locked = false;
  uint16_t players = 0, maxPlayers = 0;
  std::string hostname, gamemode, language;
};
struct QueryRule { std::string key, value; };
struct QueryPlayer { uint8_t id = 0; std::string name; uint32_t score = 0, ping = 0; };

// Builds a request into out (cleared first). ipBe = big-endian IPv4.
void BuildQuery(char opcode, uint32_t ipBe, uint16_t port, std::vector<uint8_t>& out);
// RCON variant ('c' + u16 cmdLen + cmd). Shares the header.
void BuildRcon(const std::string& cmd, uint32_t ipBe, uint16_t port, std::vector<uint8_t>& out);
// Parsers validate "SAMP" magic + opcode echo. False on truncation/mismatch.
bool ParseInfoReply(const uint8_t* data, size_t len, QueryInfo& out);
bool ParseRulesReply(const uint8_t* data, size_t len, std::vector<QueryRule>& out);
bool ParseDetailReply(const uint8_t* data, size_t len, std::vector<QueryPlayer>& out);

}} // namespace
