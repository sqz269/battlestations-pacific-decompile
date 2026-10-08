#pragma once

#include "bsp/sound_lifetime_access.hpp"

#include <cstdint>

namespace bsp {
struct NativePhysicalFileDateContext;
class ActualNativeStringPoolStorage;
class NativeRenderBatchLifetime;

// Borrow the application's actual publications and canonical owner services.
// BF42A0 produces a raw10h pool: identity D68EC0, data+4/count+8/capacity+C.
// BF50D0 produces a raw20h stream: identity D691B0, refs+4, HANDLE+8,
// untouched+C, position+10/+14, cached size+18/+1C. No replacement owner/vptr.
// Lifetime shutdown must dispatch D68EC0 to delete_pool below. This context
// neither registers a competing dispatcher nor owns any of its references.
struct NativePhysicalStreamOpenContext {
    NativePhysicalFileDateContext& physical;
    void* volatile& pool_0109dc28;
    SoundLifetimeAccess lifetime;
    NativeRenderBatchLifetime& batch_lifetime;
};

// Original no-argument singleton getter, EAX current publication, RET.
void* native_physical_stream_pool_00bf42a0(NativePhysicalStreamOpenContext&);
// Original ECX raw pool+4 header, EAX stream, RET. Pops the current last cell
// or allocates20h; reinitializes the returned raw storage under the shared lock.
void* acquire_native_physical_stream_00bf3770(void* actual_header,
    NativePhysicalStreamOpenContext&);
// Original ECX raw20h stream; EAX same; RET. Leaves +C untouched.
void* construct_native_physical_stream_00bf50d0(void*) noexcept;

// Complete raw BF50D0: ECX actual live20h backing, unused incoming EDX,
// EAX same receiver, RET. Preserves the two Original numeric profile writes
// CEB130 then D691B0 and leaves +C untouched. No Source-owned table/class,
// virtual lifetime, handle producer or substream-dispatch admission follows.
void* __fastcall raw_construct_native_physical_stream_00bf50d0(
    void* actual_backing, std::uint32_t unused_edx) noexcept;

// Original ECX raw header; stack requested DWORD; RET4. Signed comparisons,
// wrapping arithmetic, current header reads, post-free publication retained.
void reserve_native_physical_stream_slots_00bf30c0(void*, std::uint32_t);
void resize_native_physical_stream_slots_00bf3670(void*, std::uint32_t);
// ECX raw header; RET; frees already-dead streams then pointer storage.
void destroy_native_physical_stream_slots_00bf3930(void*);
// ECX raw header; stack dead stream; RET4. Shared current lock; no retain.
void append_native_physical_stream_slot_00bf5190(void*, void*,
    NativePhysicalStreamOpenContext&);
// ECX pool; stack flags; EAX original (possibly freed) address; RET4.
void* delete_native_physical_stream_pool_00bf4370(void*, std::uint32_t,
    NativePhysicalStreamOpenContext&);

// All native open modes, including ERROR_PATH_NOT_FOUND write retry. Original
// ECX stream, stack actual8h path/full flags, RET8. Uses real Win32 ANSI APIs.
// BF5590 is only JMP BF52A0; its source wrapper has the same service interface.
void open_native_physical_stream_00bf52a0(void*, const void*, std::uint32_t,
    ActualNativeStringPoolStorage&);
void open_native_physical_stream_00bf5590(void*, const void*, std::uint32_t,
    ActualNativeStringPoolStorage&);
// Original ECX provider; stack suffix/full flags; EAX stream/null; RET8.
// Complete consumer qualified to current numeric D69168+1C/BF3970 and
// D691B0+18/BF5020,+0/BF55A0,+4/BF5090. Table words must be readable at their
// actual addresses; unexpected words throw a SOURCE boundary, not native error.
void* open_native_physical_provider_00bf4ba0(void*, const void*, std::uint32_t,
    NativePhysicalStreamOpenContext&);

// Original ECX stream; no stack args; RET. AL Boolean / EDX:EAX cached64 bits.
bool valid_native_physical_stream_00bf5020(const void*) noexcept;

// Complete raw BF5020 entry, separate from the ordinary C++ helper above.
// MSVC Win32: ECX actual backing, unused incoming EDX, full EAX 0/1, RET.
// Requires stable backing with a live aligned HANDLE DWORD at +8h. Compares
// only against FFFFFFFF; zero and every other word return 1. No OS query.
// This memory-domain interface admits no source class, vtable or lifetime.
std::uint32_t __fastcall raw_valid_native_physical_stream_00bf5020(
    const void* actual_backing, std::uint32_t unused_edx) noexcept;

// Complete raw BF4F20 provider: ECX actual backing; unused incoming EDX;
// stack distance-low/high/origin; full SetFilePointerEx BOOL in EAX; RET0Ch.
// The real Win32 import receives current HANDLE+8 and writes directly to +10h.
// Requires stable live HANDLE and position fields with the real API's contract;
// no class/vtable, producer, lifetime or substream-dispatch admission follows.
std::int32_t __fastcall raw_seek_native_physical_stream_00bf4f20(
    void* actual_backing, std::uint32_t unused_edx, std::uint32_t distance_low,
    std::uint32_t distance_high, std::uint32_t origin) noexcept;

std::uint64_t size_native_physical_stream_00bf4f90(const void*) noexcept;
// Original ECX stream; stack unused DWORD; RET4; EAX GetFileSize low DWORD.
// Performs a fresh OS query, discards high DWORD, and does not update cache.
std::uint32_t query_native_physical_file_size_00bf4fa0(void*, std::uint32_t);
// Original ECX stream; stack dst/requested/optional count; EAX actual; RET0C.
// Read failure calls current manager FIELD+90 with original ECX and EDX=+18.
// Callable original-ABI failure service is required; null keeps invalid-call
// behavior. Count/position publication occurs only after that service returns.
std::uint32_t read_native_physical_stream_00bf5030(void*, void*, std::uint32_t,
    std::uint32_t*, NativePhysicalStreamOpenContext&);
// ECX stream; stack flags; EAX original address; RET4. Always CloseHandle,
// even INVALID_HANDLE_VALUE; write reference base identity; bit0 frees.
void* delete_native_physical_stream_00bf5090(void*, std::uint32_t);
// ECX stream; RET. Reference count has ALREADY reached zero. Current +4(0),
// then current pool getter and append dead address. No extra decrement.
void recycle_native_physical_stream_00bf55a0(void*, NativePhysicalStreamOpenContext&);

// New C++ service interfaces, not drop-in ABI/SEH replacements. Unimplemented
// stream slots (seek/write/etc), provider construction/index population and
// application startup wiring remain separate. No implicit destructor cleanup.
} // namespace bsp
