#include "bsp/native_renderer_parent_member_cleanup.hpp"
#include "bsp/native_renderer_resolution_enumeration.hpp"
#include "bsp/native_renderer_capability_array_cleanup.hpp"
#include "bsp/native_renderer_capability_nested_arrays.hpp"
#include "bsp/native_input_binding_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <Windows.h>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(void* base, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(base) + offset);
}
volatile Word& word(void* base, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(base, offset));
}
std::int32_t signed_word(void* base, Word offset) noexcept {
    return static_cast<std::int32_t>(word(base, offset));
}
struct CapabilityDwordsUnwind {
    void* header;
    bool armed{true};
    ~CapabilityDwordsUnwind() noexcept {
        if (armed) destroy_native_capability_dwords_00b29e40(header);
    }
};
} // namespace

void __fastcall destroy_native_renderer_resolution_pairs_008d4e60(void* header) {
    if (signed_word(header, 8) < 0) reserve_native_resolution_pairs_008d4750(header, 0, 0);
    while (signed_word(header, 4) > 0) word(header, 4) = word(header, 4) - 1u;
    void* const data = reinterpret_cast<void*>(word(header));
    word(header, 4) = 0;
    singleton_lifetime_free(data);
}

void __fastcall destroy_native_renderer_dword_array_0086ae00(void* header) {
    resize_native_input_dwords_0086a430(header, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(word(header)));
}

void __fastcall destroy_native_renderer_capabilities_00b2f690(void* capabilities) {
    CapabilityDwordsUnwind cleanup{at(capabilities, 0x44)};
    void* const headers = at(capabilities, 0x50);
    resize_native_capability_headers_00b2ae20(headers, 0, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(word(headers)));
    cleanup.armed = false;
    void* const dwords = at(capabilities, 0x44);
    resize_native_capability_dwords_00b260b0(dwords, 0, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(word(dwords)));
}

void __fastcall destroy_native_renderer_capabilities_thunk_00b2f700(void* capabilities) {
    destroy_native_renderer_capabilities_00b2f690(capabilities);
}

void __fastcall destroy_native_embedded_tracked_section_00402f70(void* storage) {
    auto* const section = static_cast<CRITICAL_SECTION*>(storage);
    if (signed_word(storage, 0x18) > 0) {
        const auto leave = &::LeaveCriticalSection;
        do {
            word(storage, 0x18) = word(storage, 0x18) - 1u;
            leave(section);
        } while (signed_word(storage, 0x18) > 0);
    }
    ::DeleteCriticalSection(section);
}
} // namespace bsp
