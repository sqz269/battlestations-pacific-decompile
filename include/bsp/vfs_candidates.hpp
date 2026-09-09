#pragma once
#include <functional>
#include <string>
#include <vector>

namespace bsp {
struct VfsExtensionPrefix {
    std::string extension;
    std::string prefix;
};
struct VfsCandidateGroup {
    std::vector<std::string> extensions;
    std::vector<std::string> prefixes;
};
// Explicit snapshots of native +54 tree iteration and +60 list iteration.
// Preserve equal-key prefix order, group order and each group's list order.
struct VfsCandidateRegistrations {
    std::vector<VfsExtensionPrefix> extension_prefixes;
    std::vector<VfsCandidateGroup> groups;
};
using VfsDirectResolver = std::function<bool(const std::string&, std::string&)>;
using VfsCandidateExists = std::function<bool(const std::string&)>;

// Native ECX manager, mutable string stack argument, AL result, RET4.
// Normalizes ASCII case/slashes but does not trim. First direct resolution can
// rewrite output; candidate existence copies the constructed spelling unchanged.
// Host domain: ASCII, no embedded NUL, lengths <= INT32_MAX. Invalid inputs or
// missing callbacks fail before mutation. Supplied registrations must remain
// stable during callbacks. Allocation/provider exceptions propagate.
bool resolve_resource_candidates_00bddc80(
    std::string& name, const VfsCandidateRegistrations& registrations,
    const VfsDirectResolver& direct, const VfsCandidateExists& exists);
}
