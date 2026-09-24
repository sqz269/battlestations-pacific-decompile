#include "bsp/native_gui_widget_copy_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI widget copy requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& word(void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<unsigned char*>(p) + offset);
}
const volatile Word& word(const void* p, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(static_cast<const unsigned char*>(p) + offset);
}
void copy_byte(void* destination, const void* source, std::size_t offset) noexcept {
    *(static_cast<volatile unsigned char*>(destination) + offset) =
        *(static_cast<const volatile unsigned char*>(source) + offset);
}
void copy_x87(void* destination, const void* source, std::size_t offset) noexcept {
    auto* d = static_cast<unsigned char*>(destination) + offset;
    const auto* s = static_cast<const unsigned char*>(source) + offset;
    __asm {
        mov ecx, s
        mov edx, d
        fld dword ptr [ecx]
        fstp dword ptr [edx]
    }
}
void copy_rotation_and_clear_node(void* destination, const void* source) noexcept {
    __asm {
        mov ecx, source
        mov edx, destination
        fld dword ptr [ecx + 048h]
        mov dword ptr [edx + 04ch], 0
        fstp dword ptr [edx + 048h]
    }
}
void unwind_copy(void* destination, std::int32_t state) noexcept {
    try {
        auto* p = static_cast<unsigned char*>(destination);
        if (state >= 2) destroy_native_gui_timed_pointers_00aa7f50(p + 0x88);
        if (state >= 1) destroy_native_gui_widget_list_thunk_00a9bce0(p + 0x64);
        if (state >= 0) destroy_native_gui_ref_base_00aa6e10(destination);
    } catch (...) { std::terminate(); }
}
} // namespace

void* construct_native_gui_widget_copy_00aa9520(void* destination,
    const void* source, NativeGuiWidgetCopyContext& context,
    NativeGuiWidgetCopyAcquired& acquired) {
    if (acquired.started || acquired.complete || acquired.model_clone.started)
        throw std::logic_error("raw widget copy requires a fresh acquisition frame");
    acquired.started = true;
    initialize_native_gui_widget_identity_00aa9520_fragment(destination);
    for (std::size_t offset = 0x0c; offset <= 0x44; offset += 4)
        copy_x87(destination, source, offset);
    copy_rotation_and_clear_node(destination, source);
    for (const auto offset : {0x50u, 0x54u, 0x58u, 0x5cu, 0x60u})
        word(destination, offset) = word(source, offset);
    acquired.native_eh_state = 0;
    try {
        acquired.active_call_site = 0x00aa95e3;
        void* const sentinel = allocate_native_gui_widget_list_sentinel_00a9b720();
        word(destination, 0x68) = reinterpret_cast<Word>(sentinel);
        word(destination, 0x6c) = 0;
        word(destination, 0x70) = word(source, 0x70);
        copy_byte(destination, source, 0x74);
        copy_byte(destination, source, 0x75);
        *(static_cast<volatile unsigned char*>(destination) + 0x76) = 1;
        for (const auto offset : {0x77u, 0x78u, 0x79u, 0x84u})
            copy_byte(destination, source, offset);
        for (const auto offset : {0x88u, 0x8cu, 0x90u})
            word(destination, offset) = 0;
        copy_x87(destination, source, 0x94);
        for (const auto offset : {0xa4u, 0xa8u, 0xacu, 0xb0u, 0xb4u, 0xb8u, 0xbcu, 0xc0u})
            word(destination, offset) = word(source, offset);
        copy_x87(destination, source, 0xc4);
        copy_byte(destination, source, 0xd4);
        for (const auto offset : {0xd8u, 0xdcu, 0xe0u})
            word(destination, offset) = word(source, offset);

        // AA96E4..AA96F7: these are CURRENT values after allocation callbacks.
        // Valid native inputs guarantee nonnull node and a type in [0,18].
        void* const current_node = reinterpret_cast<void*>(word(source, 0x4c));
        const Word current_type = word(source, 0x60);
        const Word current_profile = word(current_node, 0);
        const Word flags = context.clone_flags_00d5c0b8[current_type];
        const auto* const profile = context.model_clone.access.models.vtable_00d62de8;
        const Word target = profile ? profile[4] : 0;
        acquired.native_eh_state = 2;
        acquired.active_call_site = 0x00aa96fc;
        auto& owners = context.model_clone.access.geometry.actual_owners();
        auto* current = dynamic_cast<NativeModelReference*>(&owners.resolve_actual(current_node));
        if (current_profile != 0x00d62de8u || target != 0x00b752b0u || !current ||
            &current->model_owner().storage.node != current_node)
            throw std::logic_error("raw widget copy has no binding for current node virtual10");
        auto& result = clone_native_gui_widget_model_00b752b0(
            current->model_owner(), flags, context.model_clone, acquired.model_clone);
        void* const raw_result = &result.model_owner().storage.node;
        word(destination, 0x4c) = reinterpret_cast<Word>(raw_result);
        // Transfer the sole creator; diagnostics remain, no retain/decrement.
        acquired.model_clone.creators.model = nullptr;
        word(raw_result, 0x138) = word(raw_result, 0x138) & 0xfffffffcu;
        acquired.native_eh_state = -1;
        acquired.complete = true;
        return destination;
    } catch (...) {
        unwind_copy(destination, acquired.native_eh_state);
        acquired.native_eh_state = -1;
        throw;
    }
}
} // namespace bsp
