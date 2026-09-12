#pragma once
#include <cstdint>

namespace bsp {
// These providers resolve captured targets on the SAME actual allocations.
// AE NativeInputDeviceRuntime supplies the device/DirectInput operations;
// startup's exact4B4630 identity dispatcher supplies its supported callback.
// No numeric native address is invoked as a source function pointer.
class NativeInputBackendBindingsCalls {
public:
    virtual ~NativeInputBackendBindingsCalls() = default;
    virtual std::uint32_t device_class_vslot08(void*, std::uint32_t captured_profile) = 0;
    virtual std::int32_t identifier_vslot34(void*, std::uint32_t captured_profile) = 0;
    virtual void delete_vslot04(void*, std::uint32_t captured_profile, std::uint32_t flags) = 0;
    virtual void enumerate_devices_vslot10(void* captured_direct_input,
        std::uint32_t type, void* actual_backend, std::uint32_t flags) = 0;
    // Native ECX class, EDX previous count or -1; no stack args/result used.
    virtual void devices_changed_d8(std::uint32_t captured_identity,
        std::uint32_t class_index, std::int32_t index) = 0;
};
struct NativeInputBackendBindingsContext { NativeInputBackendBindingsCalls& calls; };

// Source storage contract for the checked STL one-pointer insertion used by
// A91620. Not an implementation/ABI replacement of native A91430/A91260.
// actual10h header is {untouched word,begin,end,capacity}; iterator output is
// actual8h {header,position}. Capture the pointed-to value before allocation,
// then use current header fields at the native copy/free/publication points.
// Valid readable ranges, complete DWORD elements and representable allocation
// sizes are required. Allocations use singleton_lifetime_allocate/free. No
// shadow vector or owner; the existing backend class destructor frees it.
void insert_input_active_pointer_storage(void* actual_iterator_output,
    void* actual_header, void* captured_position, const void* actual_value_word);

// Complete normal bodies in the stated actual-storage/provider domain. The
// native class/slot are raw DWORD arguments (no inserted signed range checks).
// Added context changes the source ABI; original FH3/SEH is not reproduced.
// A91620: ECX F8h backend, class/slot stack, RET8. Scan all accepted IDs, append
// once if accepted and absent; dirty precedes allocation, callback follows it.
void activate_native_input_device_slot_00a91620(void*, std::uint32_t class_index,
    std::uint32_t fixed_slot, NativeInputBackendBindingsContext&);
// A90EE0: ECX backend, raw device stack, RET4. First active match erased in
// order; no release/retain. Current end is decremented after real memmove_s.
void remove_active_native_input_device_00a90ee0(void*, void*, NativeInputBackendBindingsContext&);
// BEBF30: ECX backend, class stack, RET4. Eight fixed slots, direct captured
// scalar04(flags1), then clear that same cell. No refcount decrement/BD30E0.
void delete_native_input_device_class_00bebf30(void*, std::uint32_t class_index,
    NativeInputBackendBindingsContext&);
// A983C0: ECX backend, RET. Capture +E0 before clearing byte+F4, then real
// EnumDevices(interface,type0,A982B0,backend,flags1), HRESULT ignored.
void enumerate_native_input_devices_00a983c0(void*, NativeInputBackendBindingsContext&);
} // namespace bsp
