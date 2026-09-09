#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bsp {
class MemoryStream;
struct VfsMemoryOpen {
    // Set once the underlying provider has opened a stream. A subsequent host
    // buffering failure must not make traversal fall through to another mount.
    bool provider_opened = false;
    std::shared_ptr<MemoryStream> stream;
    std::string error;
};
struct VfsAlias {
    std::string from;
    std::string to;
};
struct VfsMount {
    std::string prefix;
    // Provider +10h and +24h respectively. Resolve returns a provider-relative
    // logical name, not a physical filename. Callbacks must not mutate mounts.
    std::function<bool(const std::string&)> exists;
    std::function<bool(const std::string&, std::string&)> resolve;
    // Provider +8 restricted to read-only flags=2, followed by a memory view.
    // The provider adapter reports conversion failure separately from no open.
    std::function<VfsMemoryOpen(const std::string&)> open_read_only;
};
struct VfsMountContext {
    // Supplied native iteration order; vfs_mount_registration builds this view.
    std::vector<VfsMount> mounts;
    std::vector<VfsAlias> aliases; // Native manager +94/+98, first match only.
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
// 00bdca80: first equal-length _stricmp match, one copy, no recursive alias or
// post-copy normalization. False for unsupported strings; no change on failure.
bool apply_resource_alias_00bdca80_fragment(const std::vector<VfsAlias>&, std::string&);
// Read-only fragment of00bdf310/00bda690 plus provider-to-memory adapters.
// Normalize copy, apply one alias, visit mounts, stop at first underlying open.
// Source name untouched. Native tracking/logging/error callbacks are excluded.
VfsMemoryOpen open_resource_memory_00bdf310_fragment(VfsMountContext&, const std::string&);
}
