#pragma once
#include "bsp/vfs_mounts.hpp"
#include "bsp/physical_directory.hpp"
#include "bsp/file_store.hpp"

namespace bsp {
// Host bindings of real provider operations. Captured owners replace native
// intrusive/provider-manager lifetime. Flags=2 only; other open modes excluded.
VfsMount bind_physical_directory_fragment(std::string prefix,
    const std::shared_ptr<PhysicalDirectory>&);
VfsMount bind_file_store_fragment(std::string prefix, const std::shared_ptr<FileStore>&);
//00be7ab0 native ECX store, name/flags stack RET8. This projects flags=2.
// Opens before duplicate insertion is checked. Does not resolve search fallback;
// caller supplies an exact resource name. Rejects failed/incomplete host reads.
bool cache_resource_00be7ab0_fragment(FileStore&, VfsMountContext&,
    const std::string& name, std::string& error);
}
