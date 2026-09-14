#include "bsp/vfs_file_date.hpp"
#include "bsp/resource_path.hpp"
#include "bsp/file_store.hpp"
#include "bsp/vfs_native_access.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
bool has_vfs_file_date_00bd9f00(const VfsFileDate& date) noexcept {
    for (auto word : date) if (word) return true;
    return false;
}

VfsFileDate query_vfs_file_date_00bdd340(VfsMountContext& context,
    const std::string& name) {
    if (context.native_access) return context.native_access->file_date(name);
    if (name.find('\0') != std::string::npos)
        throw std::invalid_argument("Unsupported VFS file-date name");
    std::string normalized = name;
    if (!normalize_resource_path_00bee690(normalized))
        throw std::invalid_argument("Unsupported VFS file-date name");
    context.error_code = -1;
    VfsFileDate result{};
    for (const auto& mount : context.mounts) {
        if (mount.prefix.size() > INT32_MAX || mount.prefix.find('\0') != std::string::npos)
            throw std::invalid_argument("Unsupported VFS file-date mount prefix");
        std::string suffix;
        if (mount.prefix.empty()) suffix = normalized;
        else {
            if (mount.prefix.size() >= normalized.size() ||
                normalized[mount.prefix.size()] != '/' ||
                _stricmp(normalized.substr(0, mount.prefix.size()).c_str(),
                    mount.prefix.c_str()) != 0) continue;
            suffix = normalized.substr(mount.prefix.size() + 1);
        }
        if (!mount.file_date)
            throw std::logic_error("VFS provider has no recovered file-date operation");
        // 00BD9E80 copies all five returned words, including an all-zero miss.
        result = mount.file_date(suffix, context.file_dates_disabled);
        if (has_vfs_file_date_00bd9f00(result)) break;
    }
    return result;
}

VfsFileDate query_physical_file_date_00bf3a80(PhysicalDirectory& provider,
    const std::string& name, bool dates_disabled) {
    if (dates_disabled) return {};
    if (!provider.supported())
        throw std::invalid_argument("Unsupported physical file-date provider");
    std::string path = name;
    // Native virtual +18 changes the copy only after +10 reports existence.
    if (provider.exists_00bf3f70_fragment(name) &&
        !provider.build_path_00bf3970(name, path))
        throw std::invalid_argument("Unsupported physical file-date path");
    if (path.size() > INT32_MAX || path.find('\0') != std::string::npos)
        throw std::invalid_argument("Unsupported physical file-date name");
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attributes)) {
        (void)GetLastError();
        // Native diagnostic 004254B0 is a verified bare RET.
        return {};
    }
    SYSTEMTIME time;
    if (!FileTimeToSystemTime(&attributes.ftLastWriteTime, &time))
        throw std::runtime_error("FileTimeToSystemTime failed for physical file date");
    return {time.wYear, time.wMonth, time.wDay,
        time.wSecond + (time.wMinute + time.wHour * 60U) * 60U,
        time.wMilliseconds};
}
VfsFileDate query_file_store_date_00be5c80(const FileStore& store,
    const std::string& name) {
    const auto word = store.exists_00be5c00(name) ? 0xffffffffU : 0U;
    return {word, word, word, word, word};
}
VfsFileDate empty_package_file_date_00bb9d50() noexcept { return {}; }
}
