#include "bsp/file_store.hpp"
#include "bsp/resource_path.hpp"
#include <cstring>
#include <limits>
#include <utility>

namespace bsp {
namespace {
bool supported_name(const std::string& value) noexcept {
    return value.size() <= static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())
        && value.find('\0') == std::string::npos;
}
}

bool FileStore::NameLess::operator()(const std::string& left,
    const std::string& right) const noexcept {
    //00443d00/00be54d0 use empty-string gates, then _stricmp, no length tie-break.
    if (left.empty()) return !right.empty();
    if (right.empty()) return false;
    return _stricmp(left.c_str(), right.c_str()) < 0;
}

FileStoreInsertResult FileStore::add_file_00be7760(const std::string& name,
    const std::shared_ptr<MemoryStream>& source) {
    if (!supported_name(name)) return FileStoreInsertResult::invalid;
    auto normalized = name;
    if (!normalize_resource_path_00bee690(normalized)) return FileStoreInsertResult::invalid;
    // Native checks duplicates before dereferencing the incoming stream.
    if (files_.find(normalized) != files_.end()) return FileStoreInsertResult::duplicate;
    if (!source || !source->has_backing()) return FileStoreInsertResult::invalid;
    files_.emplace(std::move(normalized), source);
    return FileStoreInsertResult::inserted;
}

bool FileStore::exists_00be5c00(const std::string& name) const {
    return supported_name(name) && files_.find(name) != files_.end();
}

std::shared_ptr<MemoryStream> FileStore::open_00be5fa0(const std::string& name,
    std::uint32_t flags) const {
    if ((flags & 1u) || !supported_name(name)) return {};
    const auto entry = files_.find(name);
    if (entry == files_.end()) return {};
    return std::make_shared<MemoryStream>(entry->second->clone_reset_00bef6d0());
}

bool FileStore::resolve_00bf0fb0(const std::string& name, std::string& output) const {
    if (!exists_00be5c00(name)) return false;
    if (&name != &output) output = name;
    return true;
}
bool FileStore::enumerate_00be6480(const std::string& directory,
    const std::string& extension, std::uint32_t flags,
    std::vector<std::string>& output, std::string& error) const {
    (void)flags;
    error.clear();
    if (!supported_name(directory) || !supported_name(extension)) {
        error = "Unsupported FileStore enumeration query.";
        return false;
    }
    auto result = output;
    for (const auto& entry : files_) {
        const auto& name = entry.first;
        if (name.find(directory) != 0) continue;
        const auto found = name.find(extension);
        const auto index = found == std::string::npos ? UINT32_MAX
            : static_cast<std::uint32_t>(found);
        const auto suffix = static_cast<std::uint32_t>(name.size())
            - static_cast<std::uint32_t>(extension.size());
        if (index == suffix) result.push_back(name);
    }
    output = std::move(result);
    return true;
}
}
