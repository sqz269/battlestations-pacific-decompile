#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_adopted_substream.hpp"

#include <cstdint>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource reader references require MSVC Win32.
#endif

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
void release_slot(void* slot, NativeAdoptedSubstreamDispatch& dispatch) {
    auto* const owner = pointer(word(slot));
    if (owner) {
        release_owner(owner, dispatch);
        put(slot, 0, 0);
    }
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
