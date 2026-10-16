#include "bsp/native_gui_text_retirement.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_gui_text_default_storage.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_wide_string.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Text retirement requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Boundary = NativeGuiTextRetirementAcquired::Boundary;
static_assert(sizeof(void*) == sizeof(Word));
static_assert(std::is_trivially_destructible_v<NativeGuiTextRetirementAcquired>);
static_assert(std::is_trivially_destructible_v<NativeGuiTextScalarRetirementAcquired>);
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(void* p, Word offset) noexcept { return ptr(bits(p) + offset); }
Word read(void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(bits(p) + offset) = value;
}
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
struct Region { const volatile void* begin; std::size_t size; };
bool disjoint(Region left, Region right) noexcept {
    const auto l = static_cast<std::uint64_t>(bits(left.begin));
    const auto r = static_cast<std::uint64_t>(bits(right.begin));
    return l + left.size <= r || r + right.size <= l;
}
bool fresh(const NativeGuiTextRetirementAcquired& a) noexcept {
    return !a.started && !a.complete && a.native_eh_state == -1 && !a.destination &&
        a.boundary == Boundary::none && !a.shader.decrement_entered &&
        !a.shadow.started && !a.glyphs.started && !a.unwind_actions_consumed;
}
void preflight(void* raw, std::size_t bytes, void* diagnostics, std::size_t diagnostic_bytes,
    NativeGuiTextRetirementContext& c, const volatile Word* flags = nullptr) {
    auto& r = c.resources;
    auto& owners = r.actual.geometry.actual_owners();
    auto& node = r.parenting.node;
    require(raw && (bits(raw) & 3u) == 0, "Text retirement requires aligned live actual storage");
    require(&c.base.nodes == &r.actual.models.nodes && &c.glyphs.nodes == &c.base.nodes &&
        &c.glyphs.children == &c.base.bindings && !r.actual.models.actual_names &&
        c.base.nodes.uses_raw_name_pool() && &c.base.nodes.require_raw_name_pool() == &r.strings &&
        &r.strings == &r.string_process.raw_context() && &r.actual.strings == &r.string_process.strings() &&
        &r.actual.models.retained_owners == &owners && &node.strings == &r.strings &&
        &node.scenes.owners == &owners && &node.trees.owners == &owners &&
        &node.trees.point_lights == &c.base.nodes.point_lights &&
        &node.scenes.decrement_00ce2220 == &r.decrement_00ce2220 &&
        &node.trees.decrement_00ce2220 == &r.decrement_00ce2220 && r.decrement_00ce2220 &&
        &r.effects.effects.owners.actual_owners() == &owners &&
        &r.effects.effects.owners.string_storage() == &r.actual.strings &&
        &r.effects.strings == &r.actual.strings && r.effects.effect_base_vtable_00d5e534 &&
        r.effects.effect_derived_vtable_00d61a00,
        "Text retirement must borrow SAME actual Node, shader, string, import and glyph domains");
    const Region writes[] = {{raw, bytes}, {diagnostics, diagnostic_bytes}};
    require(disjoint(writes[0], writes[1]), "Text retirement diagnostics cannot alias Text");
    for (const Region borrowed : {Region{&c, sizeof(c)}, {&r, sizeof(r)},
        {&c.base, sizeof(c.base)}, {&c.glyphs, sizeof(c.glyphs)}, {&r.actual, sizeof(r.actual)},
        {&r.actual.models, sizeof(r.actual.models)}, {&r.strings, sizeof(r.strings)},
        {&r.string_process, sizeof(r.string_process)}, {&r.effects, sizeof(r.effects)},
        {&r.parenting, sizeof(r.parenting)}, {&node, sizeof(node)}, {&node.trees, sizeof(node.trees)},
        {&node.scenes, sizeof(node.scenes)}, {&c.base.nodes, sizeof(c.base.nodes)},
        {&c.glyphs.children, sizeof(c.glyphs.children)}, {&r.decrement_00ce2220, sizeof(r.decrement_00ce2220)},
        {r.effects.effect_base_vtable_00d5e534, 8}, {r.effects.effect_derived_vtable_00d61a00, 8},
        {&c.actual_pool_00f8bdf0, sizeof(c.actual_pool_00f8bdf0)},
        {&c.actual_pool_00f8bdf0.storage(), sizeof(c.actual_pool_00f8bdf0.storage())}}) {
        for (const auto region : writes)
            require(disjoint(region, borrowed), "Text retirement caller writes cannot alias its provider graph");
        if (flags) require(disjoint({flags, sizeof(Word)}, borrowed),
            "Scalar Text flags backing cannot alias its provider graph");
    }
    if (flags) {
        require((bits(flags) & 3u) == 0, "Scalar Text flags require stable DWORD backing");
        for (const auto region : writes)
            require(disjoint(region, {flags, sizeof(Word)}), "Scalar Text flags backing must be separate");
    }
}
void call(NativeGuiTextRetirementAcquired& a, Word site, Boundary boundary = Boundary::provider_call) noexcept {
    a.active_call_site = site;
    a.boundary = boundary; // provider boundary, NEVER Native exception-origin proof
}
void release_shader(void* raw, NativeGuiTextRetirementContext& c, NativeGuiTextRetirementAcquired& a) {
    auto& s = a.shader;
    if (!s.identity) return;
    call(a, 0x00ab828du);
    const auto decrement = c.resources.decrement_00ce2220; // CURRENT cell, once at reached site
    s.captured_decrement = decrement;
    s.decrement_entered = true;
    const long result = decrement(reinterpret_cast<volatile long*>(bits(s.identity) + 4u));
    s.result = result;
    s.decremented = true;
    if (result == 0) {
        const Word profile = read(s.identity); // AB8297, AFTER decrement
        s.captured_profile = profile;
        call(a, 0x00ab8299u, Boundary::host_metadata);
        const volatile Word* table = nullptr;
        if (profile == 0x00d5e534u) table = c.resources.effects.effect_base_vtable_00d5e534;
        else if (profile == 0x00d61a00u) table = c.resources.effects.effect_derived_vtable_00d61a00;
        require(table != nullptr, "Text shader zero requires its genuine established actual profile");
        const Word target = table[0]; // CURRENT slot0 of SAME existing canonical alias
        s.captured_target = target;
        require(target == 0x00bd30e0u, "Text shader zero requires genuine current BD30E0 terminal");
        call(a, 0x00ab829du);
        s.zero_entered = true;
        // The real existing SAME+04 canonical terminal validates its current
        // profile/scalar route. No additional decrement or new owner.
        dispatch_native_render_actual_owner_zero(c.resources.actual.geometry.actual_owners(), s.identity);
        s.zero_returned = true;
    }
    write(raw, 0x1ec, 0); // AB829F, only after the nonnull operation returns
    s.slot_cleared = true;
}
void return_member(void* raw, Word offset, bool wide, std::size_t index,
    int next_state, Word disarmed, Word action, Word getter_site, Word return_site,
    NativeGuiTextRetirementContext& c, NativeGuiTextRetirementAcquired& a) {
    auto& s = a.strings[index];
    void* const data = ptr(read(raw, offset + 4u)); // DATA FIRST
    s.captured_data = data;
    s.data_captured = true;
    a.native_eh_state = next_state; // THEN consume outer action, including NULL
    a.disarmed_cleanup_actions |= disarmed;
    a.normal_actions_entered |= Word{1} << action;
    if (data) {
        const Word length = read(raw, offset); // only now CURRENT LENGTH
        const Word bytes = wide ? length * 2u + 2u : length + 1u;
        s.captured_bytes = bytes;
        s.length_captured = true;
        call(a, getter_site);
        s.getter_entered = true;
        auto& strings = c.resources.strings;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
        s.getter_returned = true;
        call(a, return_site);
        s.return_entered = true;
        return_native_string_pool_00bd1510(pool, data, bytes, strings.actual_small_returns_disabled_01090aa4);
        s.returned = true;
    }
    a.normal_actions_returned |= Word{1} << action;
}
void unwind(void* raw, NativeGuiTextRetirementContext& c, NativeGuiTextRetirementAcquired& a) noexcept {
    a.exception_cleanup_started = true;
    try {
        while (a.native_eh_state >= 0) {
            const auto state = a.native_eh_state;
            a.native_eh_state = state - 1;
            a.unwind_actions_consumed |= Word{1} << state;
            a.disarmed_cleanup_actions |= Word{1} << state;
            switch (state) {
            case 6: call(a, 0x00cb7ee7u);
                destroy_native_string_header_0041dd20(at(raw, 0x1c8), c.resources.strings); break;
            case 5: call(a, 0x00cb7ed9u);
                destroy_native_string_header_0041dd20(at(raw, 0x1c0), c.resources.strings); break;
            case 4: call(a, 0x00cb7ecbu);
                destroy_native_wide_string_header_00436430(at(raw, 0x1a4), c.resources.strings); break;
            case 3: call(a, 0x00cb7ebdu);
                tidy_native_gui_text_pointer_vector_005d6cf0(at(raw, 0x194)); break;
            case 2: call(a, 0x00cb7eafu);
                destroy_native_string_header_0041dd20(at(raw, 0xf4), c.resources.strings); break;
            case 1: call(a, 0x00cb7ea1u);
                destroy_native_wide_string_header_00436430(at(raw, 0xec), c.resources.strings); break;
            case 0: call(a, 0x00cb7e93u);
                a.base_entered = true;
                destroy_native_gui_widget_base_00aa9730(raw, c.base);
                a.base_returned = true; break;
            default: std::terminate();
            }
        }
    } catch (...) { std::terminate(); }
}
} // namespace

void destroy_native_gui_text_storage_00ab8250(void* raw,
    NativeGuiTextRetirementContext& c, NativeGuiTextRetirementAcquired& a) {
    require(fresh(a), "Text retirement requires fresh persistent diagnostics");
    preflight(raw, 0x1f4, &a, sizeof(a), c);
    a.started = true;
    a.destination = raw;
    try {
        write(raw, 0, 0x00d5c6c8u);
        a.shader.identity = ptr(read(raw, 0x1ec)); // AB8275 before state6
        a.native_eh_state = 6;
        release_shader(raw, c, a);
        void* const shadow = ptr(read(raw, 0x188)); // CURRENT, after shader callback/clear
        a.captured_shadow = shadow;
        if (shadow) {
            call(a, 0x00ab82afu);
            unlink_native_node_tree_00b6dfa0(shadow, c.resources.parenting.node.trees, a.shadow);
            write(raw, 0x188, 0);
            a.shadow_slot_cleared = true;
        }
        call(a, 0x00ab82bcu);
        clear_native_gui_text_glyph_children_00ab80c0(raw, c.glyphs, a.glyphs);
        return_member(raw, 0x1c8, false, 0, 5, 1u << 6, 6, 0x00ab82ddu, 0x00ab82e4u, c, a);
        return_member(raw, 0x1c0, false, 1, 4, 1u << 5, 5, 0x00ab8305u, 0x00ab830cu, c, a);
        return_member(raw, 0x1a4, true, 2, 3, 1u << 4, 4, 0x00ab832eu, 0x00ab8335u, c, a);
        // Real helper captures begin ONCE and frees/clears inline-equivalent
        // actual header storage. No extra diagnostic payload read/state2.
        call(a, 0x00ab8345u);
        a.vector_entered = true;
        a.normal_actions_entered |= 1u << 3;
        tidy_native_gui_text_pointer_vector_005d6cf0(at(raw, 0x194));
        a.vector_returned = true;
        a.normal_actions_returned |= 1u << 3;
        return_member(raw, 0xf4, false, 3, 1, (1u << 3) | (1u << 2), 2, 0x00ab837bu, 0x00ab8382u, c, a);
        return_member(raw, 0xec, true, 4, 0, 1u << 1, 1, 0x00ab83a3u, 0x00ab83aau, c, a);
        a.native_eh_state = -1;
        a.disarmed_cleanup_actions |= 1u;
        a.normal_actions_entered |= 1u;
        call(a, 0x00ab83b9u);
        a.base_entered = true;
        destroy_native_gui_widget_base_00aa9730(raw, c.base);
        a.base_returned = true;
        a.normal_actions_returned |= 1u;
        a.complete = true;
    } catch (...) {
        if (a.native_eh_state >= 0) unwind(raw, c, a);
        throw;
    }
}

void* delete_native_gui_text_storage_00ab8ee0(void* raw, const volatile Word& flags,
    NativeGuiTextRetirementContext& c, NativeGuiTextScalarRetirementAcquired& a) {
    require(!a.started && !a.complete && !a.destructor_entered && fresh(a.destruction),
        "Scalar Text retirement requires fresh persistent diagnostics");
    preflight(raw, 0x1f8, &a, sizeof(a), c, &flags); // address/separation ONLY
    a.started = true;
    a.destructor_entered = true;
    destroy_native_gui_text_storage_00ab8250(raw, c, a.destruction);
    a.destructor_returned = true;
    const auto current_flags = *reinterpret_cast<const volatile std::uint8_t*>(&flags); // AB8EE8 LOW BYTE only
    a.captured_flags_byte = current_flags;
    a.flags_captured = true;
    if (current_flags & 1u) {
        a.pool_return_entered = true;
        c.actual_pool_00f8bdf0.return_raw_slot_00ab75a0(raw);
        a.pool_returned = true;
    }
    a.complete = true;
    return raw; // SAME pointer bits, no access to ended Text payload
}
} // namespace bsp
