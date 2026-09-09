#pragma once
#include "bsp/memory_stream.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {
using PhysicalReadCallback = std::function<void(std::shared_ptr<MemoryStream>,
    const std::string& first_name, const std::string& second_name)>;

enum class PhysicalReadOutcome {
    succeeded, io_failed, short_read, invalid_transfer_count, copy_failed,
    callback_threw, close_failed
};

struct PhysicalReadCompletion {
    std::string first_name, second_name;
    std::uint32_t logical_size{}, requested_size{}, transferred_size{};
    bool transferred_size_valid{};
    bool callback_invoked{};
    PhysicalReadOutcome outcome{PhysicalReadOutcome::succeeded};
    DWORD error{}, close_error{};
};

struct PhysicalReadPumpReport {
    std::size_t completed{}, succeeded{}, failed{}, remaining{};
    // An unexpected still-pending GetOverlappedResult failure keeps ownership;
    // it is reported separately rather than counted as a terminal completion.
    std::size_t incomplete_observations{};
    DWORD first_error{};
    std::vector<PhysicalReadCompletion> completions;
};

// Owning Win32 projection of00bf43b0/00bf46b0, not a native provider object.
// Caller serializes all access. No creator-thread affinity is imposed.
// No recursive pump, destruction during callback, or callback mutation of the
// active report. Callback submission is allowed; request/name addresses stay
// stable here even when the queue grows (a safe host lifetime extension).
class PhysicalPendingReads {
public:
    PhysicalPendingReads();
    // HOST PRECONDITION: pending_count()==0 and no active pump. Violation calls
    // std::terminate; destruction never cancels, waits or dispatches callbacks.
    ~PhysicalPendingReads();
    PhysicalPendingReads(const PhysicalPendingReads&) = delete;
    PhysicalPendingReads& operator=(const PhysicalPendingReads&) = delete;
    PhysicalPendingReads(PhysicalPendingReads&&) = delete;
    PhysicalPendingReads& operator=(PhysicalPendingReads&&) = delete;

    // Native ECX provider; two name wrappers,callback,flags stack; AL; RET10h.
    // The parent resolves first_name through PhysicalDirectory and supplies its
    // physical ANSI path explicitly. No path rewrite or mount lookup occurs.
    // Requires no embedded NUL and a callable callback. Native gate: bit0 clear
    // and(flags&0xE)==2; observed2/0x32 both work. Files must be1..INT32_MAX bytes.
    // CreateFile uses OVERLAPPED|NO_BUFFERING, read sharing, OPEN_EXISTING.
    // Staging allocation=size+0x20000; address/read count round up to64KiB.
    // TRUE or ERROR_IO_PENDING from ReadFile is accepted, always queued.
    // False reports a real Win32/host guard error and does not queue a request.
    // Allocation/callback-copy exceptions may propagate before ReadFile starts.
    bool submit_00bf43b0_fragment(std::string_view physical_path,
        std::string_view first_name, std::string_view second_name,
        std::uint32_t flags, PhysicalReadCallback callback, DWORD& error);

    // Native ECX provider; no stack arguments; RET. Nonblocking: Internal103h
    // stays pending; other requests use GetOverlappedResult(wait=FALSE).
    // Success copies the original logical byte count using00befa40, then calls
    // callback(stream,first_name,second_name). Errors have no failure callback.
    // Host guard rejects a short/oversized transferred count before copying.
    // Ordered erase rechecks the same index and current count, so a callback's
    // appended request may complete in this same pump. Stream/name ownership
    // is independent of queue relocation; borrowed names expire at erase.
    // Report resets on entry except a recursive call returns ERROR_BUSY without
    // modifying it. False reports first_error but other terminal work is drained.
    // Callback exceptions: record failure, release the terminal request, restore
    // pump state, then rethrow; later requests remain owned. Allocation failures
    // before callback leave that request queued for retry and propagate.
    bool pump_00bf46b0(PhysicalReadPumpReport&, DWORD& error);

    // Stop accepting new work, including callback submissions; does not pump,
    // cancel, wait or discard. Continue pumping until remaining/pending is zero.
    // Returns ERROR_BUSY during a callback/pump, with no state change.
    bool begin_shutdown(DWORD& error) noexcept;
    bool accepting() const noexcept;
    std::size_t pending_count() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
