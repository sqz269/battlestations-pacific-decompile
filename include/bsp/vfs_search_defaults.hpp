#pragma once
#include "bsp/vfs_candidates.hpp"

namespace bsp {
// Texture/shaderfx groups from native startup00738360 only. Group insertion
// prepends; each extension/prefix list appends, suppressing duplicate values.
// Other extension groups and +54 roots are omitted. No mounts/providers/aliases.
// Evidence: docs/VFS_SEARCH_REGISTRATION.md. New typed snapshot, not native ABI.
VfsCandidateRegistrations make_asset_search_registrations_00738360_fragment();
}
