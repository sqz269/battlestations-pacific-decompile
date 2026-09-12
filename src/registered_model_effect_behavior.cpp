#include "bsp/registered_model_effect_behavior.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Registered model effect behavior requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(NativeNodeStorage) == 0x174);
static_assert(sizeof(RegisteredModelEffectStorage) == 0x20);
static_assert(offsetof(RegisteredModelEffectStorage, model_1c) == 0x1c);

template<class T> T field(const void* owner, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(owner) + offset, sizeof(value));
    return value;
}
template<class T> void store(void* owner, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(owner) + offset, &value, sizeof(value));
}
void* address(std::uintptr_t value) noexcept { return reinterpret_cast<void*>(value); }
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
// The native update keeps delta on x87 across the reference mode read.
__declspec(naked) NativeNodeStorage* __fastcall capture_update(float*, const float*,
    const void*, const RegisteredModelEffectStorage*, std::uint8_t*) {
    __asm {
        mov eax,dword ptr [esp+4]
        fld dword ptr [edx]
        cmp dword ptr [eax+198h],3
        sete dl
        mov eax,dword ptr [esp+12]
        mov byte ptr [eax],dl
        mov eax,dword ptr [esp+8]
        mov eax,dword ptr [eax+1ch]
        fstp dword ptr [ecx]
        ret 12
    }
}
__declspec(naked) void __fastcall copy_float_x87(void*, const void*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        ret
    }
}
}

std::uint8_t complete_particle_model_00af6be0(NativeNodeStorage& model,
    RegisteredModelEffectCallees& construction, RegisteredModelEffectBehaviorCallees& callees) {
    const auto* gate = static_cast<const volatile std::byte*>(construction.call_0051f6b0());
    if (gate[4] != std::byte{0}) return 1;
    if (!field<std::uint8_t>(&model, 0x1a5)) return 0;
    const auto count = field<std::uint32_t>(&model, 0x198);
    auto cursor = field<std::uintptr_t>(&model, 0x194);
    const auto end = cursor + count * 4u;
    while (cursor != end) {
        auto* const emitter = field<void*>(address(cursor), 0);
        if (field<void*>(emitter, 0x10)) {
            auto* const container = callees.call_00aff690(emitter);
            if (field<std::uint32_t>(container, 0x1c)) return 0;
        }
        cursor += 4u;
    }
    return field<std::uint32_t>(field<void*>(&model, 0x190), 0x14) == 0 ? 1 : 0;
}
std::uint8_t complete_registered_model_effect_00872010(RegisteredModelEffectStorage& effect,
    RegisteredModelEffectCallees& construction, RegisteredModelEffectBehaviorCallees& callees) {
    return complete_particle_model_00af6be0(*effect.model_1c, construction, callees);
}

void update_registered_model_effect_00872740(RegisteredModelEffectStorage& effect,
    float delta, const void* reference, RegisteredModelEffectBehaviorCallees& callees) {
    float argument;
    std::uint8_t mode;
    auto* const model = capture_update(&argument, &delta, reference, &effect, &mode);
    callees.call_00af6dd0(*model, argument, mode);
}

void stop_particle_emitter_states_00b05070(void* container,
    RegisteredModelEffectBehaviorCallees& callees) {
    std::uint32_t index = 0;
    while (signed_word(index) < field<std::int32_t>(container, 0x1c)) {
        auto* const rows = field<std::byte*>(container, 0x0c);
        auto* const states = field<std::byte*>(container, 0x14);
        const auto row_offset = index * 8u;
        const auto id = field<std::uint16_t>(rows, row_offset);
        auto* const state = states + static_cast<std::uint32_t>(id) * 0x6cu;
        auto* const definition = field<void*>(state, 0x64);
        if (field<std::uint8_t>(definition, 0x28)) {
            const auto current_id = field<std::uint16_t>(rows, row_offset);
            (void)callees.call_00b04f00(states + static_cast<std::uint32_t>(current_id) * 0x6cu, 0);
            const auto last = field<std::uint32_t>(container, 0x1c) - 1u;
            if (signed_word(index) < signed_word(last)) {
                auto* const current_rows = field<std::byte*>(container, 0x0c);
                const auto last_offset = last * 8u;
                const auto last_value = field<std::uint32_t>(current_rows, last_offset + 4u);
                const auto last_id_word = field<std::uint32_t>(current_rows, last_offset);
                store(current_rows, last_offset, field<std::uint16_t>(current_rows, row_offset));
                copy_float_x87(current_rows + last_offset + 4u, current_rows + row_offset + 4u);
                auto* const reloaded_rows = field<std::byte*>(container, 0x0c);
                store(reloaded_rows, row_offset, static_cast<std::uint16_t>(last_id_word));
                store(reloaded_rows, row_offset + 4u, last_value);
                --index;
            }
            store(container, 0x1c, field<std::uint32_t>(container, 0x1c) - 1u);
        }
        ++index;
    }
}
void stop_particle_emitter_00aff570(void* emitter,
    RegisteredModelEffectBehaviorCallees& callees) {
    if (auto* const container = field<void*>(emitter, 0x10))
        stop_particle_emitter_states_00b05070(container, callees);
}
void stop_particle_model_00af5f20(NativeNodeStorage& model,
    RegisteredModelEffectBehaviorCallees& callees) {
    auto cursor = field<std::uintptr_t>(&model, 0x194);
    store(&model, 0x1a4, std::uint8_t{0});
    const auto end = cursor + field<std::uint32_t>(&model, 0x198) * 4u;
    while (cursor != end) {
        stop_particle_emitter_00aff570(field<void*>(address(cursor), 0), callees);
        cursor += 4u;
    }
}
void deactivate_registered_model_effect_00871fe0(RegisteredModelEffectStorage& effect,
    RegisteredModelEffectBehaviorCallees& callees) {
    stop_particle_model_00af5f20(*effect.model_1c, callees);
}
} // namespace bsp
