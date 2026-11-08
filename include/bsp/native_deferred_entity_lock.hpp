#pragma once

#include <cstdint>

namespace bsp {
struct TrackedCriticalSection;

// Sole process publication corresponding to Original F899EC. The existing
// 00924990 body constructs/registers it using the application's SAME 1090AA0
// manager. Cells and deletion services survive the registered-owner drain.
void*& process_native_deferred_entity_lock_publication() noexcept;
void retire_native_deferred_entity_lock(void* captured_owner, std::uint32_t flags);

// Source counterpart of 00926EA2..00926ED0 / 00926F78..00926F88, used by
// the real host hit producer. Captures owner+4 once; enter/depth++ before
// enqueue, depth--/leave on normal return and C++ exception. No hit-record
// conversion, replacement manager, or private teardown domain is introduced.
class NativeDeferredEntityEventQueueGuard final {
public:
    NativeDeferredEntityEventQueueGuard();
    ~NativeDeferredEntityEventQueueGuard();
    NativeDeferredEntityEventQueueGuard(const NativeDeferredEntityEventQueueGuard&) = delete;
    NativeDeferredEntityEventQueueGuard& operator=(const NativeDeferredEntityEventQueueGuard&) = delete;
private:
    TrackedCriticalSection* captured_{};
};
} // namespace bsp
