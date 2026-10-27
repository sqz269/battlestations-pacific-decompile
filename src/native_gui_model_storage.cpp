#include "bsp/native_gui_model_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include <cstddef>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Model widget storage requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(GuiModelPoolTraits::object_bytes == 0x11c);
static_assert(GuiModelPoolTraits::slot_bytes == 0x120);
static_assert(sizeof(void*) == 4 && sizeof(NativeGuiModelTailStorage) == 0x30);
static_assert(offsetof(NativeGuiModelTailStorage, scale_f4) == 8);
static_assert(offsetof(NativeGuiModelTailStorage, model_name_length_10c) == 0x20);
static_assert(offsetof(NativeGuiModelTailStorage, texture_override_length_114) == 0x28);
void* at(void* raw, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(raw) + offset);
}
const void* at(const void* raw, Word offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<Word>(raw) + offset);
}
volatile Word& word(void* raw, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(raw, offset));
}
Word read(const void* raw, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(raw, offset));
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
void require_storage(void* raw) {
    if (!raw || reinterpret_cast<Word>(raw) % 4)
        throw std::invalid_argument("GUI Model widget requires aligned actual11Ch storage");
}
void begin(void* raw, NativeGuiModelStorageAcquired& a, NativeGuiModelStorageAcquired::Entry entry) {
    require_storage(raw);
    if (a.phase != NativeGuiModelStorageAcquired::Phase::fresh)
        throw std::logic_error("GUI Model operation cannot replay entered or failed work");
    a.phase = NativeGuiModelStorageAcquired::Phase::entered;
    a.actual = raw; a.entry = entry;
}
template<class Body> void operation(void* raw, NativeGuiModelStorageAcquired& a,
    NativeGuiModelStorageAcquired::Entry entry, Body body) {
    begin(raw, a, entry);
    try { body(); a.phase = NativeGuiModelStorageAcquired::Phase::complete; }
    catch (...) {
        if (!a.failed_call_site) a.failed_call_site = a.active_call_site;
        a.phase = NativeGuiModelStorageAcquired::Phase::failed; throw;
    }
}
template<class Body> void unwind(Body body) noexcept {
    try { body(); } catch (...) { std::terminate(); }
}
void copy_float_x87(void* output, const void* source) {
    __asm {
        mov eax, output
        mov edx, source
        fld dword ptr [edx]
        fstp dword ptr [eax]
    }
}
// ACCFD4 loads sourceFC, then ACCFDA arms state0, then ACCFDE storesFC.
// Preserve that x87/diagnostic-state order too; this is not an FH3 adapter.
void copy_third_float_x87(void* output, const void* source, std::int32_t* state) {
    __asm {
        mov eax, output
        mov edx, source
        fld dword ptr [edx]
        mov ecx, state
        mov dword ptr [ecx], 0
        fstp dword ptr [eax]
    }
}
void cleanup_state(void* raw, NativeGuiModelStorageContext& c,
    NativeGuiModelStorageAcquired& a, std::int32_t state, bool copy) {
    a.cleanup_entered[state] = true;
    switch (state) {
    case 2:
        a.active_call_site = copy ? 0x00cb8ecf : 0x00cb8e6f;
        destroy_native_string_header_0041dd20(at(raw, 0x114), c.strings); break;
    case 1:
        a.active_call_site = copy ? 0x00cb8ec1 : 0x00cb8e61;
        destroy_native_string_header_0041dd20(at(raw, 0x10c), c.strings); break;
    case 0:
        a.active_call_site = copy ? 0x00cb8eb3 : 0x00cb8e53;
        destroy_native_gui_widget_base_00aa9730(raw, c.lifetime); break;
    default: std::terminate();
    }
}
void cleanup_after_failure(void* raw, NativeGuiModelStorageContext& c,
    NativeGuiModelStorageAcquired& a, bool copy) noexcept {
    a.failed_call_site = a.active_call_site;
    a.failed_eh_state = a.native_eh_state;
    while (a.native_eh_state >= 0) {
        const auto state = a.native_eh_state--;
        unwind([&] { cleanup_state(raw, c, a, state, copy); });
    }
}
void destroy(void* raw, NativeGuiModelStorageContext& c, NativeGuiModelStorageAcquired& a) {
    word(raw) = 0x00d5cf38;
    a.captured_wrapper = pointer(read(raw, 0xf0));
    a.native_eh_state = 2;
    try {
        if (a.captured_wrapper) {
            a.active_call_site = 0x00acc7ea;
            a.captured_decrement = c.decrement_00ce2220;
            if (!a.captured_decrement)
                throw std::logic_error("GUI Model requires the genuine current decrement import");
            a.wrapper_release_result = a.captured_decrement(
                static_cast<volatile long*>(at(a.captured_wrapper, 4)));
            a.wrapper_decremented = true;
            if (!a.wrapper_release_result) {
                a.active_call_site = 0x00acc7fa; a.wrapper_zero_entered = true;
                dispatch_native_render_actual_owner_zero(c.actual_owners, a.captured_wrapper);
                a.wrapper_zero_returned = true;
            }
            word(raw, 0xf0) = 0; // CURRENT field cleared only after captured terminal returns
        }
        a.native_eh_state = 1; a.active_call_site = 0x00acc822;
        destroy_native_string_header_0041dd20(at(raw, 0x114), c.strings);
        a.native_eh_state = 0; a.active_call_site = 0x00acc84a;
        destroy_native_string_header_0041dd20(at(raw, 0x10c), c.strings);
        a.native_eh_state = -1; a.active_call_site = 0x00acc860;
        destroy_native_gui_widget_base_00aa9730(raw, c.lifetime);
    } catch (...) {
        cleanup_after_failure(raw, c, a, false); throw;
    }
}
} // namespace
NativeGuiModelTailStorage& native_gui_model_tail(void* raw) {
    require_storage(raw);
    return *reinterpret_cast<NativeGuiModelTailStorage*>(at(raw, 0xec));
}
NativeGuiModelStorageAcquired::~NativeGuiModelStorageAcquired() noexcept {
    if (phase == Phase::entered || phase == Phase::failed) std::terminate();
}
void* construct_native_gui_model_default_00acc740(void* raw, const volatile Word& one) {
    require_storage(raw);
    construct_native_gui_widget_base_00aa9390(raw, 9, one);
    word(raw, 0xec) = 0; word(raw, 0xf0) = 0;
    word(raw) = 0x00d5cf38;
    word(raw, 0x10c) = 0; word(raw, 0x110) = 0;
    word(raw, 0x114) = 0; word(raw, 0x118) = 0;
    return raw;
}
void* construct_native_gui_model_copy_00accf70(void* raw, const void* source,
    NativeGuiModelCopyContext& c, NativeGuiModelCopyAcquired& a) {
    auto& o = a.operation;
    operation(raw, o, decltype(o.entry)::copy, [&] {
        o.source = source;
        try {
            o.active_call_site = 0x00accf95;
            construct_native_gui_widget_copy_00aa9520(raw, source, c.base, a.base);
            a.base_returned = true;
            word(raw) = 0x00d5cf38;
            word(raw, 0xec) = 0; word(raw, 0xf0) = 0;
            o.active_call_site = 0x00accfae;
            copy_float_x87(at(raw, 0xf4), at(source, 0xf4));
            copy_float_x87(at(raw, 0xf8), at(source, 0xf8));
            copy_third_float_x87(at(raw, 0xfc), at(source, 0xfc), &o.native_eh_state);
            copy_float_x87(at(raw, 0x100), at(source, 0x100));
            copy_float_x87(at(raw, 0x104), at(source, 0x104));
            copy_float_x87(at(raw, 0x108), at(source, 0x108));
            o.active_call_site = 0x00acd017;
            copy_construct_native_string_header_00426060(at(raw, 0x10c), at(source, 0x10c), c.model.strings);
            a.name_returned = true;
            o.native_eh_state = 1; o.active_call_site = 0x00acd05f;
            copy_construct_native_string_header_00426060(at(raw, 0x114), at(source, 0x114), c.model.strings);
            a.override_returned = true;
            o.native_eh_state = 2; o.active_call_site = 0x00acd086;
            a.load_entered = true;
            c.load.load_model_00acccf0(raw, at(raw, 0x10c), at(raw, 0x114));
            a.load_returned = true; o.native_eh_state = -1;
        } catch (...) {
            cleanup_after_failure(raw, c.model, o, true); throw;
        }
    });
    return raw;
}
void* create_native_gui_model_default_00aa16e0(const volatile Word& one) {
    void* const raw = allocate_gui_widget_pool_slot<GuiModelPoolTraits>();
    if (!raw) return nullptr;
    try { return construct_native_gui_model_default_00acc740(raw, one); }
    catch (...) {
        unwind([&] { free_gui_widget_pool_slot<GuiModelPoolTraits>(raw); });
        throw;
    }
}
void destroy_native_gui_model_00acc7b0(void* raw, NativeGuiModelStorageContext& c,
    NativeGuiModelStorageAcquired& a) {
    operation(raw, a, decltype(a.entry)::destroy, [&] { destroy(raw, c, a); });
}
void* delete_native_gui_model_00acc880(void* raw, const volatile Word& flags,
    NativeGuiModelStorageContext& c, NativeGuiModelStorageAcquired& a) {
    operation(raw, a, decltype(a.entry)::scalar, [&] {
        destroy(raw, c, a);
        if (*reinterpret_cast<const volatile std::uint8_t*>(&flags) & 1u) {
            a.active_call_site = 0x00acc895;
            free_gui_widget_pool_slot<GuiModelPoolTraits>(raw); a.slot_returned = true;
        }
    });
    return raw;
}
} // namespace bsp
