#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_material_compiler_providers.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <array>
#include <charconv>
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
bool enabled(NativeMaterialPassBaseStorage& pass, std::uint32_t state) {
    require(pass.render_18 != nullptr, "native pass has no actual render-state owner");
    const auto& rows = pass.render_18->rows_08;
    require(rows.count_04 >= 0 && (!rows.count_04 || rows.data_00),
        "native render-state rows require valid extent");
    const auto* entries = static_cast<const NativeMaterialRenderState*>(rows.data_00);
    for (std::int32_t i = 0; i != rows.count_04; ++i)
        if (entries[i].state == state) return entries[i].value != 0;
    return false;
}
void remove_group(NativeMaterialPassBaseStorage& pass,
    std::initializer_list<std::uint32_t> states) {
    for (const auto state : states) remove_native_material_render_state_00b5ee00(&pass, state);
}
void current_finalize(NativeMaterialPassBaseStorage& pass,
    NativeMaterialEffectProgramsContext& context, NativeMaterialProgramChild& child) {
    // Verified actual D61BE8 and D62A80 both have current0C=B5F6A0.
    require(pass.root.vtable_00 == 0x00d61be8 || pass.root.vtable_00 == 0x00d62a80,
        "native pass current0C profile is not reconstructed");
    finalize_native_material_pass_00b5f6a0(pass, context, child);
}
void release_string(NativeString& name, NativeStringStorage& strings) noexcept {
    destroy_native_string_header_0041dd20(&name, strings);
    // Dead local storage may be reused only after its actual release callback.
    ::new (&name) NativeString;
}
void append_string(NativeString& left, const NativeString& right, NativeStringStorage& strings) {
    const auto count = right.length();
    if (!count) return;
    const auto offset = left.length();
    resize_native_string_header_0041dd40(&left, strings, offset + count, true);
    std::memcpy(left.data() + offset, right.data(), count);
}
struct LoadFrame {
    std::array<NativeShaderDescriptorStorage*,17> descriptors{};
    std::array<std::optional<NativeShaderDescriptorCallableBinding>,17> bindings;
    std::uint32_t descriptor_count{};
    NativeString stem, mode_name, key, shadow_name, shadow_literal;
    NativeString temporary, number, joined;
    NativeMaterialProgramChild child;
    std::uint32_t call_site{};
    std::uint32_t primary_written{};
    bool slots_initialized{};
    bool returned{};
    std::uint8_t result{}, policy{};

    NativeShaderDescriptorStorage* acquire(NativeStringStorage& strings) {
        require(descriptor_count < descriptors.size(), "descriptor acquisition extent exceeded");
        const auto index = descriptor_count++;
        auto*& descriptor = descriptors[index];
        // Metadata slots are preallocated in the retained frame. Publish the
        // allocation there before constructor/binding can throw.
        descriptor = static_cast<NativeShaderDescriptorStorage*>(singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0x110, 0x110}));
        if (!descriptor) return nullptr;
        initialize_native_shader_descriptor_00b43700(descriptor);
        bindings[index].emplace(*descriptor, strings);
        return descriptor;
    }
    void delete_descriptor(NativeShaderDescriptorStorage* descriptor) {
        require(descriptor != nullptr, "native descriptor direct deletion requires actual storage");
        using Delete = void* (__thiscall*)(void*, std::uint32_t);
        const auto* table = reinterpret_cast<const std::uintptr_t*>(descriptor->vtable_00);
        reinterpret_cast<Delete>(table[0])(descriptor, 1);
    }
    bool live_bindings() const noexcept {
        for (const auto& binding : bindings) if (binding && binding->bound()) return true;
        return false;
    }
    bool live_locals() const noexcept {
        return stem.data() || mode_name.data() || key.data() || shadow_name.data()
            || shadow_literal.data() || temporary.data() || number.data() || joined.data() || child;
    }
};
NativeShaderDescriptorStorage& current_descriptor(NativeMaterialEffectStorage& effect) {
    require(effect.descriptor_c4 != nullptr, "native effect descriptor allocation returned null");
    return *static_cast<NativeShaderDescriptorStorage*>(effect.descriptor_c4);
}
void read_descriptor(LoadFrame& frame, NativeShaderDescriptorStorage* descriptor,
    const void* name, std::uint32_t site, NativeMaterialEffectProgramsContext& context) {
    frame.call_site = site;
    require(descriptor != nullptr, "native descriptor reader requires actual110h storage");
    context.children.read_descriptor_00b43b00(*descriptor, name, 3, frame.child);
    frame.child.reset();
}
void resolve_name(LoadFrame& frame, NativeString& name, std::uint32_t site,
    NativeMaterialEffectProgramsContext& context) {
    frame.call_site = site;
    (void)context.children.resolve_name_00bdf4c0(context.current_vfs_0109ceec, name, frame.child);
    frame.child.reset();
}
void finish_key(LoadFrame& frame, NativeMaterialEffectProgramsContext& context) {
    construct_native_string_cstring_0041e870(&frame.temporary, frame.policy ? "T" : "F", context.strings);
    append_string(frame.key, frame.temporary, context.strings);
    release_string(frame.temporary, context.strings);
    construct_native_material_program_number_00711370(frame.number, 3, context.strings);
    append_string(frame.key, frame.number, context.strings);
    release_string(frame.number, context.strings);
}
NativeMaterialPassStorage* build_program(LoadFrame& frame, NativeMaterialEffectStorage& effect,
    NativeShaderDescriptorStorage& descriptor, NativeShaderDescriptorStorage& mode,
    std::uint8_t mode_flag, std::uint8_t descriptor_flag, std::uint32_t site,
    NativeMaterialEffectProgramsContext& context) {
    frame.call_site = site;
    auto* pass = context.children.build_program_00b3c3a0(
        {effect, descriptor, mode, mode_flag, mode.rt_count_108, descriptor_flag,
         frame.key, frame.policy, 3}, frame.child);
    frame.child.reset();
    return pass;
}
std::uint8_t load_native_material_effect_programs_00b45ee0(NativeMaterialEffectStorage& effect, const void* name,
    std::uint8_t policy, std::uint32_t override_policy, NativeMaterialEffectProgramsContext& context,
    LoadFrame& frame) {
    if (!override_policy) {
        const auto* manager = static_cast<const volatile std::uint8_t*>(context.current_manager_00f8bbf0);
        require(manager != nullptr, "B45EE0 requires current material manager+0E");
        policy = manager[0x0e];
    }
    frame.policy = policy;
    frame.call_site = 0x00b45f29;
    effect.descriptor_c4 = frame.acquire(context.strings); // B45F58 before read
    read_descriptor(frame, static_cast<NativeShaderDescriptorStorage*>(effect.descriptor_c4),
        name, 0x00b45f5e, context);
    auto& root = current_descriptor(effect);
    effect.base.word_b0 = root.priority_08;
    effect.base.word_ac = root.pipe_id_04;
    copy_native_string_header_00be0a30_fragment(&frame.stem, context.strings, name);
    const auto dot = reverse_find_native_string_bytes_004bcb80(frame.stem, ".", 0x7fffffff);
    if (dot != 0xffffffffu) {
        construct_native_string_substring_00469840(&frame.stem, &frame.temporary, 0, dot, context.strings);
        copy_native_string_header_00be0a30_fragment(&frame.stem, context.strings, &frame.temporary);
        release_string(frame.temporary, context.strings);
    }
    for (std::uint32_t mode_index = 0; mode_index != 14; ++mode_index) {
        if (!current_descriptor(effect).mode_names_48[mode_index].length()) {
            effect.passes_c8[mode_index] = nullptr;
            ++frame.primary_written;
            continue;
        }
        frame.call_site = 0x00b46070;
        auto* mode = frame.acquire(context.strings);
        copy_native_string_header_00be0a30_fragment(&frame.mode_name, context.strings,
            &current_descriptor(effect).mode_names_48[mode_index]);
        resolve_name(frame, frame.mode_name, 0x00b460f6, context);
        read_descriptor(frame, mode, &frame.mode_name, 0x00b46104, context);
        std::memcpy(mode->opaque_10c, &mode_index, 4);
        copy_native_string_header_00be0a30_fragment(&frame.key, context.strings, &frame.stem);
        construct_native_material_program_number_00711370(frame.number, mode_index, context.strings);
        concatenate_native_string_headers_004261a0(&frame.key, &frame.joined, &frame.number, context.strings);
        copy_native_string_header_00be0a30_fragment(&frame.key, context.strings, &frame.joined);
        release_string(frame.joined, context.strings);
        release_string(frame.number, context.strings);
        finish_key(frame, context);
        auto& current_root = current_descriptor(effect);
        auto* pass = build_program(frame, effect, current_root, *mode,
            std::to_integer<std::uint8_t>(mode->flags_44[1]),
            std::to_integer<std::uint8_t>(current_root.flags_44[0]), 0x00b4631e, context);
        if (!pass) {
            frame.call_site = 0x00b46445;
            frame.delete_descriptor(mode);
            release_string(frame.key, context.strings);
            release_string(frame.mode_name, context.strings);
            release_string(frame.stem, context.strings);
            frame.returned = true;
            return 0; // current primary slot and remaining preimages untouched
        }
        effect.passes_c8[mode_index] = pass;
        ++frame.primary_written;
        set_native_material_pass_effect_00b172b0(pass->base.root, &effect);
        frame.call_site = 0x00b46341;
        frame.delete_descriptor(mode);
        release_string(frame.key, context.strings);
        release_string(frame.mode_name, context.strings);
    }
    frame.call_site = 0x00b463b6;
    build_native_material_secondary_pass_00b45e00(effect, context.pass_construction,
        context.pass_copy, context.pass_registration);
    frame.slots_initialized = true;
    for (std::size_t i = 0; i != 14; ++i) {
        if (auto* pass = static_cast<NativeMaterialPassStorage*>(effect.passes_c8[i])) {
            frame.call_site = 0x00b463d2;
            current_finalize(pass->base, context, frame.child);
        }
        if (auto* pass = static_cast<NativeMaterialPassStorage*>(effect.secondary_100[i])) {
            frame.call_site = 0x00b463df;
            current_finalize(pass->base, context, frame.child);
        }
    }
    if (auto* pass = static_cast<NativeMaterialPassStorage*>(effect.passes_c8[0])) {
        frame.call_site = 0x00b463f6;
        retain_native_material_effect_pass_00b17dd0(effect, *pass, context, frame.child);
    }
    if (current_descriptor(effect).opaque_14[0] != std::byte{0}) {
        frame.call_site = 0x00b46410;
        auto* passthrough = frame.acquire(context.strings);
        construct_native_string_cstring_0041e870(&frame.shadow_literal, "shadow_passtrough.shfx", context.strings);
        // Native B464D4 requests22 bytes, then copies23 bytes from D61C6C.
        resolve_name(frame, frame.shadow_literal, 0x00b4651a, context);
        read_descriptor(frame, passthrough, &frame.shadow_literal, 0x00b46528, context);
        const std::uint32_t shadow_mode = 2;
        std::memcpy(passthrough->opaque_10c, &shadow_mode, 4);
        copy_native_string_header_00be0a30_fragment(&frame.shadow_name, context.strings,
            &current_descriptor(effect).name_100);
        resolve_name(frame, frame.shadow_name, 0x00b46589, context);
        frame.call_site = 0x00b46593;
        auto* shadow = frame.acquire(context.strings);
        read_descriptor(frame, shadow, &frame.shadow_name, 0x00b465c3, context);
        std::memcpy(shadow->opaque_10c, &shadow_mode, 4);
        std::memcpy(current_descriptor(effect).opaque_14 + 0x10, shadow->opaque_14 + 0x0c, 4);
        construct_native_string_cstring_0041e870(&frame.temporary, "sha", context.strings);
        concatenate_native_string_headers_004261a0(&frame.stem, &frame.key, &frame.temporary, context.strings);
        release_string(frame.temporary, context.strings);
        finish_key(frame, context);
        auto* pass = build_program(frame, effect, *shadow, *passthrough, 1, 0, 0x00b46795, context);
        effect.retained_138 = pass; // publication even on native null result
        if (pass) {
            frame.call_site = 0x00b46821;
            current_finalize(pass->base, context, frame.child);
            pass = static_cast<NativeMaterialPassStorage*>(effect.retained_138);
            set_native_material_pass_effect_00b172b0(pass->base.root, &effect);
            effect.passes_c8[2] = effect.retained_138; // native does not release old primary2
            static_cast<NativeMaterialPassStorage*>(effect.retained_138)->base.root.references_04.fetch_add(1);
        }
        frame.call_site = pass ? 0x00b46853 : 0x00b467ac;
        frame.delete_descriptor(passthrough);
        frame.call_site = pass ? 0x00b4685d : 0x00b467b6;
        frame.delete_descriptor(shadow);
        release_string(frame.key, context.strings);
        release_string(frame.shadow_name, context.strings);
        release_string(frame.shadow_literal, context.strings);
        if (!pass) {
            release_string(frame.stem, context.strings);
            frame.returned = true;
            return 0;
        }
    }
    release_string(frame.stem, context.strings);
    frame.returned = true;
    frame.result = 1;
    return 1;
}
} // namespace

namespace {
void prune_disabled_groups(NativeMaterialPassBaseStorage& pass) {
    if (!enabled(pass, 0x1b)) remove_group(pass, {0x13, 0x14, 0xab});
    if (!enabled(pass, 0xce)) remove_group(pass, {0xcf, 0xd0, 0xd1});
    if (!enabled(pass, 0x0f)) remove_group(pass, {0x19, 0x18});
    if (!enabled(pass, 7)) remove_group(pass, {0x17, 0x0e});
    if (!enabled(pass, 0x34)) remove_group(pass, {0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x35});
    if (!enabled(pass, 0x1c)) remove_group(pass, {0x23, 0x8c});
}
void prune_after_capabilities(NativeMaterialPassBaseStorage& pass, std::uint8_t capability) {
    remove_native_material_render_state_00b5ee00(&pass, capability ? 0x9a : 0xb5);
    remove_group(pass, {0x80, 0x81, 0x82, 0x83, 0x89, 0x8e, 0x8d,
        0x92, 0x91, 0x93, 0x94, 0x8b, 0x8f});
}
} // namespace
void prune_native_material_pass_states_00b5f160(NativeMaterialPassBaseStorage& pass,
    void* const volatile& current_renderer) {
    prune_disabled_groups(pass);
    auto* renderer = current_renderer;
    require(renderer != nullptr, "B5F160 requires current callable renderer+104");
    const auto* table = *static_cast<const std::uintptr_t* const*>(renderer);
    using Caps = const std::uint8_t* (__thiscall*)(void*);
    const auto* caps = reinterpret_cast<Caps>(table[0x104 / 4])(renderer);
    require(caps != nullptr, "renderer+104 must return actual readable capabilities");
    prune_after_capabilities(pass, caps[0x3d]);
}
void prune_native_material_pass_states_00b5f160(NativeMaterialPassBaseStorage& pass,
    void* const volatile& current_renderer, const volatile std::uint32_t* profile) {
    prune_disabled_groups(pass);
    void* const renderer = current_renderer; // B5F325
    require(renderer != nullptr, "B5F160 requires the current actual renderer");
    const auto captured_profile = *static_cast<const volatile std::uint32_t*>(renderer);
    require(captured_profile == 0x00d5f0a8 && profile,
        "B5F160 requires the actual D5F0A8 profile domain");
    const auto target = profile[0x104 / 4]; // B5F32D; never replace a mutated slot.
    require(target == 0x00b1ff50, "B5F160 current renderer+104 target is unreconstructed");
    const void* const caps = get_native_compiler_renderer_capabilities_00b1ff50(renderer);
    const auto capability = static_cast<const volatile std::uint8_t*>(caps)[0x3d];
    prune_after_capabilities(pass, capability);
}
void finalize_native_material_pass_00b5f6a0(NativeMaterialPassBaseStorage& pass,
    NativeMaterialEffectProgramsContext& context, NativeMaterialProgramChild& child) {
    prune_native_material_pass_states_00b5f160(pass, context.current_renderer_00f8d394,
        context.actual_renderer_profile_00d5f0a8);
    auto* render = pass.render_18;
    auto* third = static_cast<NativeMaterialStateOwnerStorage*>(nullptr);
    auto* result = context.children.cache_render_00b26500(context.current_renderer_00f8d394, render, child);
    // Assembly captures the NEXT input before publishing the previous result.
    third = pass.third_1c;
    pass.render_18 = result;
    child.reset();
    result = context.children.cache_third_00b265c0(context.current_renderer_00f8d394, third, child);
    auto* sampler = pass.sampler_20;
    pass.third_1c = result;
    child.reset();
    pass.sampler_20 = context.children.cache_sampler_00b26680(context.current_renderer_00f8d394, sampler, child);
    child.reset();
}
void retain_native_material_effect_pass_00b17dd0(NativeMaterialEffectStorage& effect,
    NativeMaterialPassStorage& pass, NativeMaterialEffectProgramsContext& context,
    NativeMaterialProgramChild& child) {
    current_finalize(pass.base, context, child);
    pass.base.root.borrowed_effect_14 = &effect;
    const auto index = effect.base.retained_count_a8;
    require(index >= 0 && index < 3, "native effect retained9C requires valid append extent");
    effect.base.retained_9c[static_cast<std::size_t>(index)] = &pass;
    ++effect.base.retained_count_a8;
    pass.base.root.references_04.fetch_add(1);
}
NativeString* construct_native_material_program_number_00711370(NativeString& output,
    std::uint32_t value, NativeStringStorage& strings) {
    ::new (&output) NativeString;
    char text[11];
    const auto result = std::to_chars(text, text + 10, value);
    *result.ptr = '\0';
    NativeString temporary;
    construct_native_string_cstring_0041e870(&temporary, text, strings);
    const auto count = temporary.length();
    auto* const data = temporary.data();
    try {
        resize_native_string_header_0041dd40(&output, strings, count, true);
        if (count) std::memcpy(output.data(), data, output.length());
    } catch (...) {
        destroy_native_string_header_0041dd20(&temporary, strings);
        throw;
    }
    if (data) strings.release(data, count + 1u);
    return &output;
}
struct NativeMaterialEffectProgramOperation::Impl {
    std::array<LoadFrame,2> loads;
    NativeMaterialEffectProgramPhase phase{NativeMaterialEffectProgramPhase::fresh};
    std::uint32_t current{};
    std::uint32_t wrapper_site{};
};
NativeMaterialEffectProgramOperation::NativeMaterialEffectProgramOperation() : impl_(std::make_unique<Impl>()) {}
NativeMaterialEffectProgramOperation::~NativeMaterialEffectProgramOperation() {
    // Never turn a host frame destructor into native completed-creator cleanup.
    // The canonical effect terminal must first detach every live root binding.
    if (has_live_bindings()) std::terminate();
    for (const auto& frame : impl_->loads) if (frame.live_locals()) std::terminate();
}
bool NativeMaterialEffectProgramOperation::complete() const noexcept {
    return impl_->phase == NativeMaterialEffectProgramPhase::complete;
}
bool NativeMaterialEffectProgramOperation::pass_slots_initialized() const noexcept {
    return impl_->loads[impl_->current].slots_initialized;
}
bool NativeMaterialEffectProgramOperation::has_live_bindings() const noexcept {
    return impl_->loads[0].live_bindings() || impl_->loads[1].live_bindings();
}
NativeMaterialEffectProgramPhase NativeMaterialEffectProgramOperation::phase() const noexcept { return impl_->phase; }
std::uint32_t NativeMaterialEffectProgramOperation::active_call_site() const noexcept {
    return impl_->wrapper_site ? impl_->wrapper_site : impl_->loads[impl_->current].call_site;
}
std::uint32_t NativeMaterialEffectProgramOperation::primary_slots_written() const noexcept {
    return impl_->loads[impl_->current].primary_written;
}
NativeMaterialProgramChildFrame* NativeMaterialEffectProgramOperation::active_child() const noexcept {
    return impl_->loads[impl_->current].child.get();
}
std::uint8_t load_native_material_effect_variants_00b46950(NativeMaterialEffectStorage& effect,
    const void* name, NativeMaterialEffectProgramsContext& context, NativeMaterialEffectProgramOperation& operation) {
    auto& state = *operation.impl_;
    require(state.phase == NativeMaterialEffectProgramPhase::fresh, "native effect program admission is one-shot");
    require(name != nullptr, "native effect program load requires actual name8h");
    require(&context.strings == &context.lifetime.strings
        && &context.pass_construction.lifetime == &context.pass_copy.lifetime
        && &context.pass_copy.lifetime.strings == &context.strings
        && &context.pass_copy.lifetime.retained_owners == &context.lifetime.retained_owners,
        "native programs must share actual strings and canonical retained owners");
    const bool variants = context.load_variants_0108d6f0 != 0;
    state.phase = NativeMaterialEffectProgramPhase::first_load;
    try {
        auto result = load_native_material_effect_programs_00b45ee0(effect, name, 0, variants ? 3u : 0u, context, state.loads[0]);
        if (variants) {
            state.phase = NativeMaterialEffectProgramPhase::release_derived;
            state.wrapper_site = 0x00b4696d;
            require(state.loads[0].slots_initialized,
                "B46950 cleanup reached unwritten native pass slots after compiler failure");
            release_native_material_effect_owners_00b41b10(effect, context.lifetime.retained_owners);
            state.phase = NativeMaterialEffectProgramPhase::release_base;
            state.wrapper_site = 0x00b46974;
            release_native_material_effect_base_owners_00b187a0(effect.base, context.lifetime.retained_owners);
            state.phase = NativeMaterialEffectProgramPhase::second_load;
            state.current = 1;
            // B41B10 has now cleared every primary/secondary slot. A native
            // second-load false result leaves readable zero preimages.
            state.loads[1].slots_initialized = true;
            state.wrapper_site = 0;
            (void)load_native_material_effect_programs_00b45ee0(effect, name, 1, 3, context, state.loads[1]);
            result = 1;
        }
        state.phase = NativeMaterialEffectProgramPhase::complete;
        return result;
    } catch (...) {
        state.phase = NativeMaterialEffectProgramPhase::failed;
        throw;
    }
}
} // namespace bsp
