#pragma once

#include "bsp/sound_lifetime_access.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual Win32 allocation, not InputTickState or an owning C++ container.
// A93DA0 writes profile D5B630, +4/+8/+C action base/count/capacity,
// +10/+14/+18 DWORD base/count/capacity, byte1 at+1C and DWORD1 at+20.
// Bytes+1D..1F are deliberately left untouched. No implicit initialization,
// destruction, or second copy of any native field is provided by this type.
struct alignas(4) NativeInputActionOwnerStorage {
    std::byte bytes[0x24];
};
static_assert(sizeof(NativeInputActionOwnerStorage) == 0x24);
static_assert(alignof(NativeInputActionOwnerStorage) == 4);

// Required providers act on the SAME raw12h header passed to them. Both
// native routines take a signed count, ECX header, RET4. A93DD0 passes zero;
// do not replace nonempty record cleanup with a header clear or empty-table
// success. The concrete raw record/vector implementation is a separate packet.
struct NativeInputActionOwnerCalls {
    virtual ~NativeInputActionOwnerCalls() = default;
    virtual void call_0086a430(void* actual_dword_header, std::int32_t count) = 0;
    virtual void call_00a93c10(void* actual_action_header, std::int32_t count) = 0;
};

// Borrow the real publication cell and the application's existing lifetime
// access. Use its raw01090AA0 publication form for native manager storage.
// The semantic form remains an existing fixture service, never a new domain.
// Register/delete dispatch must identify this actual24h pointer/profile.
struct NativeInputActionOwnerContext {
    void* volatile& publication_00f8bbf8;
    SoundLifetimeAccess lifetime;
    NativeInputActionOwnerCalls& records;
};

// 004BEC00: no native inputs, EAX, plain RET. Fast path returns its captured
// pointer. Slow path captures the first manager's section, enters/rechecks,
// allocates24h, constructs and publishes, gets the manager AGAIN, reloads
// publication for registration, leaves the captured section, then reloads
// the return. Registration failure retains the published allocation.
void* get_native_input_action_owner_004bec00(NativeInputActionOwnerContext&);

// A93DA0: native ECX actual24h allocation, EAX same address, plain RET.
void* construct_native_input_action_owner_00a93da0(void* actual_owner) noexcept;

// A93DD0: native ECX owner, no stack arguments/result, RET. Resize+free
// header10, then header04; reread each base AFTER its provider returns.
// Clear publication unconditionally and stamp CE3818 on success or native
// base unwind. Does NOT unregister. Raw-manager pop deletion already removes
// its entry; explicit early deletion needs the caller's unregister schedule.
// If the first provider throws, unwind also destroys/frees action header04.
// If the second throws, do not retry it or free its base. A second exception
// during cleanup terminates under the source C++ unwind contract.
void destroy_native_input_action_owner_00a93dd0(
    void* actual_owner, NativeInputActionOwnerContext&);

// A93E50: native ECX owner, stack flags, EAX original address bits, RET4.
// Test flags bit0 only AFTER destruction; optionally free that same allocation.
void* scalar_delete_native_input_action_owner_00a93e50(
    void* actual_owner, std::uint32_t flags, NativeInputActionOwnerContext&);

// These explicit-service C++ APIs preserve actual owner storage and supported
// C++ exception cleanup, not original FH3/SEH metadata, hardware-fault behavior,
// native argument/register ABI or arbitrary mutable EH-spill aliases. Fixed
// current CRT allocation/free services are used. No application/provider
// completion or game validation is implied by compiling this owner packet.
} // namespace bsp
