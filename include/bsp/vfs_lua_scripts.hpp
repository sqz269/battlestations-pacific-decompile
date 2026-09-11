#pragma once
#include "bsp/lua_script_runtime.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/vfs_candidates.hpp"
#include "bsp/vfs_mounts.hpp"
#include <optional>

namespace bsp {
// 00bdef90: ECX=VFS manager, path pointer and output vector, RET8. The suffix
// list is manager+48/+4C. Appends each existing candidate, preserving order and
// duplicates. Only '/' separates directories and the first following '.' starts
// the extension. Inputs/callbacks must remain stable; output must not alias them.
// New C++ interface; native NativeString allocator/layout and diagnostics omitted.
void append_lua_script_overrides_00bdef90(const std::string& path,
    const std::vector<std::string>& suffixes, const VfsCandidateExists& exists,
    std::vector<std::string>& output);

struct LuaRuntimeGlobals {
    bool x360comp{};
    std::string region;
};
// Actual VFS-backed files for Lua. Owns the cached fundamentals bytes; references
// the application's existing mount context and ordered suffix list. No disk-only
// bypass, extra search candidates or recursive directory scan is introduced.
class VfsLuaScriptFiles final : public LuaScriptFiles {
public:
    VfsLuaScriptFiles(VfsMountContext&, const std::vector<std::string>& suffixes) noexcept;
    bool read_file_00b66ca0(const std::string&, std::uint32_t mode,
        std::vector<char>& output) override;
    std::vector<std::string> override_paths_00bdef90(const std::string&) override;
    // Normal-flow composition of00884770/00b68340. First access reads the VFS
    // path once; subsequent Lua owners share cached source. Missing/incomplete
    // bytes throw instead of consuming native uninitialized fields/tails.
    const std::string& cached_fundamentals_00884770();
    LuaStateOwnerEnvironment owner_environment(LuaScriptRuntime&, const LuaRuntimeGlobals&);
private:
    VfsMountContext& vfs_;
    const std::vector<std::string>& suffixes_;
    std::optional<std::string> fundamentals_;
};
}
