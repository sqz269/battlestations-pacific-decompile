#pragma once

namespace bsp {
// Full 00715BF0..00715C04. Original ECX is a valid intrusive-node wrapper;
// EAX returns 0 or 1, with no stack arguments. This is a new C++ interface.
// A null contained node is accepted; a null wrapper is not a valid input.
bool native_structured_node_has_remaining_00715bf0(const void* wrapper) noexcept;
} // namespace bsp
