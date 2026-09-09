#pragma once
#include "bsp/vfs_mounts.hpp"
#include <cstdint>

namespace bsp {
struct VfsMountRegistration {
    std::int32_t priority = 0;
    VfsMount mount;
    // Native copies this byte alongside a raw provider pointer. Provider
    // destruction/unregistration is outside this insertion fragment.
    std::uint8_t native_ownership_flag = 0;
};
// 00bee390 canonicalization then00584110 trailing-slash trim. ASCII/NUL-free
// host domain, input <=INT32_MAX; output unchanged for unsupported input.
// Does not use host filesystem canonicalization or strip spaces.
bool prepare_mount_prefix_00be1740_fragment(const std::string&, std::string& output);
// ECX manager; native stack(provider,prefix,priority,ownership), RET10h.
// Supplied records must already be descending priority. Inserts after all
// existing equal priorities. Callbacks carry host provider lifetime; no native
// AddRef or inferred deleter is introduced. Allocation exceptions propagate.
bool register_ordered_mount_00be1740_fragment(
    std::vector<VfsMountRegistration>&, VfsMount, std::int32_t priority,
    std::uint8_t native_ownership_flag);
// Copy registration order/callback owners into the traversal view.
VfsMountContext make_registered_mount_context_fragment(
    const std::vector<VfsMountRegistration>&);
}
