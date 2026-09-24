#pragma once

#include "bsp/system_fog_constants.hpp"
#include "bsp/system_fog_slot.hpp"
#include "bsp/native_texture_surface_getter.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Concrete D63180 owner profile. Fog/AmbientLight are semantic hypotheses.
// The native vtable word is evidence, NOT a callable host function pointer.
// MSVC Win32 layout is checked as 94h bytes, with the actual SystemFogState at
// +08h. Camera ambient and system constants must borrow these same fields.
struct SystemFogOwner final {
    volatile std::uint32_t native_vtable_00;
    volatile long references_04;
    SystemFogState fields_08;
};

inline constexpr std::size_t kSystemFogOwnerNativeBytes = 0x94;
inline constexpr std::uint32_t kSystemFogOwnerVtable = 0x00d63180;
inline constexpr std::uint32_t kSystemFogOwnerBaseVtable = 0x00ceb130;

// New C++ ABI for native [00B84E50,00B84F61): ECX storage, EAX same, RET.
// Storage must be aligned, writable, at least sizeof(SystemFogOwner) bytes,
// with no live owner/reference being replaced. Before placement construction,
// retain raw allocation bytes [28h,68h); restore them into the actual directional
// projection. The constructor does NOT define those four float4 records.
// Supplied initialized backing preimages support deterministic in-place use.
SystemFogOwner* initialize_system_fog_owner_00b84e50(void* storage) noexcept;

// Actual nine live cells read by B84E50[273], in original MOVSS order. The
// first two are captured before destination stores; CE7804 is captured once
// for both68/78. Remaining cells are read between the native field stores.
// Cells may alias accessible destination storage; metadata/private scratch may
// not. No float conversion, cached defaults or initial zeroed backing.
struct NativeSystemFogConstants {
    const volatile std::uint32_t& scalar_6c_74_00ce386c;
    const volatile std::uint32_t& scalar_70_00d63188;
    const volatile std::uint32_t& scalar_68_78_00ce7804;
    const volatile std::uint32_t& scalar_7c_00ce77f8;
    const volatile std::uint32_t& scalar_80_00ce3950;
    const volatile std::uint32_t& scalar_84_00ce77e8;
    const volatile std::uint32_t& scalar_88_00ce77e4;
    const volatile std::uint32_t& scalar_8c_00ce77e0;
    const volatile std::uint32_t& scalar_90_00ce77dc;
};
// Same actual94h lifetime and one store engine as the legacy overload. Snapshot
// and restore ALL allocation bytes around placement construction, then perform
// the complete native schedule. Preserves28..67 exactly. No allocation/callback,
// ownership admission, native EH or implicit raw-storage cleanup is introduced.
SystemFogOwner* initialize_system_fog_owner_00b84e50(
    void* storage, const NativeSystemFogConstants&) noexcept;

struct NativeSystemFogStorageContext {
    NativeTextureSurfaceReferenceIncrement const volatile& increment_00ce221c;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
    const volatile std::uint32_t* actual_profile_00d63180;
};
// B71940[65], actual camera receiver/current stacked argument word. Capture
// incoming BEFORE old184; identity return. Publish captured incoming, CURRENT
// increment, CURRENT decrement on capturedold. Zero uses capturedold/current0
// BD30E0 then fresh current4 B84F70/flags1. Each reached CURRENT import cell
// must hold a nonnull callable genuine binding. Null is an unmasked native
// fault, not a source exception; no eager validation/cache of both imports.
// Actual old/new must be live94h owners constructed above, same genuine CRT
// allocation/volatile-long04 domain. No atomic/registry/extra credit or host
// CameraState/FogState slot projection. Callback-written184 is left untouched.
void set_native_camera_fog_storage_00b71940(void* actual_camera,
    const volatile std::uint32_t& incoming_argument, NativeSystemFogStorageContext&);
// Genuine family terminal for a separately performed decrement. Requires SAME
// captured live owner/count already0; no decrement or implicit admission. Exact
// numeric profile/current targets only; reject missing/changed bindings. The
// borrowed table must remain valid through dispatch; no raw reads after free.
void invoke_native_system_fog_zero(SystemFogOwner*, NativeSystemFogStorageContext&);
// Current-import/provider failures retain native stores and prior credits; no
// rollback is added. Callers retain backing/outstanding ownership for disposition.
// No concurrent mutation, arbitrary derived profile, native FH3/private ABI or
// full camera/application/game admission follows from these source interfaces.

// Real singleton_lifetime_allocate/free host CRT boundary: request native94h,
// allocate sizeof(SystemFogOwner), then run the same in-place initializer.
// Starts with one reference. No zeroed allocation or directional defaults.
SystemFogOwner* allocate_system_fog_owner();

// Only nonnull views originating from a live SystemFogOwner::fields_08 may be
// mapped or retained/released. Standalone diagnostic SystemFogState values and
// foreign vtable profiles are NOT accepted. No registry or second state copy.
SystemFogOwner* system_fog_owner_from_state(const SystemFogState*) noexcept;

void retain_system_fog_owner(SystemFogOwner&) noexcept;
void release_system_fog_owner(SystemFogOwner&) noexcept;

// Statically resolved concrete D63180 vtable+0 (00BD30E0) -> vtable+4
// (00B84F70, flag1). The invoker permits null, does not decrement itself, and
// is not arbitrary-subclass dispatch. Deleting destructor requires nonnull;
// bit0 frees through singleton_lifetime_free after restoring base vtable.
// Returns the original address even when freed; flags0 leaves storage intact,
// logically destroyed. It must not subsequently be retained/released.
void invoke_system_fog_deleting_destructor_00bd30e0(SystemFogOwner*) noexcept;
SystemFogOwner* delete_system_fog_owner_00b84f70(
    SystemFogOwner*, std::uint32_t flags) noexcept;

// Pass the actual camera+184 / world+10 live projection slot. Nonnull old/new
// views must belong to this concrete owned profile. Identity self-assignment
// returns; otherwise store new, increment new, decrement/release old.
// Native methods: ECX camera/world, one stack owner pointer, RET4.
void set_system_fog_camera_owner_00b71940(
    const SystemFogState*& actual_camera_slot, SystemFogOwner* new_owner) noexcept;
void set_system_fog_world_owner_00bbdf20(
    const SystemFogState*& actual_world_slot, SystemFogOwner* new_owner) noexcept;

// Native camera constructor store [00B71AE3,00B71AE9); fresh slot only.
void initialize_system_fog_camera_slot_00b71ae3(const SystemFogState*&) noexcept;
// Native [00B71F68,00B71F8A): old view stays published through zero-reference
// destruction/free, then the actual slot is cleared. Null skips the store.
void clear_system_fog_camera_slot_00b71f68(const SystemFogState*&) noexcept;

// Same publication/refcount order, accepting an actual raw owner slot as well
// as the legacy field-view slot. The descriptor never mirrors either word.
void set_system_fog_camera_owner_00b71940(SystemFogSlotRef, SystemFogOwner*) noexcept;
void initialize_system_fog_camera_slot_00b71ae3(SystemFogSlotRef) noexcept;
void clear_system_fog_camera_slot_00b71f68(SystemFogSlotRef) noexcept;

// Native float4 setters perform four forward integer read/store pairs.
// Sources may overlap destinations, including a one-word shift; this is NOT
// memmove/snapshot semantics. Native directional index is unchecked uint32
// arithmetic; callers must supply a valid resulting writable record.
void set_system_fog_color_00b84c40(SystemFogOwner&, const void* rgba_words) noexcept;
void set_system_fog_underwater_color_00b84c70(SystemFogOwner&, const void* rgba_words) noexcept;
void set_system_fog_directional_color_00b84fa0(
    SystemFogOwner&, const void* rgba_words, std::uint32_t index) noexcept;

// New raw-word C++ interfaces for the eleven native MOVSS setters. Accept
// float32 bits without a C++ float argument conversion; preserve NaNs exactly.
// The environment caller's preceding FLD/FSTP is a separate fragment.
void set_system_fog_scalar_68_00b84d00(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_6c_00b84d10(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_70_00b84d20(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_74_00b84d30(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_78_00b84d40(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_7c_00b84d50(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_80_00b84d60(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_84_00b84d80(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_88_00b84dc0(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_8c_00b84de0(SystemFogOwner&, std::uint32_t bits) noexcept;
void set_system_fog_scalar_90_00b84e00(SystemFogOwner&, std::uint32_t bits) noexcept;

} // namespace bsp
