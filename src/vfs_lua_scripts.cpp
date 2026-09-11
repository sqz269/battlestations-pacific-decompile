#include "bsp/vfs_lua_scripts.hpp"
#include "bsp/memory_stream.hpp"
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
void supported_string(const std::string& text) {
    if (text.find('\0') != std::string::npos ||
        text.size() > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
        throw std::invalid_argument("Lua override path requires a bounded native C string");
}
}
void append_lua_script_overrides_00bdef90(const std::string& path,
    const std::vector<std::string>& suffixes, const VfsCandidateExists& exists,
    std::vector<std::string>& output) {
    supported_string(path);
    if (!exists) throw std::invalid_argument("Lua override lookup requires VFS existence");
    for (const auto& suffix : suffixes) supported_string(suffix);
    const auto slash = path.find_last_of('/');
    const auto dot = path.find('.', slash == std::string::npos ? 0 : slash);
    const std::string stem = path.substr(0, dot);
    const std::string extension = dot == std::string::npos ? std::string{} : path.substr(dot);
    for (const auto& suffix : suffixes) {
        std::string candidate = stem + '_' + suffix + extension;
        if (exists(candidate)) output.push_back(std::move(candidate));
    }
}
VfsLuaScriptFiles::VfsLuaScriptFiles(VfsMountContext& vfs,
    const std::vector<std::string>& suffixes) noexcept : vfs_(vfs), suffixes_(suffixes) {}
bool VfsLuaScriptFiles::read_file_00b66ca0(const std::string& path, std::uint32_t mode,
    std::vector<char>& output) {
    if (mode != 2) throw std::invalid_argument("Native Lua reads use VFS mode2");
    auto opened = open_resource_memory_00bdf310_fragment(vfs_, path, mode);
    if (!opened.provider_opened) return false;
    if (!opened.stream || !opened.stream->has_backing() || !opened.stream->fully_initialized())
        throw std::runtime_error(opened.error.empty() ? "Lua VFS stream has incomplete backing" : opened.error);
    const auto length = opened.stream->size_00bef600();
    if (length < 0 || length > std::numeric_limits<std::int32_t>::max())
        throw std::length_error("Lua VFS stream exceeds the recovered read domain");
    std::vector<char> bytes(static_cast<std::size_t>(length));
    if (length != 0) {
        std::uint32_t actual{};
        if (!opened.stream->read_00bef590(bytes.data(), static_cast<std::uint32_t>(length), &actual)
            || actual != static_cast<std::uint32_t>(length))
            throw std::runtime_error("Lua VFS stream did not supply its full extent");
    }
    output.swap(bytes);
    return true;
}
std::vector<std::string> VfsLuaScriptFiles::override_paths_00bdef90(const std::string& path) {
    std::vector<std::string> result;
    append_lua_script_overrides_00bdef90(path, suffixes_, [this](const std::string& candidate) {
        return exists_resource_00bdd440_fragment(vfs_, candidate);
    }, result);
    return result;
}
const std::string& VfsLuaScriptFiles::cached_fundamentals_00884770() {
    if (!fundamentals_) {
        std::vector<char> source;
        if (!read_file_00b66ca0("Scripts\\fundamentals.lua", 2, source))
            throw std::runtime_error("VFS fundamentals.lua is missing");
        fundamentals_.emplace(source.begin(), source.end());
    }
    return *fundamentals_;
}
LuaStateOwnerEnvironment VfsLuaScriptFiles::owner_environment(LuaScriptRuntime& runtime,
    const LuaRuntimeGlobals& globals) {
    LuaStateOwnerEnvironment result;
    result.fundamentals = cached_fundamentals_00884770();
    result.do_file = &LuaScriptRuntime::do_file;
    result.do_file_context = &runtime;
    result.x360comp = globals.x360comp;
    result.region = globals.region;
    return result;
}
}
