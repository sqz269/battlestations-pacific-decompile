#pragma once

#include "bsp/sound_lifetime_access.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {

// The application allocates exactly F8h at0073DD6C and passes that SAME
// address to A982D0. These bytes have no implicit initialization/destruction.
// Profiles are native identity DWORDs, not callable source C++ vtables.
struct alignas(4) NativeInputBackendStorage { std::byte bytes[0xf8]; };
struct alignas(4) NativeInputBackendClassStorage { std::byte bytes[0x24]; };
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeInputBackendStorage) == 0xf8);
static_assert(sizeof(NativeInputBackendClassStorage) == 0x24);

// Concrete SDK acquisition adapter. Each recorded pointer represents ONE
// returned/added COM reference; tracking does not acquire an extra reference.
// No destructor Release: the caller explicitly releases after every raw owner,
// device wrapper and enumeration callback has stopped borrowing the interfaces.
// This owns neither native storage nor a publication/manager. It tracks only
// the backend's Create/AddRef calls, not references acquired by device factories.
class NativeInputBackendDirectInput final {
public:
    NativeInputBackendDirectInput() = default;
    NativeInputBackendDirectInput(const NativeInputBackendDirectInput&) = delete;
    NativeInputBackendDirectInput& operator=(const NativeInputBackendDirectInput&) = delete;
    // Real DirectInput8Create(module,800h,IID_IDirectInput8A,actual_output,null).
    // Preserve a nonnull returned output even when the HRESULT reports failure.
    void create_interface_00c2e016(void* module, void** actual_output);
    void add_ref_vslot04(void* actual_interface);
    std::int32_t last_create_result() const noexcept { return create_result_; }
    std::size_t pending_references() const noexcept { return references_.size(); }
    void release_tracked_references();
private:
    std::vector<void*> references_;
    std::int32_t create_result_{static_cast<std::int32_t>(0x8000000au)};
};

// Required raw-device providers. Existing InputDevice/InputFocusBackendState
// projections are NOT valid arguments. A concrete application binding must
// resolve actual profiles and operate on these same native allocations/slots.
class NativeInputBackendHost {
public:
    virtual ~NativeInputBackendHost() = default;
    // Actual COM EnumDevices(interface,type,A982B0,backend,flags), with A982B0
    // forwarding to A98030 against THIS raw backend. HRESULT is ignored.
    // The provider must contain callback exceptions at the SDK boundary.
    virtual void enumerate_devices_vslot10(void* actual_interface,
        std::uint32_t type, void* actual_backend, std::uint32_t flags) = 0;
    // A9A5A0: ECX supplied raw240h allocation, stack controller index, RET4,
    // EAX constructed pointer. The constructor owns its own member unwind;
    // this caller frees the allocation when that constructor throws.
    virtual void* construct_xinput_device_00a9a5a0(void* actual_device,
        std::int32_t index) = 0;
    // A904E0: ECX raw backend, stack slot then device, RET8. Query the class
    // through device vtable+08; slot -1 scans its eight columns. Store the
    // assigned slot at device+08 and the pointer in the raw backend table.
    // No retain occurs. A returned device is not freed if attachment throws.
    virtual void attach_device_00a904e0(void* actual_backend,
        std::int32_t slot, void* actual_device) = 0;
    // ECX current device, no stack args. Native targets include A93E80 RET
    // and joystick A98400's three actual force-feedback reset commands.
    virtual void reset_device_vslot14(void* actual_device) = 0;
    // ECX captured device AFTER its real +4 InterlockedDecrement returned0.
    // No stack flags. Resolve its CURRENT slot-zero profile and consume it.
    virtual void zero_references_device_vslot00(void* actual_device) = 0;
};

struct NativeInputBackendOwnerContext {
    SoundLifetimeAccess lifetime; // must borrow actual01090AA0, not a semantic domain
    void* volatile& global_00f8bbf4;
    NativeInputBackendHost& host;
    NativeInputBackendDirectInput& direct_input;
};

// Complete normal source behavior and documented C++ exception cleanup.
// Native ECX raw owner, EAX=this for constructors, RET; deleting entries take
// one stack flags word and RET4. Added context changes the callable ABI.
// Unwritten native bytes remain untouched; no shadow class/device arrays exist.
void* construct_native_input_backend_base_00a908a0(void*, NativeInputBackendOwnerContext&);
void destroy_native_input_backend_base_00a90940(void*, NativeInputBackendOwnerContext&);
void* scalar_delete_native_input_backend_base_00a909e0(void*, std::uint8_t flags,
    NativeInputBackendOwnerContext&);

// Class+08/+0C/+10 and +18/+1C/+20 are zeroed. +0/+4/+14 stay untouched.
// Destruction frees second then first buffer and zeros those same six words.
void* construct_native_input_backend_class_00a914c0(void*) noexcept;
void destroy_native_input_backend_class_00a90dc0(void*) noexcept;
void* construct_native_input_backend_groups_00a91570(void*, NativeInputBackendOwnerContext&);
void destroy_native_input_backend_groups_00a90e00(void*, NativeInputBackendOwnerContext&);
void* scalar_delete_native_input_backend_groups_00a91150(void*, std::uint8_t flags,
    NativeInputBackendOwnerContext&);

// Actual10h GUID-vector receiver is backend+E4. Preserve its leading DWORD.
void destroy_native_input_backend_guids_00a97b00(void*) noexcept;
void* construct_native_input_backend_00a982d0(void*, NativeInputBackendOwnerContext&);
void* scalar_delete_native_input_backend_00a97c00(void*, std::uint8_t flags,
    NativeInputBackendOwnerContext&);

// Host allocation wrapper for the caller's F8h allocation + A982D0. Free the
// captured allocation if construction throws; native surviving publication
// is deliberately not repaired here. This is not another recovered entry.
void* create_native_input_backend(NativeInputBackendOwnerContext&);

// Scalar flags1 requires the existing singleton_lifetime_allocate/free domain.
// Native destructor performs NO DirectInput Release. Raw manager drain/profile
// admission, device producers/refcounted slot-zero dispatch, and application
// bindings remain caller obligations. No original FH3/SEH or hardware-fault
// compatibility, typed-device layout compatibility, or game validation claimed.
} // namespace bsp
