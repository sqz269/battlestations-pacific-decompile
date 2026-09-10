#include "bsp/system_fog_owner.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <array>
#include <cstring>
#include <new>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error System fog ownership requires MSVC Win32 native pointer and LONG widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(std::is_standard_layout_v<SystemFogOwner>);
static_assert(std::is_trivially_destructible_v<SystemFogOwner>);
static_assert(sizeof(SystemFogOwner) == kSystemFogOwnerNativeBytes);
static_assert(offsetof(SystemFogOwner, references_04) == 4);
static_assert(offsetof(SystemFogOwner, fields_08) == 8);
static_assert(sizeof(SystemFogState) == 0x8c);
static_assert(offsetof(SystemFogState, color_08) == 0x00);
static_assert(offsetof(SystemFogState, underwater_color_18) == 0x10);
static_assert(offsetof(SystemFogState, directional_colors_28) == 0x20);
static_assert(offsetof(SystemFogState, scalar_68) == 0x60);
static_assert(offsetof(SystemFogState, scalar_90) == 0x88);

namespace {
void store_bits(float& target, std::uint32_t bits) noexcept {
    std::memcpy(&target, &bits, sizeof(bits));
}

void copy_four_words_forward(void* destination, const void* source) noexcept {
    // Keep individual MOV read/store pairs, including propagating overlap and
    // signaling-NaN payloads. memcpy of the full vector would change overlap.
    __asm {
        mov ecx, destination
        mov eax, source
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
        mov edx, dword ptr [eax + 4]
        mov dword ptr [ecx + 4], edx
        mov edx, dword ptr [eax + 8]
        mov dword ptr [ecx + 8], edx
        mov edx, dword ptr [eax + 12]
        mov dword ptr [ecx + 12], edx
    }
}

void set_counted_slot(const SystemFogState*& slot, SystemFogOwner* next) noexcept {
    const auto* old_fields = slot;
    const auto* next_fields = next ? &next->fields_08 : nullptr;
    if (old_fields == next_fields) return;
    slot = next_fields; // native publishes BEFORE InterlockedIncrement
    if (next) retain_system_fog_owner(*next);
    if (old_fields) release_system_fog_owner(*system_fog_owner_from_state(old_fields));
}
} // namespace

SystemFogOwner* initialize_system_fog_owner_00b84e50(void* storage) noexcept {
    // Capture allocation representation at native+28 BEFORE any C++ lifetime
    // starts. Do not read these bytes as floats, or replace them with zeros.
    std::array<unsigned char, 0x40> directional_preimage;
    std::memcpy(directional_preimage.data(),
        static_cast<unsigned char*>(storage) + 0x28, directional_preimage.size());
    auto* owner = ::new (storage) SystemFogOwner;
    auto& fields = owner->fields_08;

    // 00B84E50's end state: positive-zero color words, exact MOVSS constants,
    // base vtable then refcount1 then concrete vtable. No invented fields.
    std::memset(fields.color_08.data(), 0, 0x10);
    std::memset(fields.underwater_color_18.data(), 0, 0x10);
    store_bits(fields.scalar_68, 0x3ecccccd);
    store_bits(fields.scalar_6c, 0x43480000);
    store_bits(fields.scalar_70, 0x45dac000);
    store_bits(fields.scalar_74, 0x43480000);
    store_bits(fields.scalar_78, 0x3ecccccd);
    store_bits(fields.scalar_7c, 0xc2f00000);
    store_bits(fields.scalar_80, 0x44480000);
    store_bits(fields.scalar_84, 0x3f7851ec);
    store_bits(fields.scalar_88, 0xc3480000);
    store_bits(fields.scalar_8c, 0xc30c0000);
    store_bits(fields.scalar_90, 0x3f7eb852);
    owner->native_vtable_00 = kSystemFogOwnerBaseVtable;
    owner->references_04 = 1;
    owner->native_vtable_00 = kSystemFogOwnerVtable;
    std::memcpy(fields.directional_colors_28.data(), directional_preimage.data(),
        directional_preimage.size());
    return owner;
}

SystemFogOwner* allocate_system_fog_owner() {
    void* storage = singleton_lifetime_allocate({SingletonAllocationKind::object,
        kSystemFogOwnerNativeBytes, sizeof(SystemFogOwner)});
    return initialize_system_fog_owner_00b84e50(storage);
}

SystemFogOwner* system_fog_owner_from_state(const SystemFogState* fields) noexcept {
    if (!fields) return nullptr;
    return reinterpret_cast<SystemFogOwner*>(reinterpret_cast<std::uintptr_t>(fields)
        - offsetof(SystemFogOwner, fields_08));
}

void retain_system_fog_owner(SystemFogOwner& owner) noexcept {
    (void)::InterlockedIncrement(&owner.references_04);
}

void release_system_fog_owner(SystemFogOwner& owner) noexcept {
    if (::InterlockedDecrement(&owner.references_04) == 0) {
        invoke_system_fog_deleting_destructor_00bd30e0(&owner);
    }
}

void invoke_system_fog_deleting_destructor_00bd30e0(SystemFogOwner* owner) noexcept {
    if (owner) {
        // Concrete D63180[0] = BD30E0, D63180[1] = B84F70. This is the
        // actual known leaf target, not a placeholder subclass callback.
        (void)delete_system_fog_owner_00b84f70(owner, 1);
    }
}

SystemFogOwner* delete_system_fog_owner_00b84f70(
    SystemFogOwner* owner, std::uint32_t flags) noexcept {
    auto* const original_address = owner;
    owner->native_vtable_00 = kSystemFogOwnerVtable;
    owner->native_vtable_00 = kSystemFogOwnerBaseVtable; // bounded 00BD30F0
    if ((flags & 1u) != 0) {
        owner->~SystemFogOwner();
        singleton_lifetime_free(owner);
    }
    // Native omitted listing bytes00B84F8B are ADD ESP,4, then this return.
    return original_address;
}

void set_system_fog_camera_owner_00b71940(
    const SystemFogState*& slot, SystemFogOwner* next) noexcept {
    set_counted_slot(slot, next);
}

void set_system_fog_world_owner_00bbdf20(
    const SystemFogState*& slot, SystemFogOwner* next) noexcept {
    set_counted_slot(slot, next);
}

void initialize_system_fog_camera_slot_00b71ae3(const SystemFogState*& slot) noexcept {
    slot = nullptr;
}

void clear_system_fog_camera_slot_00b71f68(const SystemFogState*& slot) noexcept {
    if (const auto* fields = slot) {
        release_system_fog_owner(*system_fog_owner_from_state(fields));
        slot = nullptr; // after virtual destruction/free returns, not before
    }
}

void set_system_fog_color_00b84c40(SystemFogOwner& owner, const void* source) noexcept {
    copy_four_words_forward(owner.fields_08.color_08.data(), source);
}

void set_system_fog_underwater_color_00b84c70(
    SystemFogOwner& owner, const void* source) noexcept {
    copy_four_words_forward(owner.fields_08.underwater_color_18.data(), source);
}

void set_system_fog_directional_color_00b84fa0(
    SystemFogOwner& owner, const void* source, std::uint32_t index) noexcept {
    const auto offset = std::uint32_t{0x28} + (index << 4);
    auto* destination = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(&owner) + offset);
    copy_four_words_forward(destination, source);
}

void set_system_fog_scalar_68_00b84d00(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_68, b); }
void set_system_fog_scalar_6c_00b84d10(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_6c, b); }
void set_system_fog_scalar_70_00b84d20(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_70, b); }
void set_system_fog_scalar_74_00b84d30(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_74, b); }
void set_system_fog_scalar_78_00b84d40(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_78, b); }
void set_system_fog_scalar_7c_00b84d50(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_7c, b); }
void set_system_fog_scalar_80_00b84d60(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_80, b); }
void set_system_fog_scalar_84_00b84d80(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_84, b); }
void set_system_fog_scalar_88_00b84dc0(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_88, b); }
void set_system_fog_scalar_8c_00b84de0(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_8c, b); }
void set_system_fog_scalar_90_00b84e00(SystemFogOwner& o, std::uint32_t b) noexcept { store_bits(o.fields_08.scalar_90, b); }

} // namespace bsp
