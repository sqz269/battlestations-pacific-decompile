#pragma once
#include "bsp/native_scene_typed_lifetime.hpp"

namespace bsp {
struct NativeAmbientIdentityContext;

// Complete 00B7C290..00B7C418 (393 bytes). Native ECX actual 98h ambient
// allocation, zero stack arguments, EAX same owner, plain RET. This new C++
// interface borrows that raw storage and the current four bytes at D7A24C.
// Capture the constant once before owner writes; keep the native store/spill
// order, vtable transitions and allocator preimage at owner+28h..37h.
// No allocation or host lighting view. The actual98h Source carrier is born
// with its byte preimage restored before the Native constant capture. The
// retained lifetime record must be fresh; normal return issues its completion.
// Pointed-to storage may overlap; input pointer values and private C++ locals
// must remain stable. Actual vtable dispatch, destruction, original-caller ABI,
// unrestricted fault recovery and gameplay are outside this constructor proof.
void* construct_native_ambient_00b7c290(
    void* owner, const void* one_00d7a24c, NativeAmbientTypedLifetime&,
    NativeAmbientIdentityContext&) noexcept;

} // namespace bsp
