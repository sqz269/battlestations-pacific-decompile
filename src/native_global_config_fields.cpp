#include "bsp/native_global_config_fields.hpp"
#include "bsp/native_input_settings_vector_storage.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstring>

namespace bsp {
namespace {
template<class T> T read(const void* p) noexcept {
    T value; std::memcpy(&value, p, sizeof(value)); return value;
}
void release(void* value, GameplayEffectComponentLifetime& lifetime) {
    if (value && InterlockedDecrement(reinterpret_cast<volatile LONG*>(
        static_cast<unsigned char*>(value) + 4)) == 0)
        lifetime.zero_references_slot_00(value);
}
} // namespace

bool assign_global_config_sound_008dbe90(void* effects, std::int32_t index,
    const NativeString& name, NativeGlobalConfigSoundContext& context) {
    void* temporary;
    auto** const result = acquire_sound_sample_00a83fd0(
        *context.current_cache_00f8bbe8, temporary, name, context.cache);
    void* const next = *result;
    auto* const slot = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(effects)
        + static_cast<std::uint32_t>(index) * 4u);
    void* const old = read<void*>(slot);
    try {
        if (old != next) {
            std::memcpy(slot, &next, sizeof(next));
            if (next) InterlockedIncrement(reinterpret_cast<volatile LONG*>(
                static_cast<unsigned char*>(next) + 4));
            release(old, context.lifetime);
        }
    } catch (...) {
        // Native state0 -> CA33D0 ->4C3810 cleans the actual local pointer.
        release(temporary, context.lifetime);
        temporary = nullptr;
        throw;
    }
    // Native disarms state0 before this final release; do not retry it on throw.
    release(temporary, context.lifetime);
    return true;
}

void append_global_config_multiplier_storage(void* vector, std::uint32_t value_bits) {
    append_native_checked_float_storage(vector, value_bits);
}
} // namespace bsp
