#include "bsp/mounted_streams.hpp"
#include <utility>

namespace bsp {
VfsMount bind_physical_directory_fragment(std::string prefix,
    const std::shared_ptr<PhysicalDirectory>& directory) {
    if (!directory || !directory->supported()) return {std::move(prefix), {}, {}, {}};
    return {std::move(prefix),
        [directory](const std::string& name) { return directory->exists_00bf3f70_fragment(name); },
        [directory](const std::string& name, std::string& output) { return directory->resolve_00bf0fb0(name, output); },
        [directory](const std::string& name) {
            VfsMemoryOpen result;
            std::string path;
            if (!directory->build_path_00bf3970(name, path)) {
                result.error = "Unsupported physical provider path.";
                return result;
            }
            PhysicalFile file;
            DWORD failure{};
            const bool opened = file.open_read_only_00bf52a0_fragment(path.c_str(), failure);
            result.provider_opened = file.valid_00bf5020();
            if (!opened) {
                result.error = "Cannot open " + name + ": Win32 " + std::to_string(failure);
                return result;
            }
            auto memory = std::make_shared<MemoryStream>();
            if (!memory_stream_from_physical_00bef750_fragment(file, *memory, failure)) {
                result.error = "Cannot buffer " + name + ": Win32 " + std::to_string(failure);
                return result;
            }
            result.stream = std::move(memory);
            return result;
        }};
}
VfsMount bind_file_store_fragment(std::string prefix, const std::shared_ptr<FileStore>& store) {
    if (!store) return {std::move(prefix), {}, {}, {}};
    return {std::move(prefix),
        [store](const std::string& name) { return store->exists_00be5c00(name); },
        [store](const std::string& name, std::string& output) { return store->resolve_00bf0fb0(name, output); },
        [store](const std::string& name) {
            auto stream = store->open_00be5fa0(name, 2);
            return VfsMemoryOpen{stream != nullptr, std::move(stream), {}};
        }};
}
bool cache_resource_00be7ab0_fragment(FileStore& store, VfsMountContext& mounts,
    const std::string& name, std::string& error) {
    auto opened = open_resource_memory_00bdf310_fragment(mounts, name);
    if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
        error = opened.error.empty() ? "Cache source unavailable or incomplete: " + name : std::move(opened.error);
        return false;
    }
    auto source = std::make_shared<MemoryStream>(opened.stream->clone_reset_00bef6d0());
    if (store.add_file_00be7760(name, source) == FileStoreInsertResult::invalid) {
        error = "Unsupported cache entry: " + name;
        return false;
    }
    error.clear();
    return true;
}
}
