#pragma once

#include "bsp/native_resource_support.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
class NativeRenderActualOwners;

inline constexpr std::size_t native_hardware_layout_bytes = 0x44;
inline constexpr std::size_t native_hardware_layout_slot_bytes = 0x48;

// Borrow the actual globals, initialized pools and immutable native tables.
// Renderer dispatch supports the evidenced D5F0A8 profile (+44 = B2F4C0).
// CPU declaration dispatch supports D61D1C (+00 = BD30E0, +04 = B48CA0).
// Other runtime profiles are outside this interface's input domain. Tables
// contain original DWORD addresses, not host function pointers or callbacks.
struct NativeHardwareLayoutOwnerContext {
    void* volatile& actual_renderer_00f8d394;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    void* actual_tree_0108d530;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeResourceSupportStorage* volatile& actual_support_0108fedc;
    SoundLifetimeAccess actual_lifetime_01090aa0;
    const volatile std::uint32_t* actual_declaration_profile_00d61d1c;
    const volatile std::uint32_t* actual_type_sizes_00d61cc0;
    void* actual_declaration_pool_0108fd38;
    void* actual_hardware_layout_pool_0108fe9c;
    // Connected callers use the SAME existing declaration companions when
    // their actual +04 reaches zero. Null retains the original raw-only
    // interface; never bind a companion and then bypass its retirement.
    NativeRenderActualOwners* canonical_declaration_owners{};
};

// B483F0: native ECX actual 0Ch record, RET. Capture its first-word declaration,
// perform real InterlockedDecrement at declaration+04 and current virtual
// zero-reference/deleting dispatch. Clear record+00 only after successful
// release. Preserve the other eight bytes and a null first word.
void destroy_native_hardware_layout_record_00b483f0(
    void* actual_record, NativeHardwareLayoutOwnerContext&);

// B48950: native ECX is FOUR RECORDS BEGIN, not owner. Reverse four 0Ch records;
// on failure destroy the remaining lower prefix without retrying the failed
// record. A second exception during native array unwinding terminates.
void destroy_native_hardware_layout_records_00b48950(
    void* actual_records_begin, NativeHardwareLayoutOwnerContext&);

// B48960: native ECX 44h owner, RET. Install base hardware profile D61D10,
// read current renderer/profile/+44 notification, remove from actual tree,
// reverse-destroy records+08..+37 and finally write base profile CEB130.
void destroy_native_hardware_layout_base_00b48960(
    void* actual_owner, NativeHardwareLayoutOwnerContext&);

// B60700: native ECX owner, RET. Install D62AF4, capture COM+40, release through
// its current COM table and clear current +40, then access actual support and
// run the complete base destructor. Base cleanup also runs on COM/support
// exceptions; a second exception during that cleanup terminates.
void destroy_native_hardware_layout_00b60700(
    void* actual_owner, NativeHardwareLayoutOwnerContext&);

// B60770: native ECX owner, stack flags, EAX original owner, RET4. Complete
// destruction, then return to actual hardware pool only when flags bit0 is set.
void* delete_native_hardware_layout_00b60770(
    void* actual_owner, std::uint32_t flags, NativeHardwareLayoutOwnerContext&);

// B60110: native ECX pool, stack raw48h slot, RET4. Borrow the pool's live
// Win32 CRITICAL_SECTION+0C, depth+24, slab table+28 and earliest slab+34.
// Slot+44 is its slab index. Each32-slot slab has WORD free indices+900 and
// WORD count+940. Signed wrapped slot displacement divides by48h; publication
// reloads the current count after writing the free index, including aliases.
void return_native_hardware_layout_slot_00b60110(void* actual_pool, void* raw_slot);

// New MSVC Win32 interfaces. No full pool initialization, arbitrary virtual
// profile support, constructor, native calling convention or game proof.
} // namespace bsp
