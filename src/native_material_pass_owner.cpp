#include "bsp/native_material_pass_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
struct BaseCleanup {
    NativeMaterialPassBaseStorage& storage;
    NativeRenderActualOwners& owners;
    bool armed = true;
    ~BaseCleanup() noexcept(false) {
        if (armed) destroy_native_material_pass_base_00b5f510(storage, owners);
    }
};
struct NameCleanup {
    NativeString& header;
    NativeStringStorage& strings;
    ~NameCleanup() { destroy_native_string_header_0041dd20(&header, strings); }
};
void release_clear(void*& slot, NativeRenderActualOwners& owners) {
    void* const captured = slot;
    if (!captured) return;
    release_native_render_actual_owner(owners, captured);
    slot = nullptr;
}
} // namespace

NativeMaterialPassStorage* initialize_native_material_pass_00b44b10(
    void* raw, NativeMaterialPassConstructionAccess& access) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % alignof(NativeMaterialPassStorage)
        || !access.state_registration.bind)
        throw std::invalid_argument("native pass needs aligned storage and canonical state registration");
    auto* pass = ::new(raw) NativeMaterialPassStorage;
    initialize_native_material_pass_base_00b5f720(&pass->base);
    BaseCleanup base{pass->base, access.lifetime.retained_owners};
    pass->base.root.vtable_00 = 0x00d61be8;
    pass->binding_count_6c = 0;
    pass->vertex_shader_70 = nullptr;
    pass->pixel_shader_74 = nullptr;
    pass->index_78 = -1;
    pass->index_7c = -1;
    pass->byte_80 = 0;
    pass->fallback_84 = nullptr;
    // Native has no host companions. Register metadata for its three actual
    // state owners now so base unwind can dispatch their real terminal paths.
    access.state_registration.bind(access.state_registration.context, pass->base);
    NativeString temporary;
    resize_native_string_header_0041dd40(&temporary, access.lifetime.strings, 9, true);
    if (temporary.data()) std::memcpy(temporary.data(), "white.tga", temporary.length() + 1u);
    {
        // Native state1 is armed only after resize/copy, before renderer+64.
        const NameCleanup cleanup{temporary, access.lifetime.strings};
        void* const renderer = access.current_renderer_00f8d394;
        const auto* const table = *static_cast<const std::uintptr_t* const*>(renderer);
        using Acquire = void* (__thiscall*)(void*, NativeString*, std::uint32_t);
        pass->fallback_84 = reinterpret_cast<Acquire>(table[0x64 / 4])(renderer, &temporary, 0);
    }
    base.armed = false;
    return pass;
}
void destroy_native_material_pass_binding_00b42250(NativeMaterialPassBindingStorage& binding,
    NativeMaterialPassDestructionAccess& access) {
    const NameCleanup cleanup{binding.name_08, access.strings};
    release_clear(binding.retained_04, access.retained_owners);
}
void destroy_native_material_pass_00b454e0(NativeMaterialPassStorage& pass,
    NativeMaterialPassDestructionAccess& access) {
    pass.base.root.vtable_00 = 0x00d61be8;
    const BaseCleanup base{pass.base, access.retained_owners};
    for (std::uint32_t i = 0; i < pass.binding_count_6c; ++i) {
        if (i >= pass.bindings_5c.size())
            throw std::logic_error("native pass binding count exceeds its actual four slots");
        auto* const binding = pass.bindings_5c[i];
        if (!binding) continue;
        destroy_native_material_pass_binding_00b42250(*binding, access);
        singleton_lifetime_free(binding);
        pass.bindings_5c[i] = nullptr;
    }
    release_clear(pass.fallback_84, access.retained_owners);
    release_clear(pass.vertex_shader_70, access.retained_owners);
    release_clear(pass.pixel_shader_74, access.retained_owners);
}
NativeMaterialPassStorage* delete_native_material_pass_00b46910(NativeMaterialPassStorage* pass,
    NativeMaterialPassDestructionAccess& access, std::uint32_t flags) {
    destroy_native_material_pass_00b454e0(*pass, access);
    if (flags & 1) access.pool.return_slot_00b40a40(pass);
    return pass;
}
void* set_native_material_pass_effect_00b172b0(NativeMaterialPassRootStorage& root, void* effect) noexcept {
    root.borrowed_effect_14 = effect;
    return effect;
}
void* get_native_material_pass_effect_00b172c0(const NativeMaterialPassRootStorage& root) noexcept {
    return root.borrowed_effect_14;
}

NativeMaterialPassReference::NativeMaterialPassReference(NativeMaterialPassStorage& pass,
    NativeMaterialPassDestructionAccess& access, const volatile std::uint32_t* profile,
    NativeMaterialPassCompanionDisposal disposal)
    : RenderCommandReference(pass.base.root.references_04), storage_(pass), access_(access),
      profile_(profile), disposal_(disposal) {
    if (!disposal.retire || pass.base.root.references_04.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("native pass reference requires live storage and explicit retirement");
    require_current_profile();
}
NativeMaterialPassReference::~NativeMaterialPassReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeMaterialPassReference::require_current_profile() const noexcept {
    if (storage_.base.root.vtable_00 != 0x00d61be8 || !profile_
        || profile_[0] != 0x00bd30e0 || profile_[1] != 0x00b46910) std::terminate();
}
void NativeMaterialPassReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    try { delete_native_material_pass_00b46910(&storage_, access_, 1); }
    catch (...) { std::terminate(); }
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}
} // namespace bsp
