#pragma once
#include <cstdint>

namespace bsp {
struct NativeRefCountedDeleteCalls {
    virtual ~NativeRefCountedDeleteCalls() = default;
    // Resolve this captured profile's original slot04 to its concrete source
    // scalar body. The callback receives flags1; the BD30E0 caller has NO flags.
    virtual void delete_vslot04(void* actual_owner, std::uint32_t current_profile,
        std::uint32_t flags) = 0;
};

// BD30E0: native ECX owner, no stack args, RET. Null does nothing; otherwise
// capture current profile and invoke slot04(flags1). No reference decrement.
// Added source provider changes the ABI and does not supply arbitrary vtables.
void invoke_native_ref_counted_delete_00bd30e0(void*, NativeRefCountedDeleteCalls&);

// BD30F0's complete seven-byte body: ECX owner, stampCEB130, RET. No reference
// decrement, null check, other field write or allocation release occurs here.
void __fastcall destroy_native_ref_counted_base_00bd30f0(void*) noexcept;
} // namespace bsp
