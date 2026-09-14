#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical pending I/O requires MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;
struct NativePhysicalFileDateContext;
struct NativeRetainedMemoryOwnerContext;

// BF476D calls the actual record+30h DWORD target with three stack arguments
// (stream, first8h header, second8h header); the target removes those 12 bytes.
// This explicit source dispatcher resolves that target without calling game
// addresses. Headers remain borrowed from the current queue during the call.
class NativePhysicalPendingCompletionDispatch {
public:
    virtual ~NativePhysicalPendingCompletionDispatch() = default;
    virtual void invoke_00bf476d(std::uint32_t native_callback,
        void* actual_stream, const void* actual_first_name,
        const void* actual_second_name) = 0;
};

struct NativePhysicalPendingIoContext {
    NativePhysicalFileDateContext& physical;
    NativeRetainedMemoryOwnerContext& memory;
    NativePhysicalPendingCompletionDispatch& completion;
};

// BF3D00 complete: ECX destination, stack source, EAX destination, RET4.
// Assign established scalar words and both actual names, preserving +14/+34.
// No rollback on second-name failure; source/current headers are reread.
void* assign_native_physical_pending_record_00bf3d00(void* actual_destination,
    const void* actual_source, NativeStringStorage&);

// BF41C0 complete: ECX queue, stack source, RET4. Actual queue is the existing
// pointer/count/capacity header, stride38h. Grow only at count==capacity, then
// copy-construct at current count and increment current count. No rollback.
void append_native_physical_pending_record_00bf41c0(void* actual_queue,
    const void* actual_source, NativeStringStorage&);

// BF4240 complete: ECX queue, stack signed index, RET4. Shift by assignment,
// destroy current last names, then decrement count; capacity stays unchanged.
// Native signed comparisons/wrapping arithmetic are retained without guards.
void erase_native_physical_pending_record_00bf4240(void* actual_queue,
    std::int32_t index, NativeStringStorage&);

// BF43B0 complete body over actual provider+14h pending storage. Native ECX
// provider, stack first/second/callback/flags, AL acceptance, RET10h. Requires
// current D69168+1C/BF3970 path dispatch and uses real Win32 unbuffered,
// overlapped I/O. GetFileSizeEx false with GetLastError()==0 still proceeds.
// Allocation/read rounding uses wrapping LOW DWORD; no size/count guard.
// Immediate success and ERROR_IO_PENDING both append AFTER starting the read.
// Read failure frees ALIGNED +10h (possibly interior), leaks OVERLAPPED, closes
// handle. Native exceptional cleanup releases only temporary names. These
// original failure behaviors are preserved, not a safe owning host interface.
bool submit_native_physical_pending_io_00bf43b0(void* actual_provider,
    const void* actual_first_name, const void* actual_second_name,
    std::uint32_t native_callback, std::uint32_t flags,
    NativePhysicalPendingIoContext&);

// BF46B0 complete: native ECX provider, RET. Skip Internal==103h; otherwise
// GetOverlappedResult(FALSE), ignoring actual count. Success/type1 creates
// independent actual BEFA40 memory stream, dispatches callback, then decrements
// its actual reference. Reload queue after callback, free ORIGINAL +Ch then
// OVERLAPPED, clear +4h, close handle, erase. Errors/type!=1 skip callback.
// Recheck same index after erase against live count (including callback appends).
// No exception cleanup, recursive-pump guard, implicit cancellation or drain.
void pump_native_physical_pending_io_00bf46b0(void* actual_provider,
    NativePhysicalPendingIoContext&);

// Explicit C++ services, not original binary/FH3 ABI replacements. Production
// strings use the existing ActualNativeStringPoolStorage. Original imported
// Win32/CRT services are host boundaries. See NATIVE_PHYSICAL_PENDING_IO_BK.md.
} // namespace bsp
