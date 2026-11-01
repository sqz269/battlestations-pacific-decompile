#pragma once
#include <cstdint>
#include <exception>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeVfsNameResolutionContext;
class NativeVfsNameResolutionAcquired;
class NativeVfsRuntimeBindings;


enum class NativeParticleTextReadPhase { fresh, entered, complete, failed };
// Retained Source-only evidence from the REAL shared AF5850 body. No public
// setters/adoption or Native cleanup: only that loader can populate the actual
// malloc extent/pointer and read HOME/result. Frame stays alive with its whole
// allocator/VFS/stream/buffer/resolution graph after failure or fit rejection.
struct NativeParticleTextBufferReadAccess;
class NativeParticleTextBufferReadFrame final {
public:
    NativeParticleTextBufferReadFrame() {}
    NativeParticleTextBufferReadFrame(const NativeParticleTextBufferReadFrame&) = delete;
    NativeParticleTextBufferReadFrame& operator=(const NativeParticleTextBufferReadFrame&) = delete;
    bool normal_returned() const noexcept { return phase_ == NativeParticleTextReadPhase::complete; }
    NativeParticleTextReadPhase phase() const noexcept { return phase_; }
    const void* actual_buffer() const noexcept { return text_; }
    const void* actual_allocation() const noexcept { return allocation_; }
    std::uint32_t allocation_extent() const noexcept { return allocation_bytes_; }
    bool allocation_returned() const noexcept { return allocation_returned_; }
    bool read_returned() const noexcept { return read_returned_; }
    std::uint32_t completed_read_count() const noexcept { return read_count_at_return_; }
    std::uint32_t requested_read_count() const noexcept { return read_requested_; }
    bool allocation_released() const noexcept {
        return mismatch_free_returned_ && mismatch_free_pointer_ == allocation_;
    }
    std::exception_ptr failure() const noexcept { return failure_; }
private:
    friend struct NativeParticleTextBufferReadAccess;
    NativeParticleTextReadPhase phase_{NativeParticleTextReadPhase::fresh};
    void* text_{};
    void* stream_{};
    void* allocation_{};
    void* mismatch_free_pointer_{};
    std::uint32_t allocation_bytes_{}, read_requested_{}, read_count_at_return_{}, returned_count_{};
    std::uint32_t actual_count_home_; // Initialized from incoming name bits before Native effects.
    bool allocation_entered_{}, allocation_returned_{}, read_entered_{}, read_returned_{};
    bool mismatch_free_entered_{}, mismatch_free_returned_{};
    std::exception_ptr failure_;
};

// Complete AF5850[221]. Native ECX actual1Ch TextBuffer; stacked nullable
// C-string; EAX actual-count/zero; RET4. This new C++ API borrows the SAME
// actual VFS publication, concrete VFS services and raw string-pool cells.
// name_resolution's lookup/logging strings must be the existing VFS owner
// service over those same cells. Its existing identity checks still apply.
// No new VFS owner, stream, table, callback or private publication is supplied.
//
// The caller supplies a FRESH invocation and retains it AND actual_text_buffer
// (including its name header/data) after failed resolution. The existing
// resolver deliberately terminates if an incomplete/failed frame is destroyed.
// This function creates no local acquired frame and does not change that policy.
// Other provider failures propagate with the original loader's lack of cleanup.
//
// Resolve the current name; ignore AL; reload publication for open(flags32h).
// Consume only low32 virtual30 length. Allocate that many bytes, make ONE
// current virtual24 read, with actual-count initialized from incoming name
// pointer bits. A mismatch frees CURRENT text+14 and clears it, returning0
// WITHOUT releasing the stream. Success decrements once and dispatches the
// CURRENT terminal only on zero, then reloads actual-count for the result.
// No null-stream recovery, read retry, length guard or rollback is added.
std::uint32_t load_native_particle_text_buffer_00af5850(
    void* actual_text_buffer, const char* name,
    void* volatile& actual_vfs_publication_0109ceec,
    NativeVfsRuntimeBindings& vfs,
    NativeVfsNameResolutionContext& name_resolution,
    NativeVfsNameResolutionAcquired& invocation,
    NativeStringRawPoolContext& strings);

// Same shared whole normal body and same Native call/read/free schedule.
// A fresh externally retained record transports the actual-count DWORD HOME;
// the legacy overload keeps its local HOME. Neither interface claims Original
// private-stack/HOME aliases or physical ABI. No extra VFS resolve/read is made.
std::uint32_t load_native_particle_text_buffer_00af5850(
    void* actual_text_buffer, const char* name,
    void* volatile& actual_vfs_publication_0109ceec,
    NativeVfsRuntimeBindings&, NativeVfsNameResolutionContext&,
    NativeVfsNameResolutionAcquired&, NativeStringRawPoolContext&,
    NativeParticleTextBufferReadFrame&);

// Native table words must remain readable at their identity addresses, as
// required by the existing concrete VFS bindings. Their supported profiles
// and source exception/CRT boundaries are unchanged. GameNativeVfsRuntime can
// expose these SAME services through borrow_raw_services(); application callers
// still retain the actual text/resolution frames and supply the same raw cells.
// Original argument-slot aliases, register ABI, hardware SEH and gameplay
// equivalence are not provided by this source interface.
} // namespace bsp
