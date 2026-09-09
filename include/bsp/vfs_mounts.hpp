#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace bsp {
struct VfsMount {
    std::string prefix;
    // Provider +10h and +24h respectively. Resolve returns a provider-relative
    // logical name, not a physical filename. Callbacks must not mutate mounts.
    std::function<bool(const std::string&)> exists;
    std::function<bool(const std::string&, std::string&)> resolve;
};
struct VfsMountContext {
    // Supplied native iteration order; insertion/priority is not reconstructed.
    std::vector<VfsMount> mounts;
    std::int32_t error_code = -1; // Native manager +18h, reset before traversal.
};
// Typed fragments of ECX-manager routines, RET4 / RET8 respectively.
// Normalize a copy, traverse 00bdd0a0, stop at first successful provider.
// No search candidates or open-only alias substitution occurs at this layer.
// Nonempty mount prefixes require a following slash and match via _stricmp.
// Host rejects embedded NUL/oversized strings and missing callbacks before
// traversal. Resolve preserves output on failure, including input/output alias.
bool exists_resource_00bdd440_fragment(VfsMountContext&, const std::string&);
bool direct_resolve_resource_00bdd6e0_fragment(
    VfsMountContext&, const std::string&, std::string& output);
}
