#include "bsp/native_gui_text_storage_construct.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text sections require MSVC Win32.
#endif

namespace bsp {
// TU-local use of private retained negative metadata; no public setter/issuer.
struct NativeGuiTextRawModelSectionFailureAccess final {
    static const NativeGuiTextFailureOriginRuntime* runtime(NativeGuiTextRawModelSectionStorageAcquired& a) noexcept { return a.failure_runtime_; }
    static NativeGuiTextFailureOrigin& origin(NativeGuiTextRawModelSectionStorageAcquired& a) noexcept { return a.failure_origin_; }
};
namespace {
struct FailureTransport {
    const NativeGuiTextFailureOriginRuntime* runtime{};
    NativeGuiTextFailureOrigin* origin{};
    const void* supplier{};
};
FailureTransport failure_transport(NativeGuiTextSectionStorageAcquired&) noexcept { return {}; }
FailureTransport failure_transport(NativeGuiTextRawModelSectionStorageAcquired& a) noexcept {
    return {NativeGuiTextRawModelSectionFailureAccess::runtime(a), &NativeGuiTextRawModelSectionFailureAccess::origin(a), &a};
}
template<class Acquired, class F> decltype(auto) source_host(Acquired& a, std::uint32_t source, F&& f) {
    const auto t = failure_transport(a);
    if (t.runtime) return t.origin->host(*t.runtime, {0, source}, t.supplier, std::forward<F>(f));
    return std::forward<F>(f)();
}
template<class Acquired, class F> decltype(auto) source_unknown(Acquired& a, std::uint32_t site,
    std::uint32_t source, F&& f) {
    const auto t = failure_transport(a);
    if (t.runtime) return t.origin->unknown(*t.runtime, {site, source}, t.supplier, std::forward<F>(f));
    return std::forward<F>(f)();
}
using Word = std::uint32_t;
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
void* at(void* storage, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(storage) + offset;
}
volatile Word& word(void* storage, std::size_t offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(storage, offset));
}
void* pointer(void* storage, std::size_t offset) noexcept {
    return reinterpret_cast<void*>(word(storage, offset));
}
void raw_default_branches(void* text) {
    void* const sentinel = pointer(text, 0x68);
    require(!pointer(text, 0x4c) && !pointer(text, 0x70) && word(text, 0x6c) == 0 &&
        sentinel && pointer(sentinel, 0) == sentinel && pointer(sentinel, 4) == sentinel &&
        !word(text, 0x88) && !word(text, 0x8c) && !word(text, 0x90) &&
        !word(text, 0xd8) && !word(text, 0xdc) && !word(text, 0xe0) &&
        !word(text, 0x180) && !word(text, 0x184) && !word(text, 0x18c) && !word(text, 0x190) &&
        !word(text, 0x198) && !word(text, 0x19c) && !word(text, 0x1a0) &&
        !word(text, 0xec) && !word(text, 0xf0) && !word(text, 0xf4) && !word(text, 0xf8) &&
        !word(text, 0x1a4) && !word(text, 0x1a8) && !word(text, 0x1c0) && !word(text, 0x1c4) &&
        !word(text, 0x1c8) && !word(text, 0x1cc) && !word(text, 0x1ec),
        "Raw-Model Text admits only DEFAULT empty primary/parent/child/timed/glyph/content/shader branches");
}
template<class Frame> void* name_header(Frame& f) noexcept {
    return const_cast<Word*>(&f.locals[2]);
}
template<class Acquired> void call(Acquired& a, Word site) noexcept {
    a.active_call_site = site;
    a.failure = NativeGuiTextSectionStorageAcquired::Failure::provider_call;
}
struct Region { const volatile void* begin; std::size_t size; };
bool disjoint(Region left, Region right) noexcept {
    const auto l = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(left.begin));
    const auto r = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(right.begin));
    return l + left.size <= r || r + right.size <= l;
}
GuiNativeGeometryOwners& geometry(NativeGuiTextSectionStorageContext& c) { return c.actual.geometry; }
GuiNativeGeometryOwners& geometry(NativeGuiTextRawModelSectionStorageContext& c) { return c.geometry; }
NativeMaterialDestructionAccess& materials(NativeGuiTextSectionStorageContext& c) { return c.actual.materials; }
NativeMaterialDestructionAccess& materials(NativeGuiTextRawModelSectionStorageContext& c) { return c.materials; }
const volatile Word* material_table(NativeGuiTextSectionStorageContext& c) { return c.actual.material_vtable_00d5e520; }
const volatile Word* material_table(NativeGuiTextRawModelSectionStorageContext& c) { return c.material_vtable_00d5e520; }
NativeTextureSurfaceReferenceIncrement const volatile& decrement(NativeGuiTextSectionStorageContext& c) {
    return c.decrement_00ce2220;
}
NativeTextureSurfaceReferenceIncrement const volatile& decrement(NativeGuiTextRawModelSectionStorageContext& c) {
    return c.model.storage.node.trees.decrement_00ce2220;
}
const volatile Word& unchanged(NativeGuiTextSectionStorageContext& c) { return c.actual.unchanged_00d7a260; }
const volatile Word& unchanged(NativeGuiTextRawModelSectionStorageContext& c) { return c.association.unchanged_00d7a260; }
NativeModelOwner& model(void* raw, NativeGuiTextSectionStorageContext& c) {
    require(raw != nullptr, "Text sections require nonnull actual Model storage");
    auto* reference = dynamic_cast<NativeModelReference*>(
        &c.actual.geometry.actual_owners().resolve_actual(raw));
    require(reference && &reference->model_owner().storage.node == raw &&
        &reference->model_owner().environment == &c.actual.models &&
        reference->model_owner().phase == NativeModelOwner::Phase::live,
        "Text section Model must be the same live canonical actual owner");
    return reference->model_owner();
}
void preflight(void* text, NativeGuiTextSectionStorageFrame& f,
    NativeGuiTextSectionStorageContext& c, NativeGuiTextSectionStorageAcquired& a) {
    auto& actual = c.actual;
    auto& owners = actual.geometry.actual_owners();
    auto& node = c.parenting.node;
    const auto mesh_constants = actual.geometry.mesh_constants();
    require(text && (reinterpret_cast<std::uintptr_t>(text) & 3u) == 0 &&
        !a.started && !a.complete && a.failure == NativeGuiTextSectionStorageAcquired::Failure::none &&
        !a.model_owner && !a.model_creator && !a.mesh.started && !a.shadow_section.section.started &&
        !a.main_section.section.started && !a.parenting.started &&
        a.shadow_section.material.phase == NativeMaterialFactoryAcquired::Phase::empty &&
        a.main_section.material.phase == NativeMaterialFactoryAcquired::Phase::empty,
        "Text section entry requires fresh diagnostics and aligned actual storage");
    const Region writes[] = {{text, 0x1f4}, {&f, sizeof(f)}, {&a, sizeof(a)},
        {&f.locals[0], sizeof(f.locals)}, {&f.parenting, sizeof(f.parenting)}};
    for (std::size_t i = 0; i != 5; ++i) {
        for (std::size_t j = i + 1; j != 5; ++j)
            require(disjoint(writes[i], writes[j]), "Text section caller storage must be disjoint");
        for (const Region borrowed : {Region{&c, sizeof(c)}, {&actual, sizeof(actual)},
            {&c.node_constants, sizeof(c.node_constants)}, {&c.strings, sizeof(c.strings)},
            {&c.string_process, sizeof(c.string_process)},
            {&c.effects, sizeof(c.effects)}, {&c.parenting, sizeof(c.parenting)},
            {&c.decrement_00ce2220, sizeof(c.decrement_00ce2220)},
            {&actual.unchanged_00d7a260, sizeof(actual.unchanged_00d7a260)},
            {&c.effects.effects.construction.current_renderer_00f8d394, sizeof(void*)}})
            require(disjoint(writes[i], borrowed),
                "Text section caller writes must not alias borrowed provider/import storage");
    }
    require(actual.prepare_model && actual.retire_failed_model && actual.bind_completed_model &&
        !actual.models.actual_names && actual.models.nodes.uses_raw_name_pool() &&
        &actual.models.nodes.require_raw_name_pool() == &c.strings && &node.strings == &c.strings &&
        &c.strings == &c.string_process.raw_context() &&
        &actual.strings == &c.string_process.strings() &&
        &actual.models.retained_owners == &owners && &actual.materials.retained_owners == &owners &&
        &node.scenes.owners == &owners && &node.trees.owners == &owners &&
        &node.scenes.decrement_00ce2220 == &c.decrement_00ce2220 &&
        &node.trees.decrement_00ce2220 == &c.decrement_00ce2220 && c.decrement_00ce2220 &&
        &c.effects.effects.owners.actual_owners() == &owners &&
        &c.effects.strings == &actual.strings && &actual.materials.parameter_names == &actual.strings &&
        &c.effects.effects.construction.strings == &actual.strings &&
        c.effects.effects.cache == &c.effects && c.shadow_00d15ec4 && c.guidefault_00d5c25c &&
        c.effects.effects.construction.current_renderer_00f8d394 &&
        word(c.effects.effects.construction.current_renderer_00f8d394) == 0x00d5f0a8u &&
        c.effects.effects.renderer_profile_00d5f0a8 &&
        c.effects.effects.renderer_profile_00d5f0a8[0x48 / 4] == 0x00b318b0u &&
        actual.material_vtable_00d5e520 &&
        &actual.models.constants.maximum_00ce4970 == &c.node_constants.positive_bound_00ce4970 &&
        &actual.models.constants.minimum_00ce4adc == &c.node_constants.negative_bound_00ce4adc &&
        &mesh_constants.maximum_00ce4970 == &c.node_constants.positive_bound_00ce4970 &&
        &mesh_constants.minimum_00ce4adc == &c.node_constants.negative_bound_00ce4adc &&
        &actual.models.constants.unchanged_00d7a260 == &actual.unchanged_00d7a260,
        "Text section providers must share actual raw owner, string, import and constant domains");
    if (void* initial = pointer(text, 0x188)) model(initial, c);
    if (void* initial = pointer(text, 0x4c)) model(initial, c);
}

void preflight(void* text, NativeGuiTextRawModelSectionStorageFrame& f,
    NativeGuiTextRawModelSectionStorageContext& c, NativeGuiTextRawModelSectionStorageAcquired& a) {
    auto& storage = c.model.storage;
    auto& node = storage.node;
    auto& owners = c.geometry.actual_owners();
    const auto mesh = c.geometry.mesh_constants();
    require(text && (reinterpret_cast<std::uintptr_t>(text) & 3u) == 0 && word(text) == 0x00d5c6c8 &&
        !a.started && !a.complete && a.failure == decltype(a.failure)::none &&
        !a.model.actual && !a.model.construction.started && !a.model.reference &&
        !a.model.lifetime.started && !a.model.lifetime.scalar_started && !a.model.release.started &&
        a.model.association.phase == NativeModelStorageGeometryPhase::fresh &&
        f.association.phase() == NativeModelStorageGeometryPhase::fresh &&
        !a.mesh.started && !a.shadow_section.section.started && !a.main_section.section.started &&
        !a.parenting.started &&
        a.shadow_section.material.phase == NativeMaterialFactoryAcquired::Phase::empty &&
        a.main_section.material.phase == NativeMaterialFactoryAcquired::Phase::empty,
        "Raw-Model Text sections require one fresh complete retained metadata graph");
    const Region writes[] = {{text, 0x1f4}, {&f, sizeof(f)}, {&a, sizeof(a)},
        {&f.locals[0], sizeof(f.locals)}, {&f.arguments[0], sizeof(f.arguments)},
        {&f.parenting, sizeof(f.parenting)}};
    for (std::size_t i = 0; i != 6; ++i) {
        for (std::size_t j = i + 1; j != 6; ++j)
            require(disjoint(writes[i], writes[j]), "Raw-Model Text caller regions must be disjoint");
        for (const Region borrowed : {Region{&c, sizeof(c)}, {&c.model, sizeof(c.model)},
            {&storage, sizeof(storage)}, {&node, sizeof(node)}, {&c.association, sizeof(c.association)},
            {&storage.pool_01090054, sizeof(storage.pool_01090054)},
            {&storage.pool_01090054.storage(), sizeof(storage.pool_01090054.storage())},
            {&c.geometry, sizeof(c.geometry)}, {&c.materials, sizeof(c.materials)},
            {&c.material_strings, sizeof(c.material_strings)}, {&c.strings, sizeof(c.strings)},
            {&c.string_process, sizeof(c.string_process)}, {&c.effects, sizeof(c.effects)},
            {&c.parenting, sizeof(c.parenting)}, {&storage.constants, sizeof(storage.constants)},
            {&decrement(c), sizeof(decrement(c))}, {&unchanged(c), sizeof(Word)},
            {&storage.negative_zero_00d7a208, sizeof(Word)}})
            require(disjoint(writes[i], borrowed), "Raw-Model Text writes cannot alias providers");
    }
    require(&c.association.storage == &storage && &c.parenting.node == &node &&
        &node.scenes.owners == &owners && &node.trees.owners == &owners &&
        &c.materials.retained_owners == &owners && &c.effects.effects.owners.actual_owners() == &owners &&
        &node.strings == &c.strings && same_native_string_raw_pool_cells(c.strings, c.string_process.raw_context()) &&
        same_native_string_storage_raw_pool_cells(c.material_strings, c.strings) &&
        &c.effects.strings == &c.material_strings && &c.materials.parameter_names == &c.material_strings &&
        &c.effects.effects.construction.strings == &c.material_strings &&
        &node.scenes.decrement_00ce2220 == &decrement(c) && decrement(c) &&
        &mesh.maximum_00ce4970 == &storage.constants.positive_bound_00ce4970 &&
        &mesh.minimum_00ce4adc == &storage.constants.negative_bound_00ce4adc &&
        c.model.actual_profile_00d62de8 &&
        c.model.actual_profile_00d62de8[0] == 0x00bd30e0 &&
        c.model.actual_profile_00d62de8[1] == 0x00b75290 &&
        c.model.actual_profile_00d62de8[3] == 0x006ef860 &&
        node.world.resolve_profile(0x00d62de8) == c.model.actual_profile_00d62de8 &&
        node.trees.dispatch.resolve_profile(0x00d62de8) == c.model.actual_profile_00d62de8 &&
        c.parenting.dispatch.resolve_profile(0x00d62de8) == c.model.actual_profile_00d62de8 &&
        c.material_vtable_00d5e520 && c.shadow_00d15ec4 && c.guidefault_00d5c25c &&
        c.effects.effects.cache == &c.effects && c.effects.effects.construction.current_renderer_00f8d394 &&
        word(c.effects.effects.construction.current_renderer_00f8d394) == 0x00d5f0a8 &&
        c.effects.effects.renderer_profile_00d5f0a8 &&
        c.effects.effects.renderer_profile_00d5f0a8[0x48 / 4] == 0x00b318b0,
        "Raw-Model Text requires actual shared Model/geometry/material/string/import/provider identities");
    require(!pointer(text, 0x188) && !pointer(text, 0x4c),
        "Raw-Model Text sections admit fresh DEFAULT internal188 construction only");
    raw_default_branches(text);
}

// AB8609..8620: preserve FLD -> CURRENT188 receiver capture -> FST17C ->
// disarm state3 -> FSTP178, including masked signaling-NaN conversion.
__declspec(noinline) void* geometry_arguments(void* text,
    const volatile Word& constant, NativeGuiTextSectionStorageFrame& f,
    std::int32_t& state) noexcept {
    const volatile void* input = &constant;
    void* shadow_slot = at(text, 0x188);
    float* first = &f.scalar_178;
    float* second = &f.scalar_17c;
    auto* state_slot = &state;
    void* result;
    __asm {
        mov eax, input
        fld dword ptr [eax]
        mov edx, shadow_slot
        mov edx, dword ptr [edx]
        mov result, edx
        mov eax, second
        fst dword ptr [eax]
        mov ecx, state_slot
        mov dword ptr [ecx], -1
        mov eax, first
        fstp dword ptr [eax]
    }
    return result;
}
__declspec(noinline) void* geometry_arguments(void* text,
    const volatile Word& constant, NativeGuiTextRawModelSectionStorageFrame& f,
    std::int32_t& state) noexcept {
    const volatile void* input = &constant;
    void* shadow_slot = at(text, 0x188);
    volatile Word* first = &f.arguments[2];
    volatile Word* second = &f.arguments[3];
    auto* state_slot = &state;
    void* result;
    __asm {
        mov eax, input
        fld dword ptr [eax]
        mov edx, shadow_slot
        mov edx, dword ptr [edx]
        mov result, edx
        mov eax, second
        fst dword ptr [eax]
        mov ecx, state_slot
        mov dword ptr [ecx], -1
        mov eax, first
        fstp dword ptr [eax]
    }
    return result;
}
template<class Frame, class Context, class Acquired>
void return_name(Frame& f, Context& c, Acquired& a) {
    // 41DD20's raw pool overload captures nonnull data THEN current length+1,
    // enters419CC0, then BD1510; it never clears or rereads the header.
    destroy_native_string_header_0041dd20(name_header(f), c.strings);
    ++a.completed_name_returns;
}
template<class Frame, class Context, class Acquired>
void return_captured_material_name(void* captured_data, Frame& f, Context& c, Acquired& a) {
    // Normal AB865B/872E captured data BEFORE disarming state4/5. Length is
    // captured AFTER that disarm; never reload data after a getter callback.
    if (captured_data) {
        const Word size = f.locals[2] + 1u;
        auto* pool = native_string_pool_get_or_create_00419cc0(
            c.strings.actual_published_01090aa8, c.strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, captured_data, size,
            c.strings.actual_small_returns_disabled_01090aa4);
    }
    ++a.completed_name_returns;
}
void failed_model(void* raw, NativeGuiTextSectionStorageContext& c,
    NativeGuiTextSectionStorageAcquired& a) {
    if (a.model_owner) {
        auto* abandoned = a.model_owner;
        a.model_owner = nullptr;
        c.actual.retire_failed_model(c.actual.companion_context, *abandoned);
    }
    a.raw_model_return_entered = true;
    c.actual.models.pool_01090054.return_raw_slot_00b74750(raw);
    a.raw_model_returned = true;
}
void failed_model(void* raw, NativeGuiTextRawModelSectionStorageContext& c,
    NativeGuiTextRawModelSectionStorageAcquired& a) {
    require(!a.model.reference && !a.model.construction.complete,
        "Raw Model native constructor unwind cannot return a completed canonical owner");
    a.raw_model_return_entered = true;
    // B75030 already consumed its own node-prefix cleanup. No second base
    // destruction, count change, registration or semantic retirement occurs.
    c.model.storage.pool_01090054.return_raw_slot_00b74750(raw);
    a.raw_model_returned = true;
}
template<class Frame, class Context, class Acquired>
void cleanup(Frame& f, Context& c, Acquired& a) noexcept {
    a.exception_cleanup_started = true;
    try {
        const auto state = a.native_eh_state;
        a.native_eh_state = -1; // consume before any throwing cleanup
        if (state == 1 || state == 2) {
            if (f.locals[0] & 1u) {
                f.locals[0] &= ~Word{1};
                a.conditional_name_consumed = true;
                return_name(f, c, a);
            }
        } else if (state == 4 || state == 5) {
            return_name(f, c, a);
        }
        if (state == 0 || state == 1) {
            // Only native constructor failure reaches this arm. Metadata is
            // retired before returning the CURRENT raw slot, never a live Model.
            failed_model(reinterpret_cast<void*>(f.locals[1]), c, a);
        } else if (state == 3) {
            a.raw_mesh_return_entered = true;
            return_native_mesh_slot_00b72f70(reinterpret_cast<void*>(f.locals[1]));
        }
    } catch (...) { std::terminate(); }
}

template<class Resource, class Context, class Acquired>
void release(Resource*& creator, NativeTextureSurfaceReferenceIncrement decrement,
    NativeGuiTextSectionStorageRelease& progress, Context& c, Acquired& a) {
    // Actual function was captured by the caller at its ORIGINAL import-read
    // site. Never use generic release's atomic or reload CE2220 here.
    void* const raw = std::exchange(creator, nullptr);
    source_host(a, __COUNTER__, [&] {
        require(raw && decrement && !progress.entered, "Text creator release requires one actual reference"); });
    progress.identity = raw;
    progress.captured_decrement = decrement;
    progress.entered = true;
    progress.result = decrement(static_cast<volatile long*>(at(raw, 4)));
    progress.decremented = true;
    if (progress.result == 0) {
        progress.zero_entered = true;
        // Pure canonical lookup validates the SAME actual+04; its terminal
        // dispatches CURRENT native vtable0/profile to genuine destruction.
        dispatch_native_render_actual_owner_zero(geometry(c).actual_owners(), raw);
        progress.zero_returned = true;
    }
    progress.complete = true; // metadata only; no raw access after terminal
}

void material_ready(NativeGuiTextSectionStorageContext&, NativeGuiTextSectionStorageSection&) {}
void material_ready(NativeGuiTextRawModelSectionStorageContext&, NativeGuiTextSectionStorageSection& part) {
    require(part.material_creator && part.material.phase == NativeMaterialFactoryAcquired::Phase::complete &&
        part.material_creator->effect_7c,
        "Raw DEFAULT Text needs the real GuiDefault material/effect, not a null-effect substitute");
}
template<class Frame, class Context, class Acquired>
void section(NativeMeshStorage& mesh, bool shadow, Frame& f, Context& c,
    Acquired& a, NativeGuiTextSectionStorageSection& part) {
    call(a, shadow ? 0x00ab862b : 0x00ab86fe);
    try { part.section_creator = source_unknown(a, shadow ? 0x00ab862bu : 0x00ab86feu, __COUNTER__, [&] {
        return geometry(c).create_native_section_00533fa0(part.section); }); }
    catch (...) {
        if (part.section.creator)
            a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
        throw;
    }
    source_host(a, __COUNTER__, [&] {
        require(part.section_creator, "Native null section's later access is outside supported normal storage"); });
    call(a, shadow ? 0x00ab8635 : 0x00ab8708);
    source_unknown(a, shadow ? 0x00ab8635u : 0x00ab8708u, __COUNTER__, [&] {
        append_native_mesh_draw_section_00b73c60(mesh, part.section_creator); });
    part.appended = true;
    call(a, shadow ? 0x00ab8643 : 0x00ab8716);
    source_unknown(a, shadow ? 0x00ab8643u : 0x00ab8716u, __COUNTER__, [&] {
        construct_native_string_header_0041e870(name_header(f), c.strings, c.guidefault_00d5c25c); });
    a.native_eh_state = shadow ? 4 : 5;
    call(a, shadow ? 0x00ab8654 : 0x00ab8727);
    try { part.material_creator = source_unknown(a, shadow ? 0x00ab8654u : 0x00ab8727u, __COUNTER__, [&] {
        return create_native_material_from_effect_cache_00535320(name_header(f),
            materials(c).material_slots, geometry(c).actual_owners(), c.effects, part.material); }); }
    catch (...) {
        if constexpr (std::is_same_v<Acquired, NativeGuiTextRawModelSectionStorageAcquired>) {
            if (part.material.phase == NativeMaterialFactoryAcquired::Phase::empty ||
                part.material.phase == NativeMaterialFactoryAcquired::Phase::complete)
                a.failure = decltype(a.failure)::host_metadata;
        }
        throw;
    }
    // Native factory has returned; Source admission is not covered by its
    // caller's native name-unwind. Keep every creator/header on admission error.
    a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
    source_host(a, __COUNTER__, [&] {
        material_ready(c, part);
        if (part.material_creator)
            geometry(c).register_native_material_creator(part.material, materials(c), material_table(c));
    });
    void* const captured_name_data = reinterpret_cast<void*>(f.locals[3]); // AB865B/872E
    a.native_eh_state = -1; // AB8661/8734, BEFORE nonnull current-length read
    call(a, shadow ? 0x00ab867d : 0x00ab8750);
    source_unknown(a, shadow ? 0x00ab867du : 0x00ab8750u, __COUNTER__, [&] {
        return_captured_material_name(captured_name_data, f, c, a); });
    call(a, shadow ? 0x00ab8685 : 0x00ab8758);
    source_unknown(a, shadow ? 0x00ab8685u : 0x00ab8758u, __COUNTER__, [&] {
        set_native_mesh_section_material_00b864c0(*part.section_creator,
            geometry(c).actual_owners(), part.material_creator); });
    part.material_published = true;
    if (shadow) {
        call(a, 0x00ab868e);
        const auto first = decrement(c);
        source_unknown(a, 0x00ab868eu, __COUNTER__, [&] { release(part.section_creator, first, part.section_release, c, a); });
        // AB86A0 captures AFTER the section terminal has returned.
        a.shadow_pair_decrement = decrement(c);
        call(a, 0x00ab86aa);
        source_unknown(a, 0x00ab86aau, __COUNTER__, [&] { release(part.material_creator, a.shadow_pair_decrement, part.material_release, c, a); });
    } else {
        // AB875D: SAME captured target for both section and material.
        a.main_pair_decrement = decrement(c);
        call(a, 0x00ab8767);
        source_unknown(a, 0x00ab8767u, __COUNTER__, [&] { release(part.section_creator, a.main_pair_decrement, part.section_release, c, a); });
        call(a, 0x00ab8779);
        source_unknown(a, 0x00ab8779u, __COUNTER__, [&] { release(part.material_creator, a.main_pair_decrement, part.material_release, c, a); });
    }
}
void* allocate_model(NativeGuiTextSectionStorageContext& c) {
    return c.actual.models.pool_01090054.allocate_raw_slot_00b74d00();
}
void* allocate_model(NativeGuiTextRawModelSectionStorageContext& c) {
    return c.model.storage.pool_01090054.allocate_raw_slot_00b74d00();
}
void prepare_model(void* raw, NativeGuiTextSectionStorageContext& c,
    NativeGuiTextSectionStorageAcquired& a) {
    a.model_owner = &c.actual.prepare_model(c.actual.companion_context, raw, c.actual.models);
    require(&a.model_owner->storage.node == raw && &a.model_owner->environment == &c.actual.models &&
        a.model_owner->phase == NativeModelOwner::Phase::prepared,
        "Text Shadow preparation must preserve its SAME unused actual slot");
}
void prepare_model(void* raw, NativeGuiTextRawModelSectionStorageContext&,
    NativeGuiTextRawModelSectionStorageAcquired& a) {
    // The stable record exists before allocation; this only records identity.
    a.model.actual = raw;
}
void* construct_model(void*, void* name, NativeGuiTextSectionStorageContext& c,
    NativeGuiTextSectionStorageAcquired& a) {
    return construct_native_model_00b75030(*a.model_owner, name, c.node_constants);
}
void* construct_model(void* raw, void* name, NativeGuiTextRawModelSectionStorageContext& c,
    NativeGuiTextRawModelSectionStorageAcquired& a) {
    try { return source_unknown(a, 0x00ab859bu, __COUNTER__, [&] {
        return construct_native_model_storage_00b75030(raw, name, c.model.storage, a.model.construction); }); }
    catch (...) {
        if (!a.model.construction.started || a.model.construction.complete)
            a.failure = decltype(a.failure)::host_metadata;
        throw;
    }
}
void bind_model(NativeGuiTextSectionStorageContext& c, NativeGuiTextSectionStorageAcquired& a) {
    a.model_creator = &c.actual.bind_completed_model(c.actual.companion_context, *a.model_owner);
    require(&a.model_creator->model_owner() == a.model_owner,
        "Text Shadow registration must borrow the same actual creator");
}
void bind_model(NativeGuiTextRawModelSectionStorageContext& c, NativeGuiTextRawModelSectionStorageAcquired& a) {
    a.model.reference.emplace(a.model.actual, c.model, a.model.construction,
        a.model.lifetime_frame, a.model.lifetime);
}
void shadow_ready(void*, NativeGuiTextSectionStorageContext&, NativeGuiTextSectionStorageAcquired&) {}
void shadow_ready(void* text, NativeGuiTextRawModelSectionStorageContext& c,
    NativeGuiTextRawModelSectionStorageAcquired& a) {
    a.failure = decltype(a.failure)::host_metadata;
    require(a.model.actual && pointer(text, 0x188) == a.model.actual && a.model.reference &&
        !a.model.reference->retired() && a.model.construction.complete &&
        a.model.construction.metadata_bound && a.model.construction.context == &c.model.storage &&
        !a.model.lifetime.started && !a.model.lifetime.scalar_started && !a.model.release.started &&
        a.model.reference->reference_count.load(std::memory_order_relaxed) > 0 &&
        &c.geometry.actual_owners().resolve_actual(a.model.actual) == &*a.model.reference,
        "Raw-Model Text needs its genuine completed nonnull internal188 before first field access");
}
void associate(NativeGuiTextSectionStorageFrame& f, NativeGuiTextSectionStorageContext& c,
    NativeGuiTextSectionStorageAcquired& a) {
    call(a, 0x00ab8626);
    set_native_model_geometry_00b75170(model(a.captured_shadow_receiver, c), 0,
        a.mesh_creator, f.scalar_178, f.scalar_17c);
}
void associate(NativeGuiTextRawModelSectionStorageFrame& f, NativeGuiTextRawModelSectionStorageContext& c,
    NativeGuiTextRawModelSectionStorageAcquired& a) {
    a.failure = decltype(a.failure)::host_metadata;
    source_host(a, __COUNTER__, [&] { require(a.model.reference && a.model.actual == a.captured_shadow_receiver &&
        !a.model.reference->retired() && a.model.construction.complete &&
        a.model.construction.metadata_bound && a.model.construction.context == &c.model.storage &&
        !a.model.lifetime.started && !a.model.lifetime.scalar_started && !a.model.release.started &&
        a.model.reference->reference_count.load(std::memory_order_relaxed) > 0 &&
        &c.geometry.actual_owners().resolve_actual(a.captured_shadow_receiver) == &*a.model.reference,
        "Text B75170 receiver must be its SAME completed canonical raw Model"); });
    f.arguments[1] = reinterpret_cast<Word>(a.mesh_creator); // AB8623 PUSH EBX
    f.arguments[0] = 0; // AB8624 PUSH 0
    call(a, 0x00ab8626);
    try { source_unknown(a, 0x00ab8626u, __COUNTER__, [&] {
        set_native_model_geometry_storage_00b75170(a.captured_shadow_receiver,
            f.association, c.association, a.model.association); }); }
    catch (...) {
        if (f.association.phase() == NativeModelStorageGeometryPhase::fresh)
            a.failure = decltype(a.failure)::host_metadata;
        throw;
    }
}
void main_sections(void* text, NativeGuiTextSectionStorageFrame& f,
    NativeGuiTextSectionStorageContext& c, NativeGuiTextSectionStorageAcquired& a) {
    // AB86CB: CURRENT raw field AFTER ALL Shadow/release callbacks.
    if (void* main = pointer(text, 0x4c)) {
        call(a, 0x00ab86d6);
        model(main, c); // pure canonical metadata; actual B74650 reads180
        if (pointer(main, 0x180)) {
            call(a, 0x00ab86e8);
            main = pointer(text, 0x4c); // B74640 uses freshly loaded receiver
            model(main, c);
            auto* mesh = static_cast<NativeMeshStorage*>(pointer(main, 0x180));
            call(a, 0x00ab86f1);
            if (word(mesh, 0x58) == 0) section(*mesh, false, f, c, a, a.main_section);
        }
        void* requested = *static_cast<volatile unsigned char*>(at(text, 0x15c))
            ? pointer(text, 0x4c) : nullptr;
        void* shadow = pointer(text, 0x188);
        f.parenting.parent_argument = reinterpret_cast<Word>(requested);
        call(a, 0x00ab879e);
        set_native_raw_parent_00b6e680(shadow, f.parenting, c.parenting, a.parenting);
        if (*static_cast<volatile unsigned char*>(at(text, 0x15c)) == 0) {
            shadow = pointer(text, 0x188);
            call(a, 0x00ab87b4);
            propagate_native_node_root_storage_00b6d890(shadow, nullptr, c.parenting.node.scenes.children);
        }
    }
}
void main_sections(void* text, NativeGuiTextRawModelSectionStorageFrame&,
    NativeGuiTextRawModelSectionStorageContext& c, NativeGuiTextRawModelSectionStorageAcquired& a) {
    a.failure = decltype(a.failure)::host_metadata;
    raw_default_branches(text);
    shadow_ready(text, c, a);
}
template<class Frame, class Context, class Acquired>
void sections_body(void* text, Frame& f, Context& c, Acquired& a) {
    source_host(a, __COUNTER__, [&] { preflight(text, f, c, a); });
    a.started = true;
    f.locals[0] = 0; // AB854F; preserve raw allocation and header preimages
    try {
        if (!pointer(text, 0x188)) {
            call(a, 0x00ab8565);
            void* raw = source_unknown(a, 0x00ab8565u, __COUNTER__, [&] { return allocate_model(c); });
            f.locals[1] = reinterpret_cast<Word>(raw);
            a.native_eh_state = 0;
            if (raw) {
                a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
                source_host(a, __COUNTER__, [&] { prepare_model(raw, c, a); });
                call(a, 0x00ab8581);
                source_unknown(a, 0x00ab8581u, __COUNTER__, [&] {
                    construct_native_string_header_0041e870(name_header(f), c.strings, c.shadow_00d15ec4); });
                f.locals[0] = 1; // armed only AFTER native name construction returns
                a.native_eh_state = 1;
                call(a, 0x00ab859b);
                raw = construct_model(raw, name_header(f), c, a);
                a.model_constructed = true;
            }
            word(text, 0x188) = reinterpret_cast<Word>(raw); // AB85AA before name release
            a.shadow_published = true;
            a.native_eh_state = -1;
            if (a.model_constructed) {
                a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
                source_host(a, __COUNTER__, [&] { bind_model(c, a); });
                a.model_registered = true;
            }
            if (f.locals[0] & 1u) {
                call(a, 0x00ab85d0);
                source_unknown(a, 0x00ab85d0u, __COUNTER__, [&] { return_name(f, c, a); });
            }
            source_host(a, __COUNTER__, [&] { shadow_ready(text, c, a); });
            call(a, 0x00ab85db);
            word(pointer(text, 0x188), 0x138) &= ~Word{3};
            call(a, 0x00ab85e7);
            try { a.mesh_creator = source_unknown(a, 0x00ab85e7u, __COUNTER__, [&] {
                return geometry(c).create_native_mesh_00b73b60(a.mesh); }); }
            catch (...) {
                if (a.mesh.native_site == 0 || a.mesh.constructor_complete)
                    a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
                throw;
            }
            f.locals[1] = reinterpret_cast<Word>(a.mesh_creator);
            a.native_eh_state = 3;
            a.captured_shadow_receiver = geometry_arguments(text, unchanged(c),
                f, a.native_eh_state);
            associate(f, c, a);
            source_host(a, __COUNTER__, [&] {
                require(a.mesh_creator, "Native null mesh's later access is outside supported normal storage"); });
            section(*a.mesh_creator, true, f, c, a, a.shadow_section);
            call(a, 0x00ab86bd);
            source_unknown(a, 0x00ab86bdu, __COUNTER__, [&] { release(a.mesh_creator, a.shadow_pair_decrement, a.mesh_release, c, a); });
        }
        if constexpr (std::is_same_v<Acquired, NativeGuiTextRawModelSectionStorageAcquired>)
            source_host(a, __COUNTER__, [&] { main_sections(text, f, c, a); });
        else main_sections(text, f, c, a);
        a.complete = true;
        a.failure = NativeGuiTextSectionStorageAcquired::Failure::none;
    } catch (...) {
        const auto t = failure_transport(a);
        if (t.runtime)
            t.origin->retain_current(*t.runtime); // private HOST/UNKNOWN before legacy diagnostics
        else if (a.failure != NativeGuiTextSectionStorageAcquired::Failure::host_metadata)
            cleanup(f, c, a); // historical noexcept/secondary projection ONLY on legacy route
        throw;
    }
}
} // namespace

void ensure_native_gui_text_sections_storage_00ab8530(void* text,
    NativeGuiTextSectionStorageFrame& f, NativeGuiTextSectionStorageContext& c,
    NativeGuiTextSectionStorageAcquired& a) { sections_body(text, f, c, a); }
void ensure_native_gui_text_sections_storage_00ab8530(void* text,
    NativeGuiTextRawModelSectionStorageFrame& f, NativeGuiTextRawModelSectionStorageContext& c,
    NativeGuiTextRawModelSectionStorageAcquired& a) {
    try { sections_body(text, f, c, a); }
    catch (...) {
        if (!a.started) a.failure = decltype(a.failure)::host_metadata;
        throw;
    }
}
void ensure_native_gui_text_sections_storage_00ab8530(void* text,
    NativeGuiTextRawModelSectionStorageFrame& f, NativeGuiTextRawModelSectionStorageContext& c,
    NativeGuiTextRawModelSectionStorageAcquired& a, const NativeGuiTextFailureOriginRuntime& runtime) {
    if (!a.failure_runtime_) a.failure_runtime_ = &runtime; // never replace a retained service
    const auto& retained_runtime = *a.failure_runtime_;
    a.failure_origin_.begin(runtime, &a, 0x00ab8530u);
    try {
        sections_body(text, f, c, a);
        a.failure_origin_.complete();
    } catch (...) {
        a.failure_origin_.retain_current(retained_runtime); // exact current cause; sticky denial retained
        if (!a.started) a.failure = decltype(a.failure)::host_metadata;
        throw;
    }
}
} // namespace bsp
