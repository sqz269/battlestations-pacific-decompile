#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/director_geommesh_seg38_39_argon.hpp"

#include <cstdint>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource reader references require MSVC Win32.
#endif

namespace bsp::director_geommesh_seg38_39_argon {

// 0071F3B0..0071F3D8: native ECX=pointer slot, plain RET. New source ABI
// borrows only the two actual services this body calls. Moved here unchanged
// in behavior so both bsp_core consumers and the game use the same body.
__declspec(noinline) void bsp_ref_counted_ptr_release_0071f3b0(
    RefCountedSlotReleaseHost& host, void* self) {
    Raw slot{static_cast<std::uint8_t*>(self)};
    void* held = slot.ptr(0);
    if (held != nullptr) {
        Raw owner{static_cast<std::uint8_t*>(held)};
        if (host.interlocked_decrement_00ce2220(&owner.at<std::int32_t>(4)) == 0)
            host.vcall_00(held);
        slot.ptr(0) = nullptr;
    }
}

} // namespace bsp::director_geommesh_seg38_39_argon

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(LONG) == 4);

std::uint32_t word(const void* base) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, base
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
volatile LONG* references(void* owner) noexcept {
    return reinterpret_cast<volatile LONG*>(reinterpret_cast<std::uintptr_t>(owner) + 4u);
}
void release_owner(void* owner, NativeAdoptedSubstreamDispatch& dispatch) {
    if (InterlockedDecrement(references(owner)) == 0) {
        const auto table = word(owner);
        const auto entry = word(pointer(table));
        dispatch.source_zero_reference(entry, owner, table);
    }
}
class ResourceSlotRelease final
    : public director_geommesh_seg38_39_argon::RefCountedSlotReleaseHost {
public:
    explicit ResourceSlotRelease(NativeAdoptedSubstreamDispatch& dispatch)
        : dispatch_(dispatch) {}

    std::int32_t interlocked_decrement_00ce2220(std::int32_t* count) override {
        return InterlockedDecrement(reinterpret_cast<volatile LONG*>(count));
    }
    void vcall_00(void* owner) override {
        // Read the current profile only after the captured owner's count
        // reaches zero, exactly as the native CALL at 0071F3CE does.
        const auto table = word(owner);
        const auto entry = word(pointer(table));
        dispatch_.source_zero_reference(entry, owner, table);
    }
private:
    NativeAdoptedSubstreamDispatch& dispatch_;
};

void release_slot(void* slot, NativeAdoptedSubstreamDispatch& dispatch) {
    // Source composition, not an original call edge: bsp.py show --asm
    // 0071F3B0 and 00BE9ED0 have identical slot/decrement/vslot0/clear
    // instructions; 00483850 additionally returns the same slot address.
    // Reuse that complete body on the real reader/structured-node slot.
    // It captures the owner once and clears the slot AFTER terminal reentry.
    ResourceSlotRelease calls(dispatch);
    director_geommesh_seg38_39_argon::bsp_ref_counted_ptr_release_0071f3b0(calls, slot);
}
} // namespace

void* construct_native_resource_reader_00bf09a0(void* reader) noexcept {
    put(reader, 0, 0);
    put(reader, 4, 0);
    put(reader, 8, 0);
    put(reader, 0xc, 0);
    return reader;
}

void assign_native_resource_reader_stream_00bf0430(void* reader, void* new_stream,
    NativeAdoptedSubstreamDispatch& dispatch) {
    auto* const old_stream = pointer(word(reader));
    if (old_stream != new_stream) {
        put(reader, 0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(new_stream)));
        if (new_stream) InterlockedIncrement(references(new_stream));
        if (old_stream) release_owner(old_stream, dispatch);
    }
}

void release_native_structured_node_handle_00be9ed0(void* handle,
    NativeAdoptedSubstreamDispatch& dispatch) {
    release_slot(handle, dispatch);
}

void* release_native_resource_slot_00483850(void* slot,
    NativeAdoptedSubstreamDispatch& dispatch) {
    release_slot(slot, dispatch);
    return slot;
}

} // namespace bsp
