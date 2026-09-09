#include "bsp/resource_lookup.hpp"
#include "bsp/resource_path.hpp"

namespace bsp {
bool resolve_existing_resource_00bdf4c0_fragment(VfsMountContext& mounts,
    const VfsCandidateRegistrations& registrations, std::string& name) {
    if (!normalize_resource_path_00bee690(name)) return false;
    return resolve_resource_candidates_00bddc80(name, registrations,
        [&](const std::string& input, std::string& output) {
            return direct_resolve_resource_00bdd6e0_fragment(mounts, input, output);
        }, [&](const std::string& input) {
            return exists_resource_00bdd440_fragment(mounts, input);
        });
}
}
