#pragma once

// Segment 87 Lua script cache (cc13-scripts1, docs/CC13_LUA_SCRIPT_LOADER.md).
// New C++ interfaces, not the native register ABI or the native vector layouts. Names are
// hypotheses, not recovered symbols.

#include <cstdint>
#include <string>
#include <vector>

struct lua_State;

namespace bsp::lua_script_loader_cc13 {

// Native 0Ch-byte record {zlib data pointer, compressed size, uncompressed bound} pushed by
// 00B6CEF0. The host owns the bytes in `data` (size == compressed_size).
struct CompressedScriptRecord {
    std::vector<unsigned char> data;
    std::uint32_t compressed_size{};
    std::uint32_t bound{};
};

// VFS 0109CEEC +4(path, 2), +30h size, +24h read. Returns false when the open yields no stream;
// the native builder dereferences that null stream (00B6D28B), so a false return is a host
// failure, not a native outcome.
using ReadScriptBytes = bool (*)(const std::string& path, std::vector<unsigned char>& bytes,
                                 void* context);
// BSP_VfsManager_AppendLuaScriptOverrides 00BDEF90: appends the existing `stem_suffix.ext`
// variants of `path` in registered suffix order.
using AppendScriptOverrides = void (*)(const std::string& path, std::vector<std::string>& out,
                                       void* context);

// 00B6D1B0: size + trunc(size * -0.001 as x87 double) + 12, the zlib 1.2.1 compressBound rule
// computed with FILD/FMUL [00D62C70]/FISTP under a truncating control word.
std::uint32_t compress_bound_00b6d1b0(std::uint32_t size) noexcept;

// 00B6D1B0 BSP_LuaScriptCache_BuildCompressed. For each name, the name itself then its
// override variants; each candidate is read whole, compressed and appended to `records`.
// Returns false (with `records` holding the candidates built so far) where the native
// routine would fault on a missing stream.
bool build_compressed_00b6d1b0(const std::vector<std::string>& names, ReadScriptBytes read,
                               AppendScriptOverrides overrides, void* context,
                               std::vector<CompressedScriptRecord>& records);

// 00B6BD80 BSP_LuaScriptCache_RunRecord: inflate into a `bound`-byte buffer, then the
// 00B65EC0 DoBuffer shape: luaL_loadbuffer(..., "DoBuffer") with the status ignored and
// lua_call(L, 0, LUA_MULTRET). Unprotected like the original: call it inside lua_pcall/cpcall.
void run_record_00b6bd80(lua_State* state, const CompressedScriptRecord& record);

}  // namespace bsp::lua_script_loader_cc13
