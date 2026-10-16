#include "bsp/native_gui_text_cursor_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_node_visibility_factor.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text cursor storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Acquired = NativeGuiTextCursorStorageAcquired;
using Frame = NativeGuiTextCursorStorageFrame;
using Context = NativeGuiTextCursorStorageContext;
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
void* at(void* p, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(p) + offset;
}
volatile Word& word(void* p, std::size_t offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
void* pointer(void* p, std::size_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
void* name_header(Frame& f) noexcept { return const_cast<Word*>(&f.locals[2]); }
void call(Acquired& a, Word site) noexcept {
    a.active_call_site = site;
    a.failure = Acquired::Failure::provider_call;
}
void metadata(Acquired& a) noexcept { a.failure = Acquired::Failure::host_metadata; }
struct Region { const volatile void* begin; std::size_t size; };
bool disjoint(Region l, Region r) noexcept {
    const auto x = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(l.begin));
    const auto y = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(r.begin));
    return x + l.size <= y || y + r.size <= x;
}
struct Views {
    const volatile Word* node;
    const volatile Word* model;
    const volatile Word* group;
    const volatile Word* renderer;
    const char* model_name;
    const char* declaration_name;
    const char* material_name;
};
Views preflight(void* text, Frame& f, Context& c, Acquired& a) {
    auto& s = c.shared;
    auto& actual = s.actual;
    auto& node = s.parenting.node;
    const auto mesh_constants = actual.geometry.mesh_constants();
    require(text && (reinterpret_cast<std::uintptr_t>(text) & 3u) == 0 &&
        !a.started && !a.complete && !a.skipped && a.native_eh_state == -1 &&
        a.failure == Acquired::Failure::none && !a.model_owner && !a.model_creator &&
        !a.mesh.started && !a.declaration.reference && !a.section.started &&
        a.vertex.phase == NativeStreamClonePhase::empty &&
        a.material.phase == NativeMaterialFactoryAcquired::Phase::empty &&
        !a.parenting.started && !a.declaration_release.entered && !a.vertex_release.entered &&
        !a.material_release.entered && !a.section_release.entered && !a.mesh_release.entered,
        "Raw Text cursor requires aligned actual storage and fresh persistent diagnostics");
    const Region writes[] = {{text, 0x1f8}, {&f, sizeof(f)}, {&a, sizeof(a)},
        {&f.locals[0], sizeof(f.locals)}, {&f.matrix[0], sizeof(f.matrix)},
        {&f.parenting, sizeof(f.parenting)}};
    for (std::size_t i = 0; i != 6; ++i) {
        for (std::size_t j = i + 1; j != 6; ++j)
            require(disjoint(writes[i], writes[j]), "Raw Text cursor writable regions must be separate");
        for (const Region borrowed : {Region{&c, sizeof(c)}, {&s, sizeof(s)},
            {&actual, sizeof(actual)}, {&s.node_constants, sizeof(s.node_constants)},
            {&s.strings, sizeof(s.strings)}, {&s.string_process, sizeof(s.string_process)},
            {&s.effects, sizeof(s.effects)}, {&s.parenting, sizeof(s.parenting)},
            {&c.text_reference, sizeof(c.text_reference)}, {&c.owners, sizeof(c.owners)},
            {&c.data, sizeof(c.data)}, {&c.transforms, sizeof(c.transforms)},
            {&c.renderer, sizeof(c.renderer)}, {&c.layout, sizeof(c.layout)},
            {&s.decrement_00ce2220, sizeof(s.decrement_00ce2220)},
            {&s.node_constants.one_00d7a24c, sizeof(Word)},
            {&actual.unchanged_00d7a260, sizeof(Word)},
            {&s.effects.effects.construction.current_renderer_00f8d394, sizeof(void*)}})
            require(disjoint(writes[i], borrowed), "Raw Text cursor writes must not alias borrowed providers");
        if (a.material.cache)
            require(disjoint(writes[i], {a.material.cache.get(), sizeof(*a.material.cache)}),
                "Raw Text cursor cache diagnostics must have independent backing");
    }
    require(&actual.geometry.actual_owners() == &c.owners &&
        c.text_reference.storage() == text && c.text_reference.phase() == NativeGuiTextReference::Phase::bound &&
        c.owners.find(text) == &c.text_reference,
        "Raw Text cursor requires its SAME live canonical122 binding without a count gate");
    require(actual.prepare_model && actual.retire_failed_model && actual.bind_completed_model &&
        !actual.models.actual_names && actual.models.nodes.uses_raw_name_pool() &&
        &actual.models.nodes.require_raw_name_pool() == &s.strings && &node.strings == &s.strings &&
        &s.strings == &s.string_process.raw_context() && &actual.strings == &s.string_process.strings() &&
        &actual.models.retained_owners == &c.owners && &actual.materials.retained_owners == &c.owners &&
        &node.scenes.owners == &c.owners && &node.trees.owners == &c.owners &&
        &node.scenes.decrement_00ce2220 == &s.decrement_00ce2220 &&
        &node.trees.decrement_00ce2220 == &s.decrement_00ce2220 && s.decrement_00ce2220 &&
        &s.effects.effects.owners.actual_owners() == &c.owners &&
        &s.effects.strings == &actual.strings && &actual.materials.parameter_names == &actual.strings &&
        &s.effects.effects.construction.strings == &actual.strings && s.effects.effects.cache == &s.effects &&
        &actual.models.constants.maximum_00ce4970 == &s.node_constants.positive_bound_00ce4970 &&
        &actual.models.constants.minimum_00ce4adc == &s.node_constants.negative_bound_00ce4adc &&
        &actual.models.constants.unchanged_00d7a260 == &actual.unchanged_00d7a260 &&
        &mesh_constants.maximum_00ce4970 == &s.node_constants.positive_bound_00ce4970 &&
        &mesh_constants.minimum_00ce4adc == &s.node_constants.negative_bound_00ce4adc,
        "Raw Text cursor must share actual owner/string/node/import/constant domains");
    require_gui_text_native_renderer_domain(c.renderer,
        s.effects.effects.construction.current_renderer_00f8d394, actual.strings, c.owners);
    const Views result{
        static_cast<const volatile Word*>(c.data.data_at(0x00d62c88, 0x40)),
        static_cast<const volatile Word*>(c.data.data_at(0x00d62de8, 0x5c)),
        static_cast<const volatile Word*>(c.data.data_at(0x00d634f8, 0x40)),
        static_cast<const volatile Word*>(c.data.data_at(0x00d5f0a8, 0x64)),
        static_cast<const char*>(c.data.data_at(0x00d5c788, 11)),
        static_cast<const char*>(c.data.data_at(0x00cfada4, 17)),
        static_cast<const char*>(c.data.data_at(0x00d5c778, 15))};
    require(actual.models.vtable_00d62de8 == result.model && actual.models.vtable_00d62c88 == result.node &&
        s.effects.effects.renderer_profile_00d5f0a8 == result.renderer &&
        &s.node_constants.one_00d7a24c == c.data.data_at(0x00d7a24c, 4) &&
        &actual.unchanged_00d7a260 == c.data.data_at(0x00d7a260, 4) &&
        actual.material_vtable_00d5e520 == c.data.data_at(0x00d5e520, 8),
        "Raw Text cursor requires already held genuine constant/profile aliases");
    return result;
}
NativeModelOwner& model(void* raw, Context& c) {
    require(raw, "Raw Text cursor requires a nonnull actual Model");
    auto* reference = dynamic_cast<NativeModelReference*>(&c.owners.resolve_actual(raw));
    require(reference && &reference->model_owner().storage.node == raw &&
        &reference->model_owner().environment == &c.shared.actual.models &&
        reference->model_owner().phase == NativeModelOwner::Phase::live,
        "Raw Text cursor Model must be its SAME live actual canonical creator");
    return reference->model_owner();
}
const volatile Word* profile(void* raw) noexcept {
    return reinterpret_cast<const volatile Word*>(word(raw));
}
void return_captured_name(void* data, Frame& f, Context& c, Acquired& a) {
    if (data) {
        const Word bytes = f.locals[2] + 1u;
        auto& strings = c.shared.strings;
        auto* pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, data, bytes, strings.actual_small_returns_disabled_01090aa4);
    }
    ++a.completed_name_returns;
}
void cleanup(Frame& f, Context& c, Acquired& a) noexcept {
    a.exception_cleanup_started = true;
    try {
        for (;;) {
            const auto state = a.native_eh_state;
            if (state < 0) return;
            a.native_eh_state = state == 1 ? 0 : -1; // consume BEFORE every call
            if (state == 1 || state == 2) {
                if (f.locals[0] & 1u) {
                    f.locals[0] &= ~Word{1};
                    a.conditional_name_consumed = true;
                    destroy_native_string_header_0041dd20(name_header(f), c.shared.strings);
                    ++a.completed_name_returns;
                }
            } else if (state == 4 || state == 5) {
                destroy_native_string_header_0041dd20(name_header(f), c.shared.strings);
                ++a.completed_name_returns;
            } else if (state == 0) {
                if (a.model_owner) {
                    auto* abandoned = std::exchange(a.model_owner, nullptr);
                    c.shared.actual.retire_failed_model(c.shared.actual.companion_context, *abandoned);
                }
                a.raw_model_return_entered = true;
                c.shared.actual.models.pool_01090054.return_raw_slot_00b74750(
                    reinterpret_cast<void*>(f.locals[1]));
                a.raw_model_returned = true;
            } else if (state == 3) {
                require(!a.delegated_mesh_action3_consumed && !a.mesh.constructor_complete,
                    "Raw Text cursor cannot repeat a delegated/completed mesh return");
                a.raw_mesh_return_entered = true;
                return_native_mesh_slot_00b72f70(reinterpret_cast<void*>(f.locals[1]));
            } else std::terminate();
        }
    } catch (...) { std::terminate(); }
}
template<class Resource>
void release(Resource*& creator, NativeTextureSurfaceReferenceIncrement decrement,
    NativeGuiTextCursorStorageRelease& r, Context& c) {
    require(creator && decrement && !r.entered, "Raw Text cursor release requires one unconsumed creator");
    void* raw = std::exchange(creator, nullptr); // consume BEFORE the actual call
    r.identity = raw;
    r.captured_decrement = decrement;
    r.entered = true;
    r.result = decrement(static_cast<volatile long*>(at(raw, 4)));
    r.decremented = true;
    if (r.result == 0) {
        r.zero_entered = true;
        dispatch_native_render_actual_owner_zero(c.owners, raw);
        r.zero_returned = true;
    }
    r.complete = true; // metadata only after potentially ended raw lifetime
}

// MOVSS one -> CURRENT184 capture -> eleven zero stores -> later MOVSS
// sentinel -> diagonal/sentinel stores. No float arithmetic or cached receiver.
__declspec(noinline) void* matrix_arguments(void* text, Frame& f, Context& c) noexcept {
    const volatile void* one = &c.shared.node_constants.one_00d7a24c;
    const volatile void* sentinel = &c.shared.actual.unchanged_00d7a260;
    volatile Word* matrix = &f.matrix[0];
    void* result;
    __asm {
        xorps xmm0, xmm0
        mov eax, one
        movss xmm1, dword ptr [eax]
        mov ecx, text
        mov ecx, dword ptr [ecx+184h]
        mov result, ecx
        mov eax, matrix
        movss dword ptr [eax+4], xmm0
        movss dword ptr [eax+8], xmm0
        movss dword ptr [eax+0ch], xmm0
        movss dword ptr [eax+10h], xmm0
        movss dword ptr [eax+18h], xmm0
        movss dword ptr [eax+1ch], xmm0
        movss dword ptr [eax+20h], xmm0
        movss dword ptr [eax+24h], xmm0
        movss dword ptr [eax+2ch], xmm0
        movss dword ptr [eax+30h], xmm0
        movss dword ptr [eax+34h], xmm0
        mov edx, sentinel
        movss xmm0, dword ptr [edx]
        movss dword ptr [eax], xmm1
        movss dword ptr [eax+14h], xmm1
        movss dword ptr [eax+28h], xmm1
        movss dword ptr [eax+38h], xmm0
        movss dword ptr [eax+3ch], xmm1
    }
    return result;
}
__declspec(noinline) void* visibility_arguments(void* text, Frame& f) noexcept {
    float* scalar = &f.visibility;
    void* result;
    __asm {
        fldz
        mov ecx, text
        mov ecx, dword ptr [ecx+184h]
        mov result, ecx
        mov eax, scalar
        fstp dword ptr [eax]
    }
    return result;
}
__declspec(noinline) void* geometry_arguments(void* text, Frame& f, Context& c,
    volatile std::int32_t& state) noexcept {
    const volatile void* constant = &c.shared.actual.unchanged_00d7a260;
    float* first = &f.scalar_178;
    float* second = &f.scalar_17c;
    volatile std::int32_t* state_slot = &state;
    void* result;
    __asm {
        mov eax, constant
        fld dword ptr [eax]
        mov ecx, text
        mov ecx, dword ptr [ecx+184h]
        mov result, ecx
        mov eax, second
        fst dword ptr [eax]
        mov edx, state_slot
        mov dword ptr [edx], -1
        mov eax, first
        fstp dword ptr [eax]
    }
    return result;
}
} // namespace

void ensure_native_gui_text_cursor_storage_00ab8910(void* text, Frame& f, Context& c, Acquired& a) {
    const Views views = preflight(text, f, c, a);
    a.started = true;
    Word register_mask = 0;
    f.locals[0] = 0;
    if (pointer(text, 0x184)) {
        a.skipped = true;
        a.complete = true;
        return;
    }
    try {
        call(a, 0x00ab8947);
        void* raw = c.shared.actual.models.pool_01090054.allocate_raw_slot_00b74d00();
        f.locals[1] = reinterpret_cast<Word>(raw);
        a.native_eh_state = 0;
        if (raw) {
            metadata(a);
            a.model_owner = &c.shared.actual.prepare_model(c.shared.actual.companion_context,
                raw, c.shared.actual.models);
            require(&a.model_owner->storage.node == raw &&
                &a.model_owner->environment == &c.shared.actual.models &&
                a.model_owner->phase == NativeModelOwner::Phase::prepared,
                "Raw cursor Model metadata must preserve its SAME unused actual slot");
            call(a, 0x00ab8963);
            construct_native_string_header_0041e870(name_header(f), c.shared.strings, views.model_name);
            register_mask = 1;
            a.native_eh_state = 1; // AB8974 precedes native stack mask store
            f.locals[0] = register_mask;
            call(a, 0x00ab897d);
            try { raw = construct_native_model_00b75030(*a.model_owner, name_header(f), c.shared.node_constants); }
            catch (...) {
                // Known preentry rejection is added Source policy. A phase or
                // native-site marker alone never establishes exception origin.
                if (a.model_owner->phase == NativeModelOwner::Phase::prepared) metadata(a);
                throw;
            }
            a.model_constructed = true;
        }
        const bool return_model_name = (register_mask & 1u) != 0;
        word(text, 0x184) = reinterpret_cast<Word>(raw); // publication BEFORE name return
        a.model_published = true;
        a.native_eh_state = -1;
        if (a.model_constructed) {
            metadata(a);
            a.model_creator = &c.shared.actual.bind_completed_model(
                c.shared.actual.companion_context, *a.model_owner);
            require(&a.model_creator->model_owner() == a.model_owner,
                "Raw cursor canonical Model must borrow its SAME completed creator");
            a.model_registered = true;
        }
        if (return_model_name) {
            void* data = reinterpret_cast<void*>(f.locals[3]); // state-1 BEFORE DATA here
            call(a, 0x00ab89b2);
            return_captured_name(data, f, c, a);
        }
        metadata(a);
        void* current_model = pointer(text, 0x184);
        require(current_model, "Native null cursor's later access is outside supported actual storage");
        word(current_model, 0x138) &= ~Word{3};
        f.parenting.parent_argument = reinterpret_cast<Word>(pointer(text, 0x4c));
        current_model = pointer(text, 0x184);
        call(a, 0x00ab89ce);
        set_native_raw_parent_00b6e680(current_model, f.parenting, c.shared.parenting, a.parenting);

        a.matrix_receiver = matrix_arguments(text, f, c);
        metadata(a);
        const volatile Word* current_table = profile(a.matrix_receiver);
        require(current_table == views.node || current_table == views.model || current_table == views.group,
            "Current cursor Model38 requires a genuine mapped actual profile");
        a.matrix_target = current_table[0x38 / 4];
        require(a.matrix_target == 0x00b6db10, "Current cursor Model38 target is unsupported");
        call(a, 0x00ab8a56);
        set_raw_local_matrix_00b6db10(a.matrix_receiver, c.transforms, const_cast<Word*>(&f.matrix[0]));
        a.visibility_receiver = visibility_arguments(text, f);
        call(a, 0x00ab8a65);
        set_native_node_visibility_factor_00b6da70(static_cast<NativeNodeStorage*>(a.visibility_receiver),
            nullptr, f.visibility, 0);

        call(a, 0x00ab8a6f);
        a.delegated_mesh_action3 = true;
        try { a.mesh_creator = c.shared.actual.geometry.create_native_mesh_00b73b60(a.mesh); }
        catch (...) {
            // The verified combined provider alone consumes ctor action3.
            // It clears raw_slot only AFTER its B72F70 successfully returns.
            if (a.mesh.native_site == 0x00b4c761 && !a.mesh.constructor_complete) {
                a.delegated_mesh_action3_consumed = true;
                a.raw_mesh_return_entered = true;
                a.native_eh_state = -1;
                if (a.mesh.raw_slot) std::terminate(); // nested cleanup escape
            } else if (a.mesh.native_site == 0 || a.mesh.constructor_complete) metadata(a);
            throw;
        }
        f.locals[1] = reinterpret_cast<Word>(a.mesh_creator);
        a.native_eh_state = 3;
        a.geometry_receiver = geometry_arguments(text, f, c, a.native_eh_state);
        metadata(a);
        NativeModelOwner& receiver = model(a.geometry_receiver, c);
        call(a, 0x00ab8aaa);
        set_native_model_geometry_00b75170(receiver, 0, a.mesh_creator, f.scalar_178, f.scalar_17c);
        a.model_geometry_published = true;
        metadata(a);
        require(a.mesh_creator, "Native null mesh's later access is outside supported cursor storage");

        call(a, 0x00ab8ab8);
        construct_native_string_header_0041e870(name_header(f), c.shared.strings, views.declaration_name);
        metadata(a);
        a.declaration_renderer = c.shared.effects.effects.construction.current_renderer_00f8d394;
        current_table = profile(a.declaration_renderer);
        require(current_table == views.renderer, "Current cursor renderer38 requires its genuine actual profile");
        a.declaration_target = current_table[0x38 / 4];
        require(a.declaration_target == 0x00b317e0, "Current cursor renderer38 target is unsupported");
        a.native_eh_state = 4;
        call(a, 0x00ab8ad5);
        a.declaration.reference = load_native_renderer_vertex_declaration_00b317e0(
            a.declaration_renderer, name_header(f), c.renderer.declarations);
        metadata(a);
        c.shared.actual.geometry.register_native_declaration_reference(a.declaration, c.renderer.declarations.declarations);
        void* data = reinterpret_cast<void*>(f.locals[3]); // DATA BEFORE state-1
        a.native_eh_state = -1;
        call(a, 0x00ab8afb);
        return_captured_name(data, f, c, a);

        metadata(a);
        a.vertex_renderer = c.shared.effects.effects.construction.current_renderer_00f8d394;
        current_table = profile(a.vertex_renderer);
        require(current_table == views.renderer, "Current cursor renderer5C requires its genuine actual profile");
        a.vertex_target = current_table[0x5c / 4];
        call(a, 0x00ab8b10);
        create_native_vertex_from_captured_renderer_5c(a.vertex_renderer, a.vertex_target,
            4, 1, a.declaration.reference, c.renderer.streams, a.vertex);
        call(a, 0x00ab8b19);
        set_native_mesh_vertex_stream_00b73bb0(*a.mesh_creator, c.owners, 0, a.vertex.creator);
        a.stream_published = true;
        call(a, 0x00ab8b22);
        const auto declaration_decrement = c.shared.decrement_00ce2220;
        release(a.declaration.reference, declaration_decrement, a.declaration_release, c);
        call(a, 0x00ab8b38);
        const auto vertex_decrement = c.shared.decrement_00ce2220;
        release(a.vertex.creator, vertex_decrement, a.vertex_release, c);

        call(a, 0x00ab8b4a);
        a.section_creator = c.shared.actual.geometry.create_native_section_00533fa0(a.section);
        metadata(a);
        require(a.section_creator, "Native null section's stores are outside supported cursor storage");
        word(a.section_creator, 0x08) = 5;
        word(a.section_creator, 0x0c) = 0;
        word(a.section_creator, 0x14) = 0;
        word(a.section_creator, 0x10) = 4;
        word(a.section_creator, 0x18) = 2;
        call(a, 0x00ab8b78);
        construct_native_string_header_0041e870(name_header(f), c.shared.strings, views.material_name);
        a.native_eh_state = 5;
        call(a, 0x00ab8b85);
        a.material_creator = create_native_material_from_effect_cache_00535320(name_header(f),
            c.shared.actual.materials.material_slots, c.owners, c.shared.effects, a.material);
        metadata(a);
        if (a.material_creator)
            c.shared.actual.geometry.register_native_material_creator(a.material,
                c.shared.actual.materials, c.shared.actual.material_vtable_00d5e520);
        data = reinterpret_cast<void*>(f.locals[3]); // DATA BEFORE state-1
        a.native_eh_state = -1;
        call(a, 0x00ab8bae);
        return_captured_name(data, f, c, a);
        metadata(a);
        require(a.material_creator, "Native null material's later access is outside supported cursor storage");
        call(a, 0x00ab8bb8);
        set_native_material_parameter_owner_00b18a40(*a.material_creator, text, 1, c.owners);
        a.material_owner_published = true;
        call(a, 0x00ab8bc0);
        set_native_mesh_section_material_00b864c0(*a.section_creator, c.owners, a.material_creator);
        a.material_published = true;
        const auto final_decrement = c.shared.decrement_00ce2220; // AB8BC5 ONCE
        a.final_decrement = final_decrement; // diagnostic, not the saved-call authority
        call(a, 0x00ab8bcf);
        release(a.material_creator, final_decrement, a.material_release, c);
        call(a, 0x00ab8be0);
        rebuild_native_mesh_section_vertex_layout_00b865a0(*a.section_creator, c.owners,
            a.mesh_creator, c.layout);
        call(a, 0x00ab8be8);
        append_native_mesh_draw_section_00b73c60(*a.mesh_creator, a.section_creator);
        a.section_appended = true;
        call(a, 0x00ab8bf1);
        release(a.section_creator, final_decrement, a.section_release, c);
        call(a, 0x00ab8c03);
        release(a.mesh_creator, final_decrement, a.mesh_release, c);
        a.complete = true;
        a.failure = Acquired::Failure::none;
    } catch (...) {
        if (a.failure != Acquired::Failure::host_metadata) cleanup(f, c, a);
        throw;
    }
}
} // namespace bsp
