#include "bsp/native_particle_emitter_lifetime.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle emitter lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* base, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(base) + offset, sizeof value);
    return value;
}
template<class T> void write(void* base, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(base) + offset, &value, sizeof value);
}
std::uint32_t saturated_product(std::uint32_t count, std::uint32_t stride) noexcept {
    const auto product = static_cast<std::uint64_t>(count) * stride;
    return product > UINT32_MAX ? UINT32_MAX : static_cast<std::uint32_t>(product);
}
void* allocate(std::uint32_t size) {
    // Both BF681B and BF55BE have the existing malloc/new-handler/throw contract.
    return singleton_lifetime_allocate({SingletonAllocationKind::object, size, size});
}
void release_rows(NativeParticleEmitterRow* rows) noexcept {
    if (!rows) return;
    auto* allocation = reinterpret_cast<unsigned char*>(rows) - 4;
    const auto count = read<std::uint32_t>(allocation, 0);
    // BF7C6E decrements signed count BEFORE the sign test, then visits backwards.
    for (auto i = count - 1U; static_cast<std::int32_t>(i) >= 0; --i)
        destroy_native_particle_emitter_row_00b04910(rows[i]);
    singleton_lifetime_free(allocation);
}
struct Guard {
    std::uint32_t profile;
    TrackedCriticalSection* section;
};
static_assert(sizeof(Guard) == 8);
void decrement_and_leave(TrackedCriticalSection* section) noexcept {
    if (!section) return;
    auto depth = read<std::uint32_t>(section, 0x18);
    write(section, 0x18, depth - 1U);
    LeaveCriticalSection(&section->native);
}
} // namespace

NativeParticleEmitterRow* initialize_native_particle_emitter_row_00b04900(void* raw) noexcept {
    auto* row = ::new (raw) NativeParticleEmitterRow;
    row->state_id_00 = 0;
    row->value_04 = 0.0f;
    return row;
}
void destroy_native_particle_emitter_row_00b04910(NativeParticleEmitterRow&) noexcept {}

void replace_native_particle_emitter_rows_00b04b50(
    NativeParticleEmitterRows& rows, std::uint32_t count) {
    release_rows(rows.data_00);
    const auto product = saturated_product(count, 8);
    const auto bytes = product > UINT32_MAX - 4U ? UINT32_MAX : product + 4U;
    auto* raw = static_cast<unsigned char*>(allocate(bytes));
    if (raw) {
        write(raw, 0, count);
        auto* data = reinterpret_cast<NativeParticleEmitterRow*>(raw + 4);
        // The concrete B04900 cannot throw. Its CRT iterator's unwind is empty.
        for (std::int32_t i = 0; i < static_cast<std::int32_t>(count); ++i)
            initialize_native_particle_emitter_row_00b04900(data + i);
        rows.data_00 = data;
    } else {
        rows.data_00 = nullptr;
    }
    rows.capacity_04 = count;
}
void destroy_native_particle_emitter_rows_00b04c40(NativeParticleEmitterRows& rows) noexcept {
    release_rows(rows.data_00);
    rows.capacity_04 = 0;
    rows.data_00 = nullptr;
}
void destroy_native_particle_emitter_states_00b04a80(NativeParticleEmitterStates& states) noexcept {
    if (states.data_00) singleton_lifetime_free(states.data_00);
    states.capacity_04 = 0;
    states.data_00 = nullptr;
}
NativeParticleEmitterContainer* construct_native_particle_emitter_base_00b04e30(
    void* raw, void* model, void* emitter, std::uint32_t capacity) {
    auto* owner = ::new (raw) NativeParticleEmitterContainer;
    owner->native_vtable_00 = 0x00d5df20;
    owner->rows_0c.data_00 = nullptr;
    owner->rows_0c.capacity_04 = 0;
    owner->states_14.data_00 = nullptr;
    owner->states_14.capacity_04 = 0;
    owner->model_04 = model;
    owner->emitter_08 = emitter;
    owner->count_1c = 0;
    owner->word_20 = 0;
    owner->word_24 = 0;
    owner->word_28 = 0;
    try {
        if (static_cast<std::int32_t>(capacity) > 0) {
            replace_native_particle_emitter_rows_00b04b50(owner->rows_0c, capacity);
            if (owner->states_14.data_00) singleton_lifetime_free(owner->states_14.data_00);
            owner->states_14.data_00 = allocate(saturated_product(capacity, 0x6c));
            owner->states_14.capacity_04 = capacity;
            for (std::uint32_t i = 0; i < capacity; ++i)
                owner->rows_0c.data_00[i].state_id_00 = static_cast<std::uint16_t>(i);
        }
    } catch (...) {
        destroy_native_particle_emitter_states_00b04a80(owner->states_14);
        destroy_native_particle_emitter_rows_00b04c40(owner->rows_0c);
        throw;
    }
    return owner;
}
NativeParticleEmitterContainer* construct_native_particle_emitter_container_00b053d0(
    void* raw, void* model, void* emitter, std::uint32_t capacity) {
    auto* owner = construct_native_particle_emitter_base_00b04e30(raw, model, emitter, capacity);
    owner->native_vtable_00 = 0x00d5df24;
    owner->word_2c = 0;
    return owner;
}
void* acquire_native_particle_emitter_container_00aff690(void* emitter) {
    if (!read<void*>(emitter, 0x10)) {
        void* raw = allocate(0x30);
        void* constructed = nullptr;
        try {
            if (raw) {
                auto* definition = read<void*>(emitter, 0x0c);
                const auto capacity = read<std::uint32_t>(definition, 0x20);
                auto* model = read<void*>(emitter, 8);
                constructed = construct_native_particle_emitter_container_00b053d0(
                    raw, model, emitter, capacity);
            }
        } catch (...) {
            singleton_lifetime_free(raw);
            throw;
        }
        write(emitter, 0x10, constructed);
    }
    return read<void*>(emitter, 0x10);
}
std::uint8_t cleanup_native_particle_emitter_state_00b04f00(
    void* state, std::uint32_t argument, NativeParticleEmitterCleanupBindings& bindings) {
    auto* definition = read<void*>(state, 0x64);
    const auto table = read<std::uint32_t>(definition, 0);
    const auto target = read<std::uint32_t>(reinterpret_cast<void*>(table), 0x1c);
    const auto call = bindings.capture_definition_virtual1c(definition, table, target);
    const auto result = call.invoke(call.context, definition, state, argument);
    if (!read<void*>(state, 0x60)) return result;
    auto* lock_owner = bindings.call_0072b740();
    auto* section = read<TrackedCriticalSection*>(lock_owner, 4);
    Guard guard{0x00ce37fc, section};
    if (section) {
        EnterCriticalSection(&section->native);
        auto depth = read<std::uint32_t>(section, 0x18);
        write(section, 0x18, depth + 1U);
    }
    try {
        bindings.call_00b7c160(read<void*>(state, 0x60));
        bindings.call_00b6dfa0(read<void*>(state, 0x60));
        write<void*>(state, 0x60, nullptr);
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
    decrement_and_leave(section);
    return result;
}
} // namespace bsp
