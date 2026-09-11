#include "bsp/gameplay_effect_definition.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Actual gameplay-effect storage requires Win32");
template<class T> T read(const void* base, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(base) + offset, sizeof value);
    return value;
}
template<class T> void write(void* base, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(base) + offset, &value, sizeof value);
}
std::int32_t add32(std::int32_t a, std::int32_t b) noexcept {
    const auto bits = static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b);
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}
void* slot(void* data, std::int32_t index) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(data)
        + static_cast<std::uint32_t>(index) * 4u);
}
void retain(void* component) noexcept {
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(
        static_cast<std::byte*>(component) + 4));
}
void release_slot(void* captured_slot, GameplayEffectComponentLifetime& lifetime) {
    void* const component = read<void*>(captured_slot, 0);
    if (component) {
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
                static_cast<std::byte*>(component) + 4)) == 0)
            lifetime.zero_references_slot_00(component);
        write<void*>(captured_slot, 0, nullptr); // Captured address, after callback.
    }
}
class DefinitionMemberUnwind final {
public:
    DefinitionMemberUnwind(GameplayEffectDefinition& owner,
        GameplayEffectDefinitionContext& context) noexcept : owner_(owner), context_(context) {}
    ~DefinitionMemberUnwind() noexcept {
        // Actual mapDC7F64 transitions before each funclet. A second exception
        // from a cleanup function terminates; no remaining cleanup is invented.
        if (state >= 2) {
            state = 1; // C95FD3 ->41DD20 on current owner+1C
            destroy_native_string_header_0041dd20(owner_.native.data() + 0x1c, context_.strings);
        }
        if (state >= 1) {
            state = 0; // C95FC8 ->86FC30 on current owner+08
            destroy_gameplay_effect_components_0086fc30(owner_.native.data() + 8, context_.components);
        }
        if (state >= 0) {
            state = -1; // C95FC0 ->BD30F0 on owner
            write<std::uint32_t>(owner_.native.data(), 0, 0x00ceb130);
        }
    }
    int state{2};
private:
    GameplayEffectDefinition& owner_;
    GameplayEffectDefinitionContext& context_;
};
} // namespace

GameplayEffectDefinition& construct_gameplay_effect_definition_00870256_fragment(
    GameplayEffectDefinition& owner) noexcept {
    auto* data = owner.native.data();
    write<std::uint32_t>(data, 0, 0x00ceb130);
    // Start the actual count's C++ lifetime at the native's one refs=1 store.
    // All existing raw Interlocked paths address this SAME four-byte word.
    ::new (static_cast<void*>(data + 4)) std::atomic<std::int32_t>(1);
    write<std::uint32_t>(data, 0, 0x00d0da58);
    ::new (static_cast<void*>(data + 8)) void**(nullptr);
    ::new (static_cast<void*>(data + 0xc)) std::int32_t(0);
    ::new (static_cast<void*>(data + 0x10)) std::int32_t(0);
    for (const auto offset : {0x1cu, 0x20u})
        write<std::uint32_t>(data, offset, 0);
    return owner;
}
GameplayEffectDefinition* allocate_gameplay_effect_definition_00870240_fragment() {
    void* const storage = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x24, sizeof(GameplayEffectDefinition)});
    if (!storage) return nullptr;
    std::array<std::byte, 0x24> preimage;
    std::memcpy(preimage.data(), storage, preimage.size());
    auto* const owner = ::new (storage) GameplayEffectDefinition;
    std::memcpy(owner->native.data(), preimage.data(), preimage.size());
    construct_gameplay_effect_definition_00870256_fragment(*owner);
    return owner;
}
void set_gameplay_effect_definition_identity_0086b870(GameplayEffectDefinition& owner,
    std::int32_t id, const void* source, NativeStringStorage& strings) {
    auto* const name = owner.native.data() + 0x1c;
    write<std::int32_t>(owner.native.data(), 0x18, id);
    if (name == source) return;
    resize_native_string_header_0041dd40(name, strings, read<std::uint32_t>(source, 0), true);
    if (read<std::uint32_t>(source, 0) != 0) {
        const auto length = read<std::uint32_t>(name, 0);
        if (length) std::memcpy(read<void*>(name, 4), read<void*>(source, 4), length);
    }
}
void reserve_gameplay_effect_components_0086e770(void* header,
    std::int32_t requested, GameplayEffectComponentLifetime& lifetime) {
    if (requested < 1) requested = 1;
    if (read<std::int32_t>(header, 8) >= requested) return;
    const auto bytes = static_cast<std::uint32_t>(requested) * 4u;
    void* const allocated = singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes}); //00BF55BE ->00BF681B
    for (std::int32_t i = 0; i < read<std::int32_t>(header, 4); i = add32(i, 1)) {
        void* const destination = slot(allocated, i);
        if (destination) {
            void* const source = slot(read<void*>(header, 0), i);
            write<void*>(destination, 0, nullptr);
            if (void* const component = read<void*>(source, 0)) {
                write<void*>(destination, 0, component);
                retain(component);
            }
        }
    }
    for (std::int32_t i = 0; i < read<std::int32_t>(header, 4); i = add32(i, 1))
        release_slot(slot(read<void*>(header, 0), i), lifetime);
    singleton_lifetime_free(read<void*>(header, 0)); // Current data after callbacks.
    write<void*>(header, 0, allocated);
    write<std::int32_t>(header, 8, requested);
}
void append_gameplay_effect_component_0086eb60(void* header,
    const void* source, GameplayEffectComponentLifetime& lifetime) {
    const auto capacity = read<std::int32_t>(header, 8);
    if (read<std::int32_t>(header, 4) == capacity) {
        auto grown = add32(capacity, capacity);
        if (grown <= 1) grown = 1;
        reserve_gameplay_effect_components_0086e770(header, grown, lifetime);
    }
    void* const destination = slot(read<void*>(header, 0), read<std::int32_t>(header, 4));
    if (destination) {
        write<void*>(destination, 0, nullptr);
        if (void* const component = read<void*>(source, 0)) {
            write<void*>(destination, 0, component);
            retain(component);
        }
    }
    write<std::int32_t>(header, 4, add32(read<std::int32_t>(header, 4), 1));
}
void resize_gameplay_effect_components_0086edd0(void* header,
    std::int32_t requested, GameplayEffectComponentLifetime& lifetime) {
    if (requested > read<std::int32_t>(header, 8))
        reserve_gameplay_effect_components_0086e770(header, requested, lifetime);
    for (auto i = read<std::int32_t>(header, 4); i < requested; i = add32(i, 1)) {
        if (void* const destination = slot(read<void*>(header, 0), i))
            write<void*>(destination, 0, nullptr);
    }
    while (requested < read<std::int32_t>(header, 4)) {
        write<std::int32_t>(header, 4, add32(read<std::int32_t>(header, 4), -1));
        const auto index = read<std::int32_t>(header, 4);
        void* const data = read<void*>(header, 0);
        release_slot(slot(data, index), lifetime);
    }
    write<std::int32_t>(header, 4, requested);
}
void destroy_gameplay_effect_components_0086fc30(void* header,
    GameplayEffectComponentLifetime& lifetime) {
    resize_gameplay_effect_components_0086edd0(header, 0, lifetime);
    singleton_lifetime_free(read<void*>(header, 0)); // Current buffer after callbacks.
}
void destroy_gameplay_effect_definition_00870d00(GameplayEffectDefinition& owner,
    GameplayEffectDefinitionContext& context) {
    auto* const data = owner.native.data();
    write<std::uint32_t>(data, 0, 0x00d0da58);
    DefinitionMemberUnwind unwind(owner, context); // State2 BEFORE manager/map calls.
    auto* const manager = get_gameplay_effect_manager_004c1650(context.manager);
    const auto id = read<std::int32_t>(data, 0x18); // After current getter.
    auto& definitions = *manager->definitions;
    const auto found = definitions.find(id); //0086B650, signed ID comparison.
    if (found == definitions.end()) throw std::out_of_range("invalid map/set<T> iterator");
    definitions.erase(found); //0086E8A0: erase by ID without inspecting its value.
    unwind.state = 1; //00870D69: the name is no longer an armed cleanup.
    destroy_native_string_header_0041dd20(data + 0x1c, context.strings);
    unwind.state = 0; //00870D8D: array-stage throws must NOT retry the array.
    destroy_gameplay_effect_components_0086fc30(data + 8, context.components);
    unwind.state = -1; //00870DA4, before normal base destruction.
    write<std::uint32_t>(data, 0, 0x00ceb130); // Existing00BD30F0 base primitive.
}
GameplayEffectDefinition* scalar_delete_gameplay_effect_definition_00871440(
    GameplayEffectDefinition* owner, std::uint32_t flags,
    GameplayEffectDefinitionContext& context) {
    auto* const original = owner;
    destroy_gameplay_effect_definition_00870d00(*owner, context);
    if ((flags & 1u) != 0) {
        owner->~GameplayEffectDefinition();
        singleton_lifetime_free(owner);
    }
    return original;
}
} // namespace bsp
