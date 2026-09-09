#include "bsp/physical_directory.hpp"
#include "bsp/physical_file.hpp"
#include "bsp/resource_path.hpp"
#include <cstdint>
#include <limits>
#include <cstring>
#include <utility>

namespace bsp {
namespace {
bool supported_string(const std::string& value) noexcept {
    return value.size() <= static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())
        && value.find('\0') == std::string::npos;
}
}

PhysicalDirectory::PhysicalDirectory(std::string root, bool accept_nonempty_names)
    : root_(std::move(root)), supported_(supported_string(root_)),
      accept_nonempty_names_(accept_nonempty_names) {}

bool PhysicalDirectory::build_path_00bf3970(const std::string& suffix,
    std::string& output) const {
    if (!supported_ || !supported_string(suffix) ||
        suffix.size() > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())
            - root_.size()) return false;
    auto path = root_ + suffix;
    for (std::size_t i = root_.size(); i < path.size(); ++i)
        if (path[i] == '/') path[i] = '\\';
    output = std::move(path);
    return true;
}

bool PhysicalDirectory::exists_00bf3f70_fragment(const std::string& suffix) {
    if (!supported_ || !supported_string(suffix) || suffix.empty()) return false;
    // 00435c40 compares stored lengths, then CRT case-insensitive C strings.
    if (suffix.size() == last_success_.size() &&
        _stricmp(suffix.c_str(), last_success_.c_str()) == 0) return true;
    if (accept_nonempty_names_) return true;
    std::string path;
    if (!build_path_00bf3970(suffix, path)) return false;
    const bool found = GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
    if (found) last_success_ = suffix;
    else last_success_.clear();
    return found;
}

bool PhysicalDirectory::resolve_00bf0fb0(const std::string& suffix, std::string& output) {
    if (!exists_00bf3f70_fragment(suffix)) return false;
    if (&suffix != &output) output = suffix;
    return true;
}
bool PhysicalDirectory::enumerate_00bf47e0_fragment(const std::string& directory,
    const std::string& extension, std::uint32_t flags,
    std::vector<std::string>& output, std::string& error) const {
    error.clear();
    std::string path;
    if (!supported_string(extension) || !build_path_00bf3970(directory, path)
        || path.empty() || path.size() > static_cast<std::size_t>(INT32_MAX) - 2) {
        error = "Unsupported physical enumeration path or extension.";
        return false;
    }
    if (path.back() != '\\') path += '\\';
    path += '*';
    WIN32_FIND_DATAA data{};
    struct FindHandle {
        HANDLE value;
        ~FindHandle() { if (value != INVALID_HANDLE_VALUE) FindClose(value); }
    } found{FindFirstFileA(path.c_str(), &data)};
    if (found.value == INVALID_HANDLE_VALUE) return true;
    auto result = output;
    do {
        const std::string child = data.cFileName;
        if (child.empty() || child.front() == '.') continue;
        const bool is_directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        if (is_directory) {
            if ((flags & 0xffu) == 0) continue;
        } else {
            if (child.size() <= extension.size()
                || _stricmp(child.c_str() + child.size() - extension.size(), extension.c_str()) != 0
                || _stricmp(child.c_str(), "MidwayDL_Content") == 0) continue;
        }
        std::string logical;
        if (!join_resource_path_00bee520_fragment(directory, child, logical)) {
            error = "Unsupported physical enumeration logical name.";
            return false;
        }
        if (is_directory) {
            if (!enumerate_00bf47e0_fragment(logical, extension, flags, result, error)) return false;
        } else result.push_back(std::move(logical));
    } while (FindNextFileA(found.value, &data));
    output = std::move(result);
    return true;
}
std::shared_ptr<PhysicalDirectory> create_physical_directory_00bf4df0_fragment(
    const std::string& system_path, const std::string& virtual_path) {
    if (!supported_string(system_path) || !supported_string(virtual_path)
        || system_path.empty() || system_path.back() != '\\') return {};
    return std::make_shared<PhysicalDirectory>(system_path,
        _stricmp(virtual_path.c_str(), "persistent_data") == 0);
}
}
