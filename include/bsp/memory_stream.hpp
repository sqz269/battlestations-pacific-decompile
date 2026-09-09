#pragma once
#include "bsp/physical_file.hpp"
#include <memory>

namespace bsp {
struct MemoryStreamBacking;

// Shared native-style backing with independent cursors; not the original ABI.
// Supported backing lengths are 1..INT32_MAX. No allocator counters/pool.
class MemoryStream {
public:
    MemoryStream() noexcept = default;
    ~MemoryStream() = default;
    MemoryStream(MemoryStream&&) noexcept = default;
    MemoryStream& operator=(MemoryStream&&) noexcept = default;
    MemoryStream(const MemoryStream&) = delete;
    MemoryStream& operator=(const MemoryStream&) = delete;

    bool has_backing() const noexcept { return backing_ != nullptr; }
    // New retained owner; cursor resets to zero, matching 00bef6d0.
    MemoryStream clone_reset_00bef6d0() const noexcept;
    std::int64_t size_00bef600() const noexcept;
    std::int64_t position_00bef580() const noexcept;
    // Base pointer, independent of cursor. A short file read leaves an
    // uninitialized tail: consumers of the whole extent must first check
    // fully_initialized(). Neither length nor contents are silently repaired.
    const std::uint8_t* data_00bef610() const noexcept;
    std::uint32_t initialized_size() const noexcept;
    bool fully_initialized() const noexcept;
    // Ignores offset high DWORD. Origins 0/1/other mean start/current/end.
    // Host bounds failure preserves cursor; native does not check bounds.
    bool seek_00bef540(std::int64_t offset, std::uint32_t origin) noexcept;
    // Unsigned remaining clamp, special 2/4-byte copy, optional actual output.
    // Host false for missing backing, invalid destination or uninitialized tail;
    // that failure leaves cursor/destination/actual unchanged.
    bool read_00bef590(void* destination, std::uint32_t requested,
        std::uint32_t* actual = nullptr) noexcept;

private:
    friend bool memory_stream_from_physical_00bef750_fragment(
        PhysicalFile&, MemoryStream&, DWORD&) noexcept;
    std::shared_ptr<MemoryStreamBacking> backing_;
    std::uint32_t cursor_{};
};

// Seek to zero, allocate uninitialized backing, make exactly one read, preserve
// requested backing size independently of actual count. Short reads succeed.
// Read failure returns false/error but still supplies backing and actual extent;
// pre-read failures leave output unchanged. Does not close the source file.
bool memory_stream_from_physical_00bef750_fragment(PhysicalFile& source,
    MemoryStream& output, DWORD& error) noexcept;
}
