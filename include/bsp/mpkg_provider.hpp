#pragma once
#include "bsp/mpkg_archive.hpp"
#include "bsp/vfs_mounts.hpp"

namespace bsp {
// Invoke the CURRENT VFS on every call. Capture its live owner weakly (or use
// another lifetime-safe current-context binding), not a snapshot/physical file.
// Callbacks must not mutate the mount collection during traversal.
using MpkgLogicalOpen = std::function<VfsMemoryOpen(
    const std::string& logical_path, std::uint32_t flags)>;
enum class MpkgCreateStatus { declined, loaded, failed };

//00bb9d90: ECX factory, system/virtual paths stack, EAX provider, RET8.
// Virtual path is unused natively, so this host API omits it. Accepts length>5
// and case-insensitive final .mpkg. Constructor00bb9920 opens the unchanged
// supplied logical path with flags2 through open_current_vfs; large reopens do
// the same later. Decline/failure preserve output. Exceptions propagate.
// Does not register a mount or assign priority/device/ownership metadata.
MpkgCreateStatus create_mpkg_archive_00bb9d90_fragment(
    const std::string& system_path, MpkgLogicalOpen open_current_vfs,
    std::shared_ptr<MpkgArchive>& output, std::string& error);

// Captures archive ownership for open/exists/resolve/enumerate. Matched-entry
// materialization failure is terminal, preserving provider diagnostics; absence
// permits fallback. Host read flags are exactly2/0x32. No native provider ABI.
VfsMount bind_mpkg_archive_fragment(std::string prefix,
    const std::shared_ptr<MpkgArchive>& archive);
}
