#include "bsp/native_gui_curve_storage.hpp"
#include "bsp/cc13_front6.hpp"
#include "bsp/native_animation_registry.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_load_hooks.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Curve storage requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(GuiCurvePoolTraits::object_bytes == 0x140);
static_assert(GuiCurvePoolTraits::slot_bytes == 0x144);
static_assert(sizeof(void*) == 4 && sizeof(NativeGuiCurveCountedArrayStorage) == 0xc);
static_assert(sizeof(NativeGuiCurveTailStorage) == 0x54);
static_assert(offsetof(NativeGuiCurveTailStorage, material_12c) == 0x40);
static_assert(offsetof(NativeGuiCurveTailStorage, authored_proxy_130) == 0x44);
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
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void require_storage(void* raw) {
    if (!raw || reinterpret_cast<Word>(raw) % 4)
        throw std::invalid_argument("Curve requires its aligned actual140h storage");
}
void begin(void* raw, NativeGuiCurveStorageAcquired& a, NativeGuiCurveStorageAcquired::Entry entry) {
    require_storage(raw);
    if (a.phase != NativeGuiCurveStorageAcquired::Phase::fresh)
        throw std::logic_error("Curve operation cannot replay entered or failed work");
    a.phase = NativeGuiCurveStorageAcquired::Phase::entered;
    a.actual = raw; a.entry = entry;
}
template<class Body> void operation(void* raw, NativeGuiCurveStorageAcquired& a,
    NativeGuiCurveStorageAcquired::Entry entry, Body body) {
    begin(raw, a, entry);
    try { body(); a.phase = NativeGuiCurveStorageAcquired::Phase::complete; }
    catch (...) {
        if (!a.failed_call_site) a.failed_call_site = a.active_call_site;
        a.phase = NativeGuiCurveStorageAcquired::Phase::failed; throw;
    }
}
template<class Body> void unwind(Body body) noexcept {
    try { body(); } catch (...) { std::terminate(); }
}
void defaults(void* raw, const volatile Word& one) noexcept {
    word(raw) = 0x00d5c7f8;
    for (Word offset = 0xec; offset <= 0x118; offset += 4) word(raw, offset) = 0;
    word(raw, 0x11c) = 0; word(raw, 0x120) = 0;
    const Word captured_one = one; // ABE0B7 / ABE18A, after the base returns.
    word(raw, 0x124) = captured_one; word(raw, 0x128) = captured_one;
    word(raw, 0x12c) = 0;
    word(raw, 0x134) = 0; word(raw, 0x138) = 0; word(raw, 0x13c) = 0;
}
// Reuse the existing recovered86A350 body. Only its two actual CRT sites are
// reachable; unrelated members of this older wide host reject explicitly.
// This binding adds no array, allocator, process singleton or native count.
class CurveArrayCrtBinding final : public cc13_front6::Front6Host {
    [[noreturn]] static void unreached() {
        throw std::logic_error("Curve array CRT binding reached a non-array service");
    }
public:
    void* operator_new_00bf55be(Word bytes) override {
        return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    }
    void free_00bf6989(void* block) override { singleton_lifetime_free(block); }
    void* global_address(Word) override { unreached(); }
    void* global_pointer(Word) override { unreached(); }
    void invalid_parameter_00bf6713() override { unreached(); }
    std::int32_t stricmp_00bf7fbf(const char*, const char*) override { unreached(); }
    void* sized_pool_00419cc0() override { unreached(); }
    void sized_pool_return_00bd1510(void*, void*, Word, Word) override { unreached(); }
    cc13_front6::NativeStr* native_string_from_int_004260b0(cc13_front6::NativeStr*, std::int32_t) override { unreached(); }
    std::uint8_t* linked_list_nth_node_004be940(void*, std::int32_t) override { unreached(); }
    cc13_front6::MapIterator* map_find_00a03b20(void*, cc13_front6::MapIterator*, const Word*) override { unreached(); }
    void archive_enter_04(void*, cc13_front6::TaggedValue) override { unreached(); }
    void archive_leave_08(void*) override { unreached(); }
    void archive_write_0c(void*, cc13_front6::TaggedValue, cc13_front6::TaggedValue) override { unreached(); }
    void game_entity_write_archive_00928450(void*, void*) override { unreached(); }
    void recon_detection_set_write_archive_00805020(void*, void*) override { unreached(); }
    void observer_member_write_ref_0070e890(void*, void*) override { unreached(); }
    void* object_vslot_08(void*) override { unreached(); }
    float* vector4_scale_00bee140(float*, const float*, const float*) override { unreached(); }
    void effect_named_record14_copy_construct_0086cdb0(void*, const void*) override { unreached(); }
};
void reserve_pairs(void* header, std::int32_t count) {
    CurveArrayCrtBinding binding;
    cc13_front6::float_pair_array_reserve_0086a350(
        {static_cast<std::uint8_t*>(header)}, count, binding);
}
void clear_pairs(void* header) {
    if (static_cast<std::int32_t>(read(header, 8)) < 0) reserve_pairs(header, 0);
    while (static_cast<std::int32_t>(read(header, 4)) > 0) word(header, 4) -= 1u;
    word(header, 4) = 0;
}
void tidy_pairs(void* header) {
    clear_pairs(header);
    singleton_lifetime_free(pointer(read(header))); // native header stays untouched
}
void tidy_widths(void* header) {
    resize_native_float_array_00818030(header, 0);
    singleton_lifetime_free(pointer(read(header)));
}
void tidy_authored(void* raw) noexcept {
    if (void* block = pointer(read(raw, 0x134))) singleton_lifetime_free(block);
    word(raw, 0x134) = 0; word(raw, 0x138) = 0; word(raw, 0x13c) = 0;
}
void copy_pair_x87(void* destination, const void* source) {
    __asm {
        mov eax, destination
        mov edx, source
        fld dword ptr [edx]
        fstp dword ptr [eax]
        fld dword ptr [edx + 4]
        fstp dword ptr [eax + 4]
    }
}
std::int32_t doubled_capacity(Word capacity) noexcept {
    const auto doubled = static_cast<std::int32_t>(capacity + capacity);
    return doubled <= 1 ? 1 : doubled;
}
void return_failed_slot(void* raw) noexcept {
    unwind([&] { free_gui_widget_pool_slot<GuiCurvePoolTraits>(raw); });
}
void cleanup_state(void* raw, NativeGuiCurveStorageContext& c,
    NativeGuiCurveStorageAcquired& a, std::int32_t state) {
    a.cleanup_entered[state] = true;
    switch (state) {
    case 5: a.active_call_site = 0x00cb8359; tidy_authored(raw); break;
    case 4: a.active_call_site = 0x00cb834b; tidy_pairs(at(raw, 0x10c)); break;
    case 3: a.active_call_site = 0x00cb833d;
        destroy_native_string_header_0041dd20(at(raw, 0x104), c.strings); break;
    case 2: a.active_call_site = 0x00cb832f; tidy_widths(at(raw, 0xf8)); break;
    case 1: a.active_call_site = 0x00cb8321; tidy_pairs(at(raw, 0xec)); break;
    case 0: a.active_call_site = 0x00cb8313;
        destroy_native_gui_widget_base_00aa9730(raw, c.lifetime); break;
    default: std::terminate();
    }
}
void destroy(void* raw, NativeGuiCurveStorageContext& c, NativeGuiCurveStorageAcquired& a) {
    word(raw) = 0x00d5c7f8;
    a.captured_material = pointer(read(raw, 0x12c));
    a.native_eh_state = 5;
    try {
        if (a.captured_material) {
            a.active_call_site = 0x00abdece;
            a.captured_decrement = c.decrement_00ce2220;
            if (!a.captured_decrement)
                throw std::logic_error("Curve requires the genuine current InterlockedDecrement import");
            a.material_release_result = a.captured_decrement(
                static_cast<volatile long*>(at(a.captured_material, 4)));
            a.material_decremented = true;
            if (!a.material_release_result) {
                a.active_call_site = 0x00abdede; a.material_zero_entered = true;
                dispatch_native_render_actual_owner_zero(c.actual_owners, a.captured_material);
                a.material_zero_returned = true;
            }
            word(raw, 0x12c) = 0; // only AFTER current terminal callback returns
        }
        a.active_call_site = 0x00abdef1; tidy_authored(raw);
        a.native_eh_state = 3; a.active_call_site = 0x00abdf1e;
        tidy_pairs(at(raw, 0x10c));
        a.native_eh_state = 2; a.active_call_site = 0x00abdf62;
        destroy_native_string_header_0041dd20(at(raw, 0x104), c.strings);
        a.native_eh_state = 1; a.active_call_site = 0x00abdf7c;
        tidy_widths(at(raw, 0xf8));
        a.native_eh_state = 0; a.active_call_site = 0x00abdf9e;
        tidy_pairs(at(raw, 0xec));
        a.native_eh_state = -1; a.active_call_site = 0x00abdfc4;
        destroy_native_gui_widget_base_00aa9730(raw, c.lifetime);
    } catch (...) {
        a.failed_call_site = a.active_call_site;
        a.failed_eh_state = a.native_eh_state;
        while (a.native_eh_state >= 0) {
            const auto state = a.native_eh_state--;
            unwind([&] { cleanup_state(raw, c, a, state); });
        }
        throw;
    }
}
} // namespace
NativeGuiCurveTailStorage& native_gui_curve_tail(void* raw) {
    require_storage(raw);
    return *reinterpret_cast<NativeGuiCurveTailStorage*>(at(raw, 0xec));
}
NativeGuiCurveStorageAcquired::~NativeGuiCurveStorageAcquired() noexcept {
    if (phase == Phase::entered || phase == Phase::failed) std::terminate();
}
void* construct_native_gui_curve_default_00abe030(void* raw, const volatile Word& one) {
    require_storage(raw);
    construct_native_gui_widget_base_00aa9390(raw, 13, one);
    defaults(raw, one); return raw;
}
void* construct_native_gui_curve_copy_00abe100(void* raw, const void* source,
    NativeGuiWidgetCopyContext& c, NativeGuiWidgetCopyAcquired& a, const volatile Word& one) {
    require_storage(raw);
    construct_native_gui_widget_copy_00aa9520(raw, source, c, a);
    defaults(raw, one); return raw;
}
void* create_native_gui_curve_default_00aa1800(const volatile Word& one) {
    void* const raw = allocate_gui_widget_pool_slot<GuiCurvePoolTraits>();
    if (!raw) return nullptr;
    try { return construct_native_gui_curve_default_00abe030(raw, one); }
    catch (...) { return_failed_slot(raw); throw; }
}
void* create_native_gui_curve_copy_00aa1800(const void* source, NativeGuiWidgetCopyContext& c,
    NativeGuiWidgetCopyAcquired& a, const volatile Word& one) {
    if (!source) throw std::invalid_argument("Curve copy arm requires its captured nonnull source");
    const void* const captured = source;
    void* const raw = allocate_gui_widget_pool_slot<GuiCurvePoolTraits>();
    if (!raw) return nullptr;
    try { return construct_native_gui_curve_copy_00abe100(raw, captured, c, a, one); }
    catch (...) { return_failed_slot(raw); throw; }
}
void clear_native_gui_curve_points_00abc920(void* raw, NativeGuiCurveStorageAcquired& a) {
    operation(raw, a, decltype(a.entry)::clear, [&] {
        a.active_call_site = 0x00abc937; clear_pairs(at(raw, 0x10c));
        a.active_call_site = 0x00abc95d; resize_native_float_array_00818030(at(raw, 0xf8), 0);
    });
}
void append_native_gui_curve_point_00abc970(void* raw, const void* pair,
    const volatile Word& width, NativeGuiCurveStorageAcquired& a) {
    operation(raw, a, decltype(a.entry)::append, [&] {
        if (read(raw, 0x110) == read(raw, 0x114)) {
            a.active_call_site = 0x00abc997;
            reserve_pairs(at(raw, 0x10c), doubled_capacity(read(raw, 0x114)));
        }
        void* output = pointer(read(raw, 0x10c) + read(raw, 0x110) * 8u);
        a.active_call_site = 0x00abc9ac;
        if (output) copy_pair_x87(output, pair);
        word(raw, 0x110) += 1u;
        if (read(raw, 0xfc) == read(raw, 0x100)) {
            a.active_call_site = 0x00abc9dd;
            reserve_native_float_array_008153e0(at(raw, 0xf8), doubled_capacity(read(raw, 0x100)));
        }
        output = pointer(read(raw, 0xf8) + read(raw, 0xfc) * 4u);
        a.active_call_site = 0x00abc9ee;
        if (output) word(output) = width;
        word(raw, 0xfc) += 1u;
    });
}
void load_native_gui_curve_00abc040(void* raw, NativeGuiCurveLoadedCalls& calls,
    NativeGuiCurveStorageAcquired& a) {
    operation(raw, a, decltype(a.entry)::loaded, [&] {
        a.active_call_site = 0x00abc043; calls.loaded_base_00aa7170(raw);
        a.active_call_site = 0x00abc04c;
        set_native_gui_widget_active_00aa6a30(raw, nullptr, 1);
    });
}
void destroy_native_gui_curve_00abde90(void* raw, NativeGuiCurveStorageContext& c,
    NativeGuiCurveStorageAcquired& a) {
    operation(raw, a, decltype(a.entry)::destroy, [&] { destroy(raw, c, a); });
}
void* delete_native_gui_curve_00abe010(void* raw, const volatile Word& flags,
    NativeGuiCurveStorageContext& c, NativeGuiCurveStorageAcquired& a) {
    operation(raw, a, decltype(a.entry)::scalar, [&] {
        destroy(raw, c, a);
        if (*reinterpret_cast<const volatile std::uint8_t*>(&flags) & 1u) {
            a.active_call_site = 0x00abe025;
            free_gui_widget_pool_slot<GuiCurvePoolTraits>(raw); a.slot_returned = true;
        }
    });
    return raw;
}
} // namespace bsp
