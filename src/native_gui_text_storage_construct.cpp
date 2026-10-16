#include "bsp/native_gui_text_storage_construct.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <exception>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text sections require MSVC Win32.
#endif

namespace bsp {
namespace {
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
void* name_header(NativeGuiTextSectionStorageFrame& f) noexcept {
    return const_cast<Word*>(&f.locals[2]);
}
void call(NativeGuiTextSectionStorageAcquired& a, Word site) noexcept {
    a.active_call_site = site;
    a.failure = NativeGuiTextSectionStorageAcquired::Failure::provider_call;
}
struct Region { const volatile void* begin; std::size_t size; };
bool disjoint(Region left, Region right) noexcept {
    const auto l = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(left.begin));
    const auto r = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(right.begin));
    return l + left.size <= r || r + right.size <= l;
}
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
void return_name(NativeGuiTextSectionStorageFrame& f,
    NativeGuiTextSectionStorageContext& c, NativeGuiTextSectionStorageAcquired& a) {
    // 41DD20's raw pool overload captures nonnull data THEN current length+1,
    // enters419CC0, then BD1510; it never clears or rereads the header.
    destroy_native_string_header_0041dd20(name_header(f), c.strings);
    ++a.completed_name_returns;
}
void return_captured_material_name(void* captured_data, NativeGuiTextSectionStorageFrame& f,
    NativeGuiTextSectionStorageContext& c, NativeGuiTextSectionStorageAcquired& a) {
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
void cleanup(NativeGuiTextSectionStorageFrame& f,
    NativeGuiTextSectionStorageContext& c, NativeGuiTextSectionStorageAcquired& a) noexcept {
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
            if (a.model_owner) {
                auto* abandoned = a.model_owner;
                a.model_owner = nullptr;
                c.actual.retire_failed_model(c.actual.companion_context, *abandoned);
            }
            a.raw_model_return_entered = true;
            c.actual.models.pool_01090054.return_raw_slot_00b74750(
                reinterpret_cast<void*>(f.locals[1]));
            a.raw_model_returned = true;
        } else if (state == 3) {
            a.raw_mesh_return_entered = true;
            return_native_mesh_slot_00b72f70(reinterpret_cast<void*>(f.locals[1]));
        }
    } catch (...) { std::terminate(); }
}

template<class Resource>
void release(Resource*& creator, NativeTextureSurfaceReferenceIncrement decrement,
    NativeGuiTextSectionStorageRelease& progress, NativeGuiTextSectionStorageContext& c) {
    // Actual function was captured by the caller at its ORIGINAL import-read
    // site. Never use generic release's atomic or reload CE2220 here.
    void* const raw = std::exchange(creator, nullptr);
    require(raw && decrement && !progress.entered, "Text creator release requires one actual reference");
    progress.identity = raw;
    progress.captured_decrement = decrement;
    progress.entered = true;
    progress.result = decrement(static_cast<volatile long*>(at(raw, 4)));
    progress.decremented = true;
    if (progress.result == 0) {
        progress.zero_entered = true;
        // Pure canonical lookup validates the SAME actual+04; its terminal
        // dispatches CURRENT native vtable0/profile to genuine destruction.
        dispatch_native_render_actual_owner_zero(c.actual.geometry.actual_owners(), raw);
        progress.zero_returned = true;
    }
    progress.complete = true; // metadata only; no raw access after terminal
}

void section(NativeMeshStorage& mesh, bool shadow,
    NativeGuiTextSectionStorageFrame& f, NativeGuiTextSectionStorageContext& c,
    NativeGuiTextSectionStorageAcquired& a, NativeGuiTextSectionStorageSection& part) {
    call(a, shadow ? 0x00ab862b : 0x00ab86fe);
    try { part.section_creator = c.actual.geometry.create_native_section_00533fa0(part.section); }
    catch (...) {
        if (part.section.creator)
            a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
        throw;
    }
    require(part.section_creator, "Native null section's later access is outside supported normal storage");
    call(a, shadow ? 0x00ab8635 : 0x00ab8708);
    append_native_mesh_draw_section_00b73c60(mesh, part.section_creator);
    part.appended = true;
    call(a, shadow ? 0x00ab8643 : 0x00ab8716);
    construct_native_string_header_0041e870(name_header(f), c.strings, c.guidefault_00d5c25c);
    a.native_eh_state = shadow ? 4 : 5;
    call(a, shadow ? 0x00ab8654 : 0x00ab8727);
    part.material_creator = create_native_material_from_effect_cache_00535320(name_header(f),
        c.actual.materials.material_slots, c.actual.geometry.actual_owners(), c.effects, part.material);
    // Native factory has returned; Source admission is not covered by its
    // caller's native name-unwind. Keep every creator/header on admission error.
    a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
    if (part.material_creator)
        c.actual.geometry.register_native_material_creator(part.material,
            c.actual.materials, c.actual.material_vtable_00d5e520);
    void* const captured_name_data = reinterpret_cast<void*>(f.locals[3]); // AB865B/872E
    a.native_eh_state = -1; // AB8661/8734, BEFORE nonnull current-length read
    call(a, shadow ? 0x00ab867d : 0x00ab8750);
    return_captured_material_name(captured_name_data, f, c, a);
    call(a, shadow ? 0x00ab8685 : 0x00ab8758);
    set_native_mesh_section_material_00b864c0(*part.section_creator,
        c.actual.geometry.actual_owners(), part.material_creator);
    part.material_published = true;
    if (shadow) {
        call(a, 0x00ab868e);
        const auto first = c.decrement_00ce2220;
        release(part.section_creator, first, part.section_release, c);
        // AB86A0 captures AFTER the section terminal has returned.
        a.shadow_pair_decrement = c.decrement_00ce2220;
        call(a, 0x00ab86aa);
        release(part.material_creator, a.shadow_pair_decrement, part.material_release, c);
    } else {
        // AB875D: SAME captured target for both section and material.
        a.main_pair_decrement = c.decrement_00ce2220;
        call(a, 0x00ab8767);
        release(part.section_creator, a.main_pair_decrement, part.section_release, c);
        call(a, 0x00ab8779);
        release(part.material_creator, a.main_pair_decrement, part.material_release, c);
    }
}
} // namespace

void ensure_native_gui_text_sections_storage_00ab8530(void* text,
    NativeGuiTextSectionStorageFrame& f, NativeGuiTextSectionStorageContext& c,
    NativeGuiTextSectionStorageAcquired& a) {
    preflight(text, f, c, a);
    a.started = true;
    f.locals[0] = 0; // AB854F; preserve raw allocation and header preimages
    try {
        if (!pointer(text, 0x188)) {
            call(a, 0x00ab8565);
            void* raw = c.actual.models.pool_01090054.allocate_raw_slot_00b74d00();
            f.locals[1] = reinterpret_cast<Word>(raw);
            a.native_eh_state = 0;
            if (raw) {
                a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
                a.model_owner = &c.actual.prepare_model(c.actual.companion_context, raw, c.actual.models);
                require(&a.model_owner->storage.node == raw &&
                    &a.model_owner->environment == &c.actual.models &&
                    a.model_owner->phase == NativeModelOwner::Phase::prepared,
                    "Text Shadow preparation must preserve its SAME unused actual slot");
                call(a, 0x00ab8581);
                construct_native_string_header_0041e870(name_header(f), c.strings, c.shadow_00d15ec4);
                f.locals[0] = 1; // armed only AFTER native name construction returns
                a.native_eh_state = 1;
                call(a, 0x00ab859b);
                raw = construct_native_model_00b75030(*a.model_owner, name_header(f), c.node_constants);
                a.model_constructed = true;
            }
            word(text, 0x188) = reinterpret_cast<Word>(raw); // AB85AA before name release
            a.shadow_published = true;
            a.native_eh_state = -1;
            if (a.model_constructed) {
                a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
                a.model_creator = &c.actual.bind_completed_model(c.actual.companion_context, *a.model_owner);
                require(&a.model_creator->model_owner() == a.model_owner,
                    "Text Shadow registration must borrow the same actual creator");
                a.model_registered = true;
            }
            if (f.locals[0] & 1u) {
                call(a, 0x00ab85d0);
                return_name(f, c, a);
            }
            call(a, 0x00ab85db);
            word(pointer(text, 0x188), 0x138) &= ~Word{3};
            call(a, 0x00ab85e7);
            try { a.mesh_creator = c.actual.geometry.create_native_mesh_00b73b60(a.mesh); }
            catch (...) {
                if (a.mesh.native_site == 0 || a.mesh.constructor_complete)
                    a.failure = NativeGuiTextSectionStorageAcquired::Failure::host_metadata;
                throw;
            }
            f.locals[1] = reinterpret_cast<Word>(a.mesh_creator);
            a.native_eh_state = 3;
            a.captured_shadow_receiver = geometry_arguments(text, c.actual.unchanged_00d7a260,
                f, a.native_eh_state);
            call(a, 0x00ab8626);
            set_native_model_geometry_00b75170(model(a.captured_shadow_receiver, c), 0,
                a.mesh_creator, f.scalar_178, f.scalar_17c);
            require(a.mesh_creator, "Native null mesh's later access is outside supported normal storage");
            section(*a.mesh_creator, true, f, c, a, a.shadow_section);
            call(a, 0x00ab86bd);
            release(a.mesh_creator, a.shadow_pair_decrement, a.mesh_release, c);
        }
        // AB86CB: CURRENT raw field AFTER ALL Shadow/release callbacks.
        if (void* main = pointer(text, 0x4c)) {
            call(a, 0x00ab86d6);
            model(main, c); // pure canonical metadata; actual B74650 reads180
            if (pointer(main, 0x180)) {
                call(a, 0x00ab86e8);
                main = pointer(text, 0x4c); // B74640 uses the freshly loaded receiver
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
                propagate_native_node_root_storage_00b6d890(shadow, nullptr,
                    c.parenting.node.scenes.children);
            }
        }
        a.complete = true;
        a.failure = NativeGuiTextSectionStorageAcquired::Failure::none;
    } catch (...) {
        if (a.failure != NativeGuiTextSectionStorageAcquired::Failure::host_metadata)
            cleanup(f, c, a);
        throw;
    }
}
} // namespace bsp
