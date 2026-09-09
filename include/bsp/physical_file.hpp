#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>

namespace bsp {
// Typed owning projection of the native physical stream, not its vtable/pool ABI.
// No VFS normalization, mount selection, tracking, or native error callbacks.
class PhysicalFile {
public:
    PhysicalFile() noexcept = default; // 00bf50d0 handle/position/size initialization
    ~PhysicalFile();
    PhysicalFile(const PhysicalFile&) = delete;
    PhysicalFile& operator=(const PhysicalFile&) = delete;

    // Only flags=2 of 00bf52a0. ANSI path; nullptr uses the native empty fallback.
    // Requires a closed owner. If size query fails, returns false but keeps the
    // successfully opened handle until close/destruction; inspect valid().
    bool open_read_only_00bf52a0_fragment(const char* path, DWORD& error) noexcept;
    bool valid_00bf5020() const noexcept;
    std::uint64_t size_00bf4f90() const noexcept;
    std::uint64_t position() const noexcept { return position_; }
    bool seek_00bf4f20(std::int64_t distance, DWORD origin, DWORD& error) noexcept;
    // One synchronous read. actual and cached position update even on failure.
    // False exposes the Win32 failure instead of invoking the native callback.
    bool read_00bf5030(void* destination, std::uint32_t requested,
        std::uint32_t& actual, DWORD& error) noexcept;
    // Handle-release part of 00bf5090; preserves cached size and position.
    // Already closed is a host no-op; no native allocator/pool operations.
    bool close_00bf5090_fragment(DWORD& error) noexcept;

private:
    HANDLE handle_{INVALID_HANDLE_VALUE};
    std::uint64_t position_{};
    std::uint64_t size_{};
};
}
