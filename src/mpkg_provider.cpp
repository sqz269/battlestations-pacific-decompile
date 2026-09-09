#include "bsp/mpkg_provider.hpp"
#include <cstring>
#include <limits>
#include <utility>

namespace bsp {
namespace {
class ReopenedMemorySource final : public InflateSource {
public:
    explicit ReopenedMemorySource(const MemoryStream& memory) noexcept
        : cursor_(memory.clone_reset_00bef6d0()) {}
    bool seek_absolute(std::uint32_t offset) noexcept override {
        return cursor_.seek_00bef540(offset, 0);
    }
    bool read(void* destination, std::uint32_t requested,
        std::uint32_t& actual) noexcept override {
        actual = 0;
        return cursor_.read_00bef590(destination, requested, &actual);
    }
private:
    MemoryStream cursor_;
};

bool valid_source(const VfsMemoryOpen& opened, std::string& error) {
    if (!opened.provider_opened || !opened.stream) {
        error = opened.error.empty() ? "MPKG logical source was not opened or buffered." : opened.error;
        return false;
    }
    const auto size = opened.stream->size_00bef600();
    if (!opened.stream->fully_initialized() || size <= 0 ||
        size > (std::numeric_limits<std::int32_t>::max)()) {
        error = opened.error.empty() ? "MPKG logical source requires complete backing of 1..INT32_MAX bytes."
            : opened.error;
        return false;
    }
    return true;
}
}

MpkgCreateStatus create_mpkg_archive_00bb9d90_fragment(
    const std::string& system_path, MpkgLogicalOpen open_current_vfs,
    std::shared_ptr<MpkgArchive>& output, std::string& error) {
    error.clear();
    if (system_path.size() > static_cast<std::size_t>(
        (std::numeric_limits<std::int32_t>::max)()) || system_path.find('\0') != std::string::npos) {
        error = "MPKG factory requires a bounded NUL-free logical path.";
        return MpkgCreateStatus::failed;
    }
    if (system_path.size() <= 5 ||
        _stricmp(system_path.c_str() + system_path.size() - 5, ".mpkg") != 0)
        return MpkgCreateStatus::declined;
    if (!open_current_vfs) {
        error = "MPKG factory requires a current-VFS logical opener.";
        return MpkgCreateStatus::failed;
    }
    auto initial = open_current_vfs(system_path, 2);
    if (!valid_source(initial, error)) return MpkgCreateStatus::failed;
    auto archive = std::make_shared<MpkgArchive>();
    // Preserve the logical path and callable state after the initial open.
    // Each later invocation queries its live VFS again and owns a new cursor.
    MpkgReopenSource reopen = [path = system_path, opener = std::move(open_current_vfs)]
        (std::string& source_error) -> std::shared_ptr<InflateSource> {
        source_error.clear();
        auto opened = opener(path, 2);
        if (!valid_source(opened, source_error)) return {};
        return std::make_shared<ReopenedMemorySource>(*opened.stream);
    };
    if (!archive->load_00bb9920_fragment(*initial.stream, std::move(reopen), error))
        return MpkgCreateStatus::failed;
    output = std::move(archive);
    return MpkgCreateStatus::loaded;
}

VfsMount bind_mpkg_archive_fragment(std::string prefix,
    const std::shared_ptr<MpkgArchive>& archive) {
    VfsMount mount;
    mount.prefix = std::move(prefix);
    if (!archive) return mount;
    mount.exists = [archive](const std::string& name) {
        return archive->contains_00bb8e00(name);
    };
    mount.resolve = [archive](const std::string& name, std::string& output) {
        if (!archive->contains_00bb8e00(name)) return false;
        output = name;
        return true;
    };
    mount.open_read_only = [archive](const std::string& name, std::uint32_t flags) {
        if (flags != 2 && flags != 0x32)
            return VfsMemoryOpen{false, {}, "Unsupported MPKG read flags."};
        if (!archive->contains_00bb8e00(name)) return VfsMemoryOpen{};
        // Once selected, a guarded host failure must not substitute another
        // provider's bytes. Native malformed-entry behavior is not replicated.
        VfsMemoryOpen result{true, {}, {}};
        auto stream = std::make_shared<MemoryStream>();
        if (archive->open_entry_00bb8d60_fragment(name, flags, *stream, result.error))
            result.stream = std::move(stream);
        return result;
    };
    mount.enumerate = [archive](const std::string& directory, const std::string& extension,
        std::uint32_t flags, std::vector<std::string>& output, std::string& error) {
        return archive->enumerate_00bb97b0_fragment(directory, extension, flags, output, error);
    };
    return mount;
}
}
