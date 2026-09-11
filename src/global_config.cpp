#include "bsp/global_config.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <new>

namespace bsp {
namespace {
template<class T> T read(const void* base, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(base) + offset, sizeof value);
    return value;
}
template<class T> void write(void* base, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(base) + offset, &value, sizeof value);
}
void release(void* object, GlobalConfigEffects& effects) noexcept {
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
            static_cast<std::byte*>(object) + 4)) == 0)
        effects.zero_references_slot_00(object);
}
void destroy_slot(void* slot, GlobalConfigEffects& effects) noexcept {
    void* const captured = read<void*>(slot, 0);
    if (captured) {
        release(captured, effects);
        write<void*>(slot, 0, nullptr);
    }
}
struct CapturedSection {
    SystemSingletonCriticalSection* section;
    explicit CapturedSection(SystemSingletonCriticalSection* captured) : section(captured) {
        if (section) {
            singleton_enter_critical_section(*section);
            ++section->recursion_18;
        }
    }
    ~CapturedSection() {
        if (section) {
            --section->recursion_18;
            singleton_leave_critical_section(*section);
        }
    }
};
} // namespace

void destroy_global_config_pointer_slot_004c3810(void* slot,
    GlobalConfigEffects& effects) noexcept { destroy_slot(slot, effects); }
void destroy_global_config_pointer_slot_00524180(void* slot,
    GlobalConfigEffects& effects) noexcept { destroy_slot(slot, effects); }

void construct_global_config_effects_008dbcd0(void* storage,
    GlobalConfigEffects& effects) noexcept {
    for (std::size_t i = 0; i != 3; ++i) write<void*>(storage, i * 4, nullptr);
    for (std::size_t i = 0; i != 3; ++i) write<void*>(storage, 0xc + i * 4, nullptr);
    write<std::uint32_t>(storage, 0x18, 0);
    for (std::size_t i = 0; i != 3; ++i) {
        auto* first = static_cast<std::byte*>(storage) + i * 4;
        destroy_slot(first, effects);
        write<void*>(first, 0, nullptr); // native unconditional second store
        auto* second = first + 0xc;
        destroy_slot(second, effects);
        write<void*>(second, 0, nullptr);
    }
}
void destroy_global_config_effects_008dbdb0(void* storage,
    GlobalConfigEffects& effects) noexcept {
    for (std::size_t i = 0; i != 3; ++i) {
        auto* second = static_cast<std::byte*>(storage) + 0xc + i * 4;
        void* const captured = read<void*>(second, 0);
        if (captured) {
            effects.stop_slot_08(captured, 0);
            destroy_slot(second, effects); // CURRENT slot after virtual+8
            write<void*>(second, 0, nullptr);
        }
        auto* first = second - 0xc;
        destroy_slot(first, effects);
        write<void*>(first, 0, nullptr);
    }
    // Earlier callbacks can repopulate slots already visited. The native EH
    //iterator destructors therefore remain observable even after the loop.
    for (std::size_t i = 3; i != 0; --i)
        destroy_global_config_pointer_slot_00524180(
            static_cast<std::byte*>(storage) + 0xc + (i - 1) * 4, effects);
    for (std::size_t i = 3; i != 0; --i)
        destroy_global_config_pointer_slot_004c3810(
            static_cast<std::byte*>(storage) + (i - 1) * 4, effects);
}
void destroy_global_config_strings_004312b0(void* headers,
    NativeStringStorage& strings) noexcept {
    for (std::size_t i = 7; i != 0; --i)
        destroy_native_string_header_0041dd20(
            static_cast<std::byte*>(headers) + (i - 1) * 8, strings);
}
void destroy_global_config_vectors_00431210(void* headers) noexcept {
    for (std::size_t i = 4; i != 0; --i) {
        const auto offset = (i - 1) * 0x10;
        void* const begin = read<void*>(headers, offset + 4);
        if (begin) singleton_lifetime_free(begin);
        write<void*>(headers, offset + 4, nullptr);
        write<void*>(headers, offset + 8, nullptr);
        write<void*>(headers, offset + 0xc, nullptr);
    }
}
void destroy_global_config_name_range_00432050(void* begin, void* end,
    NativeStringStorage& strings) noexcept {
    for (auto cursor = reinterpret_cast<std::uintptr_t>(begin);
            cursor != reinterpret_cast<std::uintptr_t>(end); cursor += 8)
        destroy_native_string_header_0041dd20(reinterpret_cast<void*>(cursor), strings);
}
GlobalConfigOwner& construct_global_config_004324e0(GlobalConfigOwner& owner,
    GlobalConfigEffects& effects) noexcept {
    auto* data = owner.native.data();
    write<std::uint32_t>(data, 0, 0x00ce3d98);
    for (std::size_t vector = 0; vector != 5; ++vector)
        for (std::size_t word = 0; word != 3; ++word)
            write<std::uint32_t>(data, 0x10 + vector * 0x10 + word * 4, 0);
    for (std::size_t word = 0; word != 14; ++word)
        write<std::uint32_t>(data, 0xa8 + word * 4, 0);
    construct_global_config_effects_008dbcd0(data + 0x2c0, effects);
    return owner;
}
GlobalConfigOwner* get_global_config_00432650(GlobalConfigContext& context) {
    if (auto* const existing = context.singleton_00f878e4) return existing;
    {
        auto& first_manager = context.lifetime.get_manager_00415350()->system_owner();
        CapturedSection lock(first_manager.section_10);
        if (!context.singleton_00f878e4) {
            void* const storage = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 0x2e8, sizeof(GlobalConfigOwner)});
            GlobalConfigOwner* owner = nullptr;
            if (storage) {
                // Preserve representation bytes before beginning the C++ shell's
                //lifetime. No float load or blanket zero initialization.
                std::array<std::byte, 0x2e8> preimage;
                std::memcpy(preimage.data(), storage, preimage.size());
                owner = ::new (storage) GlobalConfigOwner;
                std::memcpy(owner->native.data(), preimage.data(), preimage.size());
                construct_global_config_004324e0(*owner, context.effects);
            }
            context.singleton_00f878e4 = owner;
            auto* const second_manager = context.lifetime.get_manager_00415350();
            second_manager->register_object(context.singleton_00f878e4);
        }
    } // decrement captured recursion counter and leave before FINAL slot reload
    return context.singleton_00f878e4;
}
void destroy_global_config_004325b0(GlobalConfigOwner& owner,
    GlobalConfigContext& context) noexcept {
    auto* data = owner.native.data();
    destroy_global_config_effects_008dbdb0(data + 0x2c0, context.effects);
    destroy_global_config_strings_004312b0(data + 0xa8, context.strings);
    destroy_global_config_vectors_00431210(data + 0x1c);
    void* const begin = read<void*>(data, 0x10);
    if (begin) {
        void* const end = read<void*>(data, 0x14);
        destroy_global_config_name_range_00432050(begin, end, context.strings);
        singleton_lifetime_free(read<void*>(data, 0x10)); // reload after strings
    }
    write<void*>(data, 0x10, nullptr);
    write<void*>(data, 0x14, nullptr);
    write<void*>(data, 0x18, nullptr);
    context.singleton_00f878e4 = nullptr; // unconditional, not identity-checked
    write<std::uint32_t>(data, 0, 0x00ce3818);
}
GlobalConfigOwner* scalar_delete_global_config_00432710(GlobalConfigOwner* owner,
    std::uint32_t flags, GlobalConfigContext& context) noexcept {
    auto* const original = owner;
    destroy_global_config_004325b0(*owner, context);
    if ((flags & 1u) != 0) {
        owner->~GlobalConfigOwner();
        singleton_lifetime_free(owner);
    }
    return original;
}
} // namespace bsp
