#pragma once
#include "bsp/physical_directory.hpp"
#include "bsp/vfs_mounts.hpp"

namespace bsp {
using VfsFileDate = std::array<std::uint32_t, 5>;
class FileStore;

// Native metadata visitor +8 at 00BD9F00; any nonzero word stops traversal.
bool has_vfs_file_date_00bd9f00(const VfsFileDate&) noexcept;
// ECX manager, hidden output pointer and name pointer, EAX output, RET8.
// Normalized copy; same ordered mounts as existing VFS. No open-only aliases.
// Missing provider support and unsupported names are explicit host failures.
VfsFileDate query_vfs_file_date_00bdd340(VfsMountContext&, const std::string&);
// Physical provider +20h at 00BF3A80, ECX provider, output/name, RET8.
// native VFS+78 disables dates. Uses actual last-write UTC FileTime.
// Covers the existing PhysicalDirectory empty-index projection. A failed
// FileTime conversion is rejected; native would consume unspecified words.
VfsFileDate query_physical_file_date_00bf3a80(PhysicalDirectory&,
    const std::string&, bool dates_disabled);
// FileStore returns all FFFFFFFF for a matching name, otherwise all zero.
VfsFileDate query_file_store_date_00be5c80(const FileStore&, const std::string&);
// MPKG +20 always returns zero; it does not probe an archive entry.
VfsFileDate empty_package_file_date_00bb9d50() noexcept;
}
