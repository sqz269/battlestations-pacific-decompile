#pragma once
#include "bsp/vfs_mounts.hpp"
#include "bsp/physical_directory.hpp"

namespace bsp {
struct VfsPendingPumpVisit {
    std::size_t mount_index{};
    std::string prefix;
    PhysicalReadPumpReport reads;
    bool invoked{};
    bool succeeded{};
    DWORD error{};
};
struct VfsPendingPumpReport {
    std::vector<VfsPendingPumpVisit> visits;
    std::size_t completed{}, succeeded{}, failed{};
    // Sum of per-visit remaining counts, NOT unique outstanding requests: the
    // same provider may occur at multiple mount records and is pumped each time.
    std::size_t remaining_observations{};
    DWORD first_error{};
};

//00bdda10: ECX manager; first name,second name,callback,flags; AL; RET10h.
// No normalization, candidate resolution or alias substitution. Traverse by
// second_name and send BOTH full names unchanged; mount suffix is not used.
// False/decline continues traversal, true means provider acceptance only.
// Native00bdc100 owns both visitor names; providers must copy before returning.
// Host rejects NUL/oversized strings, missing operations and empty callback.
bool open_resource_pending_00bdda10_fragment(VfsMountContext&,
    const std::string& first_name, const std::string& second_name,
    PhysicalReadCallback, std::uint32_t flags, DWORD& error);

//00bdb0b0: ECX manager; RET. Visit every mount record in supplied order,
// including repeated provider identities. No prefix filter or deduplication.
// All access is caller-serialized; no recursive pump, mount mutation or owner
// destruction during callbacks. Callback submission is supported. No thread
// affinity. Per-visit reports survive provider errors and callback exceptions;
// exceptions propagate after recording completed work and restoring the guard.
// Reentrant calls return ERROR_BUSY without changing the active report.
bool pump_pending_resources_00bdb0b0_fragment(VfsMountContext&,
    VfsPendingPumpReport&, DWORD& error);

// Host shutdown operation: stop submissions at every provider, without any
// pump, wait, cancellation or pending-map clearing. Drain explicitly afterward.
// A failure leaves any earlier providers stopped; there is no rollback.
bool begin_pending_shutdown_fragment(VfsMountContext&, DWORD& error);

// Bind actual00bf43b0/00bf46b0 behavior to an existing physical mount. The
// first full name goes through00bf3970; no matched-prefix stripping occurs.
// Captures both owners. They must remain registered/alive until queues drain.
// Invalid owners return false without changing the mount. Exceptions propagate.
bool bind_physical_pending_fragment(VfsMount&,
    const std::shared_ptr<PhysicalDirectory>&,
    const std::shared_ptr<PhysicalPendingReads>&);
}
