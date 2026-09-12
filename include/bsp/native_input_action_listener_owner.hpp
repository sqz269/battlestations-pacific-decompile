#pragma once
#include "bsp/native_input_action_records.hpp"
#include "bsp/native_ref_counted.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
// Non-owning storage description. The actual allocation is24h, profile at+0,
// Interlocked reference count at+4, twelve bytes at+8 and four floats at+14.
struct alignas(4) NativeInputActionListenerStorage { std::byte bytes[0x24]; };

// A92B70: native ECX actual30h action, no stack arguments, RET, void result.
// Allocates/initializes a fresh listener, then releases captured action+2C and
// finally replaces that cell. No cleanup protects the fresh allocation if the
// old listener's slot0 throws. The supplied provider has no ownership itself.
void replace_native_input_action_listener_00a92b70(void* actual_action,
    NativeInputActionRecordCalls&);

// A92150: native ECX actual24h listener; one stack DWORD (lowbyte flag bit0),
// EAX original address, RET4. No reference decrement. Matching CRT free domain.
void* scalar_delete_native_input_action_listener_00a92150(void*, std::uint32_t flags) noexcept;

// Finite source binding for D5B610: slot0=BD30E0, slot04=A92150. The shared
// BD30E0 body rereads the current profile before its flags1 call. Unknown
// profiles are binding errors; this does not implement arbitrary listeners.
class NativeInputActionListenerCalls final : public NativeInputActionRecordCalls,
    public NativeRefCountedDeleteCalls {
public:
    void call_listener_slot0(void*, std::uint32_t captured_profile) override;
    void delete_vslot04(void*, std::uint32_t captured_profile, std::uint32_t flags) override;
};
} // namespace bsp
