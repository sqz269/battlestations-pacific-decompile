// Segment 87 Lua script cache (cc13-scripts1). Evidence and uncertainty:
// docs/CC13_LUA_SCRIPT_LOADER.md. Reconstructed and build-tested only; not game-validated.

#include "bsp/lua_script_loader_cc13.hpp"

#include <cstring>
#include <utility>

#include <lua.h>
#include <lauxlib.h>
#include <zlib.h>

namespace bsp::lua_script_loader_cc13 {

namespace {
// 00D62C70: double bytes 00 00 00 E0 4D 62 50 BF, the float -0.001 widened.
constexpr double kNegativeThousandth = -0.0010000000474974513;
}  // namespace

std::uint32_t compress_bound_00b6d1b0(std::uint32_t size) noexcept
{
    // 00B6D2C9..00B6D305: FILD of the signed size, +2^32 when negative (00CE3978), FMUL, then
    // FISTP qword under control word | 0C00h (truncate). ESI = size - low dword + 0Ch.
    const double scaled = static_cast<double>(size) * kNegativeThousandth;
    const auto truncated = static_cast<std::int64_t>(scaled);
    return size - static_cast<std::uint32_t>(truncated) + 0x0Cu;
}

bool build_compressed_00b6d1b0(const std::vector<std::string>& names, ReadScriptBytes read,
                               AppendScriptOverrides overrides, void* context,
                               std::vector<CompressedScriptRecord>& records)
{
    // 00B6D1E6..00B6D258: the name itself (004CDC20), then 00BDEF90 per name, into one
    // candidate vector, before any file is read.
    std::vector<std::string> candidates;
    for (const std::string& name : names) {
        candidates.push_back(name);
        overrides(name, candidates, context);
    }
    for (const std::string& path : candidates) {
        std::vector<unsigned char> bytes;
        if (!read(path, bytes, context))
            return false;  // native: null stream dereferenced at 00B6D28D
        const auto size = static_cast<std::uint32_t>(bytes.size());
        uLongf length = compress_bound_00b6d1b0(size);
        std::vector<unsigned char> packed(length);
        // 00B6D323: the compress status is not tested.
        (void)compress(packed.data(), &length, bytes.data(), size);
        CompressedScriptRecord record;
        record.data.assign(packed.begin(), packed.begin() + length);  // exact copy, 00B6D341
        record.compressed_size = static_cast<std::uint32_t>(length);
        record.bound = compress_bound_00b6d1b0(size);  // +8 keeps the bound, not the size
        records.push_back(std::move(record));
    }
    return true;
}

void run_record_00b6bd80(lua_State* state, const CompressedScriptRecord& record)
{
    // 00B6BD85..00B6BDA7: allocate record+8 bytes; uncompress updates the length slot that is
    // then passed to DoBuffer, so the chunk is the inflated length, not the bound.
    std::vector<char> text(record.bound);
    uLongf length = record.bound;
    (void)uncompress(reinterpret_cast<Bytef*>(text.data()), &length, record.data.data(),
                     record.compressed_size);
    // 00B65EC0: load status ignored; a compile error leaves the message string, which
    // lua_call then attempts to call.
    (void)luaL_loadbuffer(state, text.data(), length, "DoBuffer");
    lua_call(state, 0, LUA_MULTRET);
}

}  // namespace bsp::lua_script_loader_cc13
