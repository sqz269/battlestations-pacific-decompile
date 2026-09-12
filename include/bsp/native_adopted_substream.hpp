#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

inline constexpr std::size_t native_adopted_substream_bytes = 0x28;

// Source dispatch, separate from the actual 28h owner. Each entry is the
// CURRENT method word captured at its native call site, not a cached table or
// replacement owner. A numeric-profile implementation must route that entry
// to a reconstructed method; the callable adapter below requires real host
// functions with the documented original Win32 ABI.
class NativeAdoptedSubstreamDispatch {
public:
    virtual ~NativeAdoptedSubstreamDispatch() = default;
    virtual std::uint8_t source_is_open(std::uintptr_t entry, void* source) = 0;
    virtual std::uint32_t source_seek(std::uintptr_t entry, void* source,
        std::uint32_t low, std::uint32_t high, std::uint32_t origin) = 0;
    virtual void source_read(std::uintptr_t entry, void* source, void* destination,
        std::uint32_t requested, std::uint32_t* actual) = 0;
    virtual void source_write(std::uintptr_t entry, void* source, const void* bytes,
        std::uint32_t requested, std::uint32_t* actual) = 0;
    virtual void source_zero_reference(std::uintptr_t entry, void* source,
        std::uintptr_t captured_table) = 0;
};
NativeAdoptedSubstreamDispatch& callable_native_adopted_substream_dispatch() noexcept;

// BB8B80: ECX ignored; stack token; AL result; RET4. Two CURRENT DWORD IDs
// from file descriptor 0109DB58/0109DB5C, excluding its name word. No implicit
// initialization or zero normalization; this source interface borrows storage.
bool query_native_file_stream_type_00bb8b80(std::uint32_t token,
    const volatile std::uint32_t* actual_file_ids_0109db58) noexcept;

// BF1130: ECX raw28h, stack adopted source/start-low/high/length-low/high;
// EAX owner; RET14h. No retain. Writes source/start/wrapping end, seeks current
// source slot1C to start, then RELOADS start into current position. Leaves +0C
// untouched. A throwing seek restores only stream/reference base profiles;
// it neither releases the adopted source nor clears its stored pointer.
void* construct_native_adopted_substream_00bf1130(void* actual_owner,
    void* adopted_source, std::uint32_t start_low, std::uint32_t start_high,
    std::uint32_t length_low, std::uint32_t length_high,
    NativeAdoptedSubstreamDispatch&);

// BF1000 / BF1040: ECX owner; destination/source, requested, optional actual
// stack; EAX actual; RET0C. Forward slot24/28 unchanged, with a local actual
// initialized to requested. Ignore callee return; add actual to CURRENT 64-bit
// position and then publish optional actual. No entry-end clamp or transform.
std::uint32_t read_native_adopted_substream_00bf1000(void*, void*,
    std::uint32_t, std::uint32_t*, NativeAdoptedSubstreamDispatch&);
std::uint32_t write_native_adopted_substream_00bf1040(void*, const void*,
    std::uint32_t, std::uint32_t*, NativeAdoptedSubstreamDispatch&);

// BF1080/BF10A0: ECX owner, EDX:EAX wrapping current-start/end-start, RET.
std::uint64_t position_native_adopted_substream_00bf1080(const void*) noexcept;
std::uint64_t length_native_adopted_substream_00bf10a0(const void*) noexcept;
// BF10B0: ECX owner, stack origin, EDX:EAX absolute base, RET4.
// Origin0 -> start;1 -> current; EVERY other value -> end.
std::uint64_t origin_native_adopted_substream_00bf10b0(
    const void*, std::uint32_t origin) noexcept;
// BF1090: ECX owner -> current source+8/slot18 tail dispatch, AL open result.
std::uint8_t open_native_adopted_substream_00bf1090(
    void*, NativeAdoptedSubstreamDispatch&);
// BF10E0: ECX owner, stack low/high/origin, RET0C; forwards source EAX.
// Publish wrapping absolute position BEFORE current source slot1C source_seek(...,0).
// Failed/throwing source seek does not restore that position.
std::uint32_t seek_native_adopted_substream_00bf10e0(void*, std::uint32_t,
    std::uint32_t, std::uint32_t, NativeAdoptedSubstreamDispatch&);

// BF11C0: ECX owner, RET. Real InterlockedDecrement of captured source+4;
// current slot0 only at zero. Clear CURRENT owner+8 after successful return,
// even if callback replaced it. Normal/unwind restore D5C104 then CEB130.
void destroy_native_adopted_substream_00bf11c0(void*, NativeAdoptedSubstreamDispatch&);
// BF1240: ECX owner, stack flags, EAX original owner, RET4. Destroy first,
// free through shared allocation service only after success and flags bit0.
void* delete_native_adopted_substream_00bf1240(
    void*, std::uint32_t flags, NativeAdoptedSubstreamDispatch&);

// Complete actual-storage bodies with new C++ interfaces. Native CRT/FH3
// identity, archive construction and numeric-profile dispatch are separate.
} // namespace bsp
