#pragma once
#include <cstdint>

namespace bsp {
// Borrow the live original-value cells. The native routine reloads +20 several
// times; neither a copied timing structure nor a fixed default is substituted.
struct NativeInputActionTimingGlobals {
    const volatile float& quick_edge_and_hold_00e12f20;
    const volatile float& repeat_delay_00e12f24;
    const volatile float& repeat_step_00e12f28;
};

// A91A50: ECX actual24h listener, stack float delta then two DWORD slots whose
// LOW BYTES are previous/current. RET0C. Only +8..+23 are written; profile+0
// and refcount+4 remain borrowed. Producer A92B70 allocates24h, stampsD5B610,
// starts refcount1 and initializes these exact flags/timers. This does not own
// or construct that allocation. The x87 operation/store/comparison order is kept.
void update_native_input_action_listener_00a91a50(void* actual_listener,
    float seconds, std::uint8_t previous, std::uint8_t current,
    const NativeInputActionTimingGlobals&) noexcept;

class NativeInputActionTickCalls {
public:
    virtual ~NativeInputActionTickCalls() = default;
    // The caller has already loaded/spilled delta through x87 and captured
    // backend/profile. Source binding routes this to actual raw A918A0.
    virtual void backend_vslot04(void* actual_backend,
        std::uint32_t captured_profile, float seconds) = 0;
    // Separate raw binding provider packet; same owner/record allocations.
    virtual void call_00a922a0(void* actual_owner) = 0;
    virtual void call_00a92370(void* actual_record) = 0;
    // Native tail loads/tests F8BBFC once, then calls the captured identity with
    // no semantic arguments/result. Known writer4DD71B installs6965A0, which
    // reloads E188A8 and tail-jumps4D8CD0. A reached provider must do real work.
    virtual void post_tick_callback_00f8bbfc(std::uint32_t captured_identity) = 0;
};

struct NativeInputActionTickContext {
    void* volatile& backend_00f8bbf4;
    const volatile std::uint32_t& callback_00f8bbfc;
    const volatile float& zero_00d7a218;
    NativeInputActionTimingGlobals timing;
    NativeInputActionTickCalls& calls;
};

// A92C40: ECX actual24h action owner, stack float delta, RET4. Uses its real
// +4 action base/+8 count and 30h records. Captures each record across polling,
// then reloads owner count/base for the endpoint. Callbacks can change later
// slots, counts, globals, flags and listeners; no vector snapshot or guard is
// substituted. Valid original storage is required; exceptions preserve partial
// effects and skip the remaining schedule. No ownership/cleanup is added.
void update_native_input_action_owner_00a92c40(void* actual_owner,
    float seconds, NativeInputActionTickContext&);

// New C++ ABI over actual storage, not an original callable/FH3/SEH replacement.
// No asynchronous mutation, hardware fault, SDK, frame or gameplay claim.
} // namespace bsp
