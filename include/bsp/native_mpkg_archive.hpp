#pragma once

#include <cstdint>

namespace bsp {
struct NativeStoredStreamConversionContext;
struct NativeMpkgDirectoryContext;

// Captured original method entries, separate from the actual raw owners.
// Supply concrete source dispatch; numeric addresses are never executed.
class NativeMpkgArchiveServices {
public:
    virtual ~NativeMpkgArchiveServices() = default;
    virtual void* open_manager_00bb99a0(std::uintptr_t captured_entry,
        void* captured_manager, const void* original_name,
        std::uint32_t flags) = 0;
    virtual void delete_stream_00bb9aae(std::uintptr_t captured_entry,
        void* captured_stream, std::uint32_t flags) = 0;
};

// All services borrow the application's existing owners and publication cells.
// Directory strings and explicit cleanup getters must use the same raw pool.
struct NativeMpkgArchiveContext {
    void* volatile& actual_manager_publication_0109ceec;
    NativeStoredStreamConversionContext& conversion;
    NativeMpkgDirectoryContext& directory;
    NativeMpkgArchiveServices& services;
    const volatile std::uint8_t* actual_xor_key_00e144f0;
};

// Complete BB9920..BB9C0A (747 bytes): original ECX actual34h archive,
// stacked original8h system-name header, EAX owner, RET4. Populate the actual
// path/vector headers, open mode2, convert/decode original stream, then read
// the native end record and directory. Field +0 remains untouched; the end
// record scan publishes the decoded stream's low length DWORD at +10.
// The final partial block uses global-index modulo its short block length;
// replacing it with a conventional per-block reversal changes the byte order.
void* construct_native_mpkg_archive_00bb9920(void* actual_archive,
    const void* original_system_name, NativeMpkgArchiveContext&);

// Complete BB9C10..BB9CA0 (145 bytes): decrement actual decoded stream,
// invoke its current slot0 only at zero, destroy entries/current backing,
// then release the current path. No clearing of stale pointers or owner free.
void destroy_native_mpkg_archive_00bb9c10(
    void* actual_archive, NativeMpkgArchiveContext&);

// New source interfaces, not original stack/FH3/SEH replacements. Native
// states reclaim only the path/vector and a backing allocation whose own
// constructor throws; no extra stream rollback or malformed-input check is
// added. Short reads retain the current scratch DWORD, initially the backing
// pointer. Existing nested string release remains noexcept. Simultaneous
// cleanup exceptions and arbitrary aliases into native stack spills are
// outside this source interface; see the evidence report for fixture scope.
} // namespace bsp
