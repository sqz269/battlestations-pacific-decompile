#include "bsp/native_gui_text_retirement.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_native_gui_text_process.hpp"
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
        !a.shadow.started && !a.glyphs.started && !a.unwind_actions_consumed && !a.unwind_actions_returned;
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
void raw_default_admission(void* raw, NativeGuiTextRawModelRetirementContext& c) {
    auto& process = game::game_native_gui_text_process();
    require(&c.base.actual_base_lineage_00f8bc88 == &process.element_lineage_00f8bc88() &&
        &c.base.actual_text_descriptor_00f8be28 == &process.text_lineage_00f8be28()[0] &&
        !native_gui_widget_is_kind_of_00a9e070(native_gui_text_descriptor_00ab6a30(
            c.base.actual_text_descriptor_00f8be28), c.base.actual_base_lineage_00f8bc88),
        "Raw Text retirement needs CURRENT canonical lineage and the real empty base branch");
    void* const head = ptr(read(raw, 0x68));
    require(!read(raw, 0x4c) && !read(raw, 0x70) && !read(raw, 0x6c) && head &&
        read(head) == bits(head) && read(head, 4) == bits(head) &&
        !read(raw, 0x88) && !read(raw, 0x8c) && !read(raw, 0x90) &&
        !read(raw, 0x180) && !read(raw, 0x184) && !read(raw, 0x18c) && !read(raw, 0x190) &&
        !read(raw, 0x198) && !read(raw, 0x19c) && !read(raw, 0x1a0) && !read(raw, 0x1ec) &&
        !read(raw, 0xec) && !read(raw, 0xf0) && !read(raw, 0xf4) && !read(raw, 0xf8) &&
        !read(raw, 0x1a4) && !read(raw, 0x1a8) && !read(raw, 0x1c0) && !read(raw, 0x1c4) &&
        !read(raw, 0x1c8) && !read(raw, 0x1cc),
        "Raw Text retirement admits DEFAULT empty primary/parent/children/timed/glyph/content/shader branches");
}
void preflight(void* raw, std::size_t bytes, void* diagnostics, std::size_t diagnostic_bytes,
    NativeGuiTextRawModelRetirementContext& c, const volatile Word* flags = nullptr) {
    auto& r = c.resources;
    auto& storage = r.model.storage;
    auto& node = storage.node;
    auto& owners = r.geometry.actual_owners();
    require(raw && !(bits(raw) & 3u) && read(raw) == 0x00d5c6c8,
        "Raw Text retirement requires SAME completed DEFAULT actual storage");
    require(&r.association.storage == &storage && &r.parenting.node == &node &&
        &c.glyphs.nodes == &c.base.nodes && &c.glyphs.children == &c.base.bindings &&
        c.base.actual_base_table_00d5c130 && c.base.nodes.uses_raw_name_pool() &&
        &c.base.nodes.require_raw_name_pool() == &r.strings &&
        &node.trees.point_lights == &c.base.nodes.point_lights &&
        &node.strings == &r.strings && &node.scenes.owners == &owners && &node.trees.owners == &owners &&
        &r.materials.retained_owners == &owners &&
        same_native_string_storage_raw_pool_cells(r.material_strings, r.strings) &&
        same_native_string_raw_pool_cells(r.strings, r.string_process.raw_context()) &&
        &node.scenes.decrement_00ce2220 == &node.trees.decrement_00ce2220 && node.trees.decrement_00ce2220 &&
        &r.effects.effects.owners.actual_owners() == &owners &&
        &r.effects.effects.owners.string_storage() == &r.material_strings &&
        &r.effects.strings == &r.material_strings &&
        &c.actual_pool_00f8bdf0 == &game::game_native_gui_text_process().pool_00f8bdf0(),
        "Raw Text retirement must borrow SAME genuine Model/string/import/point-light/Text-pool providers");
    require(c.model.actual && c.model.reference && !c.model.reference->retired() &&
        c.model.construction.complete && c.model.construction.metadata_bound &&
        c.model.construction.context == &storage && !c.model.lifetime.started &&
        !c.model.lifetime.scalar_started && !c.model.release.started &&
        c.model.association.phase == NativeModelStorageGeometryPhase::complete &&
        ptr(read(raw, 0x188)) == c.model.actual &&
        c.model.reference->reference_count.load(std::memory_order_relaxed) > 0 &&
        &owners.resolve_actual(c.model.actual) == &*c.model.reference,
        "Raw Text retirement must retain its SAME genuine completed internal Model record");
    require(!read(c.model.actual, 0x30) && !read(c.model.actual, 0x34) && !read(c.model.actual, 0x38) &&
        !read(c.model.actual, 0x3c) && !read(c.model.actual, 0x40) &&
        *static_cast<const volatile std::uint8_t*>(at(c.model.actual, 0x44)) == 0 &&
        !read(c.model.actual, 0xa0) && !read(c.model.actual, 0xa4) && !read(c.model.actual, 0x130) &&
        !read(c.model.actual, 0x164) && !read(c.model.actual, 0x168) && !read(c.model.actual, 0x16c) &&
        !read(c.model.actual, 0x170) && !read(c.model.actual, 0x174),
        "DEFAULT Text private Model retirement excludes hierarchy/scene/retained/point-light publications");
    const Region writes[] = {{raw, bytes}, {diagnostics, diagnostic_bytes}};
    require(disjoint(writes[0], writes[1]), "Raw Text retirement diagnostics cannot alias Text");
    for (const Region borrowed : {Region{&c, sizeof(c)}, {&r, sizeof(r)}, {&c.model, sizeof(c.model)},
        {&c.base, sizeof(c.base)}, {&c.glyphs, sizeof(c.glyphs)}, {&r.model, sizeof(r.model)},
        {&storage, sizeof(storage)}, {&node, sizeof(node)}, {&node.trees, sizeof(node.trees)},
        {&node.scenes, sizeof(node.scenes)}, {&node.world, sizeof(node.world)},
        {&r.association, sizeof(r.association)}, {&r.geometry, sizeof(r.geometry)},
        {&r.materials, sizeof(r.materials)}, {&r.material_strings, sizeof(r.material_strings)},
        {&r.strings, sizeof(r.strings)}, {&r.string_process, sizeof(r.string_process)},
        {&r.effects, sizeof(r.effects)}, {&r.parenting, sizeof(r.parenting)},
        {&c.base.nodes, sizeof(c.base.nodes)}, {&c.glyphs.children, sizeof(c.glyphs.children)},
        {&node.trees.decrement_00ce2220, sizeof(node.trees.decrement_00ce2220)},
        {&c.actual_pool_00f8bdf0, sizeof(c.actual_pool_00f8bdf0)},
        {&c.actual_pool_00f8bdf0.storage(), sizeof(c.actual_pool_00f8bdf0.storage())}}) {
        for (const auto region : writes)
            require(disjoint(region, borrowed), "Raw Text retirement writes cannot alias its retained graph");
        if (flags) require(disjoint({flags, sizeof(Word)}, borrowed),
            "Raw Text flags cannot alias its retained graph");
    }
    if (flags) {
        require(!(bits(flags) & 3u), "Raw Text scalar flags require stable DWORD backing");
        for (const auto region : writes) require(disjoint(region, {flags, sizeof(Word)}),
            "Raw Text scalar flags must be separate from Text and diagnostics");
    }
    raw_default_admission(raw, c);
}
void call(NativeGuiTextRetirementAcquired& a, Word site, Boundary boundary = Boundary::provider_call) noexcept {
    a.active_call_site = site;
    a.boundary = boundary; // provider boundary, NEVER Native exception-origin proof
}
GuiNativeGeometryOwners& geometry(NativeGuiTextRetirementContext& c) { return c.resources.actual.geometry; }
GuiNativeGeometryOwners& geometry(NativeGuiTextRawModelRetirementContext& c) { return c.resources.geometry; }
NativeTextureSurfaceReferenceIncrement const volatile& decrement(NativeGuiTextRetirementContext& c) {
    return c.resources.decrement_00ce2220;
}
NativeTextureSurfaceReferenceIncrement const volatile& decrement(NativeGuiTextRawModelRetirementContext& c) {
    return c.resources.model.storage.node.trees.decrement_00ce2220;
}
void default_boundary(void*, NativeGuiTextRetirementContext&, NativeGuiTextRetirementAcquired&) {}
void default_boundary(void* raw, NativeGuiTextRawModelRetirementContext& c, NativeGuiTextRetirementAcquired& a) {
    a.boundary = Boundary::host_metadata;
    raw_default_admission(raw, c); // CURRENT after every preceding callback
}
void shadow_returned(NativeGuiTextRetirementContext&, NativeGuiTextRetirementAcquired&) {}
void shadow_returned(NativeGuiTextRawModelRetirementContext& c, NativeGuiTextRetirementAcquired& a) {
    a.boundary = Boundary::host_metadata;
    require(c.model.reference && c.model.release.started && c.model.release.complete,
        "Private Model current18 must use the retained genuine B6F310 invocation");
    if (c.model.reference->retired())
        require(c.model.lifetime.complete && c.model.lifetime.scalar_complete &&
            c.model.lifetime.flags_read && c.model.lifetime.pool_returned ==
                ((c.model.lifetime.flags_low_byte & 1u) != 0),
            "Model metadata retirement alone is not genuine current04 cleanup/late-flags/return proof");
    else require(!c.model.lifetime.started && !c.model.lifetime.scalar_started,
        "A retained live Model credit cannot conceal an entered incomplete Model destructor");
}
template<class Context>
void release_shader(void* raw, Context& c, NativeGuiTextRetirementAcquired& a) {
    auto& s = a.shader;
    if (!s.identity) return;
    call(a, 0x00ab828du);
    const auto captured_decrement = decrement(c); // CURRENT cell, once at reached site
    s.captured_decrement = captured_decrement;
    s.decrement_entered = true;
    const long result = captured_decrement(reinterpret_cast<volatile long*>(bits(s.identity) + 4u));
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
        dispatch_native_render_actual_owner_zero(geometry(c).actual_owners(), s.identity);
        s.zero_returned = true;
    }
    write(raw, 0x1ec, 0); // AB829F, only after the nonnull operation returns
    s.slot_cleared = true;
}
template<class Context>
void return_member(void* raw, Word offset, bool wide, std::size_t index,
    int next_state, Word disarmed, Word action, Word getter_site, Word return_site,
    Context& c, NativeGuiTextRetirementAcquired& a) {
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
template<class Context>
void unwind(void* raw, Context& c, NativeGuiTextRetirementAcquired& a) {
    a.exception_cleanup_started = true;
    try {
        while (a.native_eh_state >= 0) {
            const auto state = a.native_eh_state;
            if (state == 0) default_boundary(raw, c, a);
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
            // Independent metadata only: consumption precedes the call, but
            // completion is published only after its actual normal return.
            a.unwind_actions_returned |= Word{1} << state;
        }
    } catch (...) {
        if constexpr (std::is_same_v<Context, NativeGuiTextRawModelRetirementContext>) {
            if (a.boundary == Boundary::host_metadata) throw;
        }
        std::terminate();
    }
}
template<class Context>
void destroy_text(void* raw, Context& c, NativeGuiTextRetirementAcquired& a) {
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
            shadow_returned(c, a);
            write(raw, 0x188, 0);
            a.shadow_slot_cleared = true;
        }
        default_boundary(raw, c, a);
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
        default_boundary(raw, c, a);
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
        if constexpr (std::is_same_v<Context, NativeGuiTextRawModelRetirementContext>) {
            if (a.boundary == Boundary::host_metadata) throw;
            if (a.native_eh_state >= 0) default_boundary(raw, c, a);
        }
        if (a.native_eh_state >= 0) unwind(raw, c, a);
        throw;
    }
}
template<class Context>
void* delete_text(void* raw, const volatile Word& flags, Context& c, NativeGuiTextScalarRetirementAcquired& a) {
    require(!a.started && !a.complete && !a.destructor_entered && fresh(a.destruction),
        "Scalar Text retirement requires fresh persistent diagnostics");
    preflight(raw, 0x1f8, &a, sizeof(a), c, &flags); // address/separation ONLY
    a.started = true;
    a.destructor_entered = true;
    destroy_text(raw, c, a.destruction);
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
} // namespace
void destroy_native_gui_text_storage_00ab8250(void* raw,
    NativeGuiTextRetirementContext& c, NativeGuiTextRetirementAcquired& a) { destroy_text(raw, c, a); }
void destroy_native_gui_text_storage_00ab8250(void* raw,
    NativeGuiTextRawModelRetirementContext& c, NativeGuiTextRetirementAcquired& a) {
    try { destroy_text(raw, c, a); }
    catch (...) { if (!a.started) a.boundary = Boundary::host_metadata; throw; }
}
void* delete_native_gui_text_storage_00ab8ee0(void* raw, const volatile Word& flags,
    NativeGuiTextRetirementContext& c, NativeGuiTextScalarRetirementAcquired& a) { return delete_text(raw, flags, c, a); }
void* delete_native_gui_text_storage_00ab8ee0(void* raw, const volatile Word& flags,
    NativeGuiTextRawModelRetirementContext& c, NativeGuiTextScalarRetirementAcquired& a) {
    try { return delete_text(raw, flags, c, a); }
    catch (...) { if (!a.destruction.started) a.destruction.boundary = Boundary::host_metadata; throw; }
}
} // namespace bsp
