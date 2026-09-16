#pragma once
#include "bsp/lua_script_runtime.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/vfs_candidates.hpp"
#include "bsp/vfs_mounts.hpp"
#include "bsp/native_string.hpp"
#include <optional>

namespace bsp {
struct NativeLuaBootstrapInputs;
// 00bdef90: ECX=VFS manager, path pointer and output vector, RET8. The suffix
// list is manager+48/+4C. Appends each existing candidate, preserving order and
// duplicates. Only '/' separates directories and the first following '.' starts
// the extension. Inputs/callbacks must remain stable; output must not alias them.
// New C++ interface; native NativeString allocator/layout and diagnostics omitted.
void append_lua_script_overrides_00bdef90(const std::string& path,
    const std::vector<std::string>& suffixes, const VfsCandidateExists& exists,
    std::vector<std::string>& output);

struct LuaRuntimeGlobals {
    std::uint8_t x360comp{}; // actual0108FF20 byte
    std::uint8_t reserved_0108ff21[3]{}; // same loader-zero padding before header
    NativeString region;   // actual0108FF24/28 header, no implicit destruction
};
static_assert(offsetof(LuaRuntimeGlobals, region) == 4);
static_assert(sizeof(LuaRuntimeGlobals) == 12);
// Actual VFS-backed files for Lua. Production borrows the raw fundamentals cache;
// unbound semantic fixtures retain their existing local cache. References
// the application's existing mount context and ordered suffix list. No disk-only
// bypass, extra search candidates or recursive directory scan is introduced.
class VfsLuaScriptFiles final : public LuaScriptFiles {
public:
    VfsLuaScriptFiles(VfsMountContext&, const std::vector<std::string>& suffixes,
        const NativeLuaBootstrapInputs* native_bootstrap = nullptr) noexcept;
    bool read_file_00b66ca0(const std::string&, std::uint32_t mode,
        std::vector<char>& output) override;
    std::vector<std::string> override_paths_00bdef90(const std::string&) override;
    // Return one semantic owner-input copy. A native binding captures length
    // and bytes from two canonical00884770 calls, with no replacement VFS read.
    // The unbound fixture path retains its checked local-cache behavior.
    std::string cached_fundamentals_00884770();
    LuaStateOwnerEnvironment owner_environment(LuaScriptRuntime&, const LuaRuntimeGlobals&);
private:
    VfsMountContext& vfs_;
    const std::vector<std::string>& suffixes_;
    // Production borrows the canonical native cache getter and never fills
    // the semantic fallback cache below. Keep this binding through Lua closes.
    const NativeLuaBootstrapInputs* native_bootstrap_;
    std::optional<std::string> fundamentals_;
};
}
