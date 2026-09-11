#include "bsp/vfs_locale_runtime.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/resource_path.hpp"
#include "bsp/vfs_lua_scripts.hpp"
#include <stdexcept>
#include <utility>

namespace bsp {
VfsLocaleRuntime::VfsLocaleRuntime(VfsMountContext& vfs,
    const VfsCandidateRegistrations& registrations, const std::vector<std::string>& suffixes,
    std::function<std::uint32_t()> hint_count) : vfs_(vfs), registrations_(registrations),
    suffixes_(suffixes), hint_count_(std::move(hint_count)) {
    if (!hint_count_) throw std::invalid_argument("Language catalog requires the current profile-hint count");
}
std::vector<std::string> VfsLocaleRuntime::override_paths_00bdef90(const std::string& name) {
    std::vector<std::string> result;
    append_lua_script_overrides_00bdef90(name, suffixes_, [this](const std::string& candidate) {
        return exists_resource_00bdd440_fragment(vfs_, candidate);
    }, result);
    return result;
}
bool VfsLocaleRuntime::read_bytes(const std::string& name, std::uint32_t mode,
    std::vector<std::uint8_t>& output) {
    auto opened = open_resource_memory_00bdf310_fragment(vfs_, name, mode);
    if (!opened.provider_opened) return false;
    if (!opened.stream || !opened.stream->has_backing() || !opened.stream->fully_initialized())
        throw std::runtime_error(opened.error.empty() ? "Locale VFS stream has incomplete backing" : opened.error);
    const auto length = opened.stream->size_00bef600();
    if (length < 0 || length > INT32_MAX) throw std::length_error("Locale VFS stream exceeds signed32 length");
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
    if (length != 0) {
        std::uint32_t actual{};
        if (!opened.stream->read_00bef590(bytes.data(), static_cast<std::uint32_t>(length), &actual)
            || actual != static_cast<std::uint32_t>(length))
            throw std::runtime_error("Locale VFS stream did not supply its full extent");
    }
    output.swap(bytes);
    return true;
}
bool VfsLocaleRuntime::read_file(const std::string& name, std::uint32_t mode,
    std::vector<std::uint8_t>& output) {
    if (mode != 2) throw std::invalid_argument("Native locale tables use mode2");
    return read_bytes(name, mode, output);
}
bool VfsLocaleRuntime::resolves_existing_name_00bdf4c0(const std::string& name) {
    auto copy = name;
    return resolve_existing_resource_00bdf4c0_fragment(vfs_, registrations_, copy);
}
std::vector<std::string> VfsLocaleRuntime::enumerate_descriptors_00886280(
    const std::string& directory, const std::string& extension, std::uint32_t flags) {
    auto normalized = directory;
    if (!normalize_resource_path_00bee690(normalized))
        throw std::invalid_argument("Language descriptor enumeration path is unsupported");
    std::vector<std::string> output;
    std::string error;
    if (!enumerate_resources_00bdd990_fragment(vfs_, normalized, extension, flags, output, error))
        throw std::runtime_error(error);
    return output;
}
bool VfsLocaleRuntime::read_descriptor_00bef2e0(const std::string& name, std::uint32_t mode,
    std::vector<std::uint8_t>& output) {
    if (mode != 0x32) throw std::invalid_argument("Native descriptor scanner uses mode32h");
    return read_bytes(name, mode, output);
}
std::uint32_t VfsLocaleRuntime::profile_hints_count_08() { return hint_count_(); }
}
