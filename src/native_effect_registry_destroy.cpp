#include "bsp/native_effect_registry_destroy.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_render_resource_container_removal.hpp"
#include "bsp/native_renderer_cache_cleanup.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);
template<class T> volatile T& field(void* base, Word offset) noexcept {
    return *reinterpret_cast<volatile T*>(reinterpret_cast<Word>(base) + offset);
}
void* plus(void* base, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(base) + offset);
}
void* current_last_effect(void* registry) noexcept {
    const auto count = field<Word>(registry, 8);
    auto* const data = field<void*>(registry, 4);
    return field<void*>(plus(data, count * 0x2cu - 4u), 0);
}
Word current_effect_size(void* effect, NativeMaterialEffectCacheContext& c) {
    const auto profile = field<Word>(effect, 0);
    const volatile Word* table = nullptr;
    if (profile == 0x00d5e534u) table = c.effect_base_vtable_00d5e534;
    if (profile == 0x00d61a00u) table = c.effect_derived_vtable_00d61a00;
    if (table == nullptr || table[3] != 0x00a82250u)
        throw std::invalid_argument("unsupported current actual effect accounting profile");
    return native_resource_zero_accounted_size_00a82250();
}
void current_registry_release(void* registry, void* captured_effect,
    NativeEffectRegistryDestructionContext& c) {
    const auto profile = field<Word>(registry, 0);
    const volatile Word* table = nullptr;
    if (profile == 0x00d5f04cu) table = c.actual_base_profile_00d5f04c;
    if (profile == 0x00d5f074u) table = c.actual_cache.registry_vtable_00d5f074;
    if (table == nullptr || table[4] != 0x00b32010u)
        throw std::invalid_argument("unsupported current actual effect registry release slot");
    release_native_cached_effect_00b32010(captured_effect, c.actual_cache.effects.owners.actual_owners());
}
struct ArrayUnwind {
    void* const registry;
    NativeMaterialEffectCacheContext& context;
    bool armed = true;
    ~ArrayUnwind() noexcept {
        if (armed) destroy_native_effect_record_array_00b317c0(plus(registry, 4), context);
    }
};
} // namespace

void release_native_cached_effect_00b32010(void* effect, NativeRenderActualOwners& owners) {
    // Existing actual+04 atomic helper resolves ONLY after decrement reaches
    // zero; the canonical terminal validates its current concrete profile.
    release_native_render_actual_owner(owners, effect);
}

void clear_native_effect_registry_00b31750(void* registry, NativeEffectRegistryDestructionContext& c) {
    auto& cache = c.actual_cache;
    while (field<Word>(registry, 8) != 0) {
        const auto size = current_effect_size(current_last_effect(registry), cache);
        field<Word>(registry, 0x10) = field<Word>(registry, 0x10) - size;
        auto* const effect = current_last_effect(registry);
        current_registry_release(registry, effect, c);
        const auto count = field<Word>(registry, 8);
        if (count != 0) {
            auto* const data = field<void*>(registry, 4);
            auto* const record = static_cast<NativeRenderResourceRecord*>(plus(data, count * 0x2cu - 0x2cu));
            destroy_native_effect_record_00b2fa10(*record, cache.strings);
            field<Word>(registry, 8) = field<Word>(registry, 8) - 1u;
        }
    }
    resize_native_effect_records_00b30410(plus(registry, 4), 0, cache);
}

void destroy_native_effect_record_array_00b317c0(void* header, NativeMaterialEffectCacheContext& c) {
    resize_native_effect_records_00b30410(header, 0, c);
    // Reload only after resize/destruction. The native body leaves this freed
    // pointer and its capacity intact; its count is whatever resize produced.
    c.free_array_00bf6989(field<void*>(header, 0));
}

void destroy_native_effect_registry_00b320f0(void* registry, NativeEffectRegistryDestructionContext& c) {
    field<Word>(registry, 0) = 0x00d5f04cu;
    ArrayUnwind cleanup{registry, c.actual_cache};
    clear_native_effect_registry_00b31750(registry, c);
    cleanup.armed = false;
    destroy_native_effect_record_array_00b317c0(plus(registry, 4), c.actual_cache);
}

void destroy_native_effect_registry_thunk_00b32200(void* registry, NativeEffectRegistryDestructionContext& c) {
    destroy_native_effect_registry_00b320f0(registry, c);
}
} // namespace bsp
