#pragma once

#include "bsp/native_animation_channel_body_reader.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {

// Complete B8A3C0 against an actual data/count/capacity key-vector header.
// Growth delegates to the existing B76680 reserve, then zeroes ten DWORDs per
// new key. Shrink changes count only; no key payload destructor is called.
void resize_native_animation_keys_00b8a3c0(void* actual_header,
    std::int32_t requested);

// Complete channel destructor/scalar deletion. Key storage is resized to zero
// and freed before the raw pooled name is returned and BD30F0 stamps CEB130.
// The scalar wrapper tests only flags bit0 after destruction and returns the
// original address, including after free. These are source ABIs, not RET/SEH.
void destroy_native_animation_channel_00b8a7d0(void* actual_channel,
    NativeStringRawPoolContext& strings);
void* delete_native_animation_channel_00b8ad60(void* actual_channel,
    std::uint32_t flags, NativeStringRawPoolContext& strings);

// Complete group destructor/scalar deletion. Destruction stamps D62ED4, walks
// the CURRENT vector/count from the back, captures each slot address and owner,
// then invokes that owner's captured profile slot04 with flags1. The captured
// slot is cleared only after the call returns; count is reloaded afterward.
void destroy_native_animation_channel_group_00b78530(void* actual_group,
    NativeStringRawPoolContext& strings, NativeRefCountedDeleteCalls& deletes);
void* delete_native_animation_channel_group_00b78d00(void* actual_group,
    std::uint32_t flags, NativeStringRawPoolContext& strings,
    NativeRefCountedDeleteCalls& deletes);

struct NativeAnimationDeleteContext {
    NativeStringRawPoolContext& strings;
    const volatile std::uint32_t* actual_group_profile_00d62ed4;
    const volatile std::uint32_t* actual_channel_profile_00d632b0;
};

// Finite actual-profile binding for the group and channel terminals. The
// captured numeric profile selects the supplied application table, whose
// CURRENT slot04 is loaded for every dispatch. Other profiles/slots fail; no
// fallback deletion or reference decrement is invented.
class NativeAnimationDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    explicit NativeAnimationDeleteCalls(NativeAnimationDeleteContext context) noexcept
        : context_(context) {}
    void delete_vslot04(void* actual_owner, std::uint32_t captured_profile,
        std::uint32_t flags) override;
private:
    NativeAnimationDeleteContext context_;
};

// Requires actual20h groups, actual28h channels/keys, the canonical raw string
// pool context and live D62ED4/D632B0 table words. No binary ABI, original FH3/
// SEH identity, arbitrary native vtable execution or gameplay proof is claimed.
} // namespace bsp
