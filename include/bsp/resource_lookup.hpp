#pragma once
#include "bsp/vfs_mounts.hpp"
#include "bsp/vfs_candidates.hpp"

namespace bsp {
// 00bdf4c0 ECX manager, stack mutable name, AL result, RET4. Normalizes the
// caller's name before direct/candidate lookup; name can change on failure.
// Projection supplies mount/search state and omits successful-search logging
// 00bdeb40. Native mount allocation, archives and error callbacks are separate.
bool resolve_existing_resource_00bdf4c0_fragment(
    VfsMountContext&, const VfsCandidateRegistrations&, std::string& name);
}
