#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {
// Full B66FA0 normal body. Copy owner/kind/index/opaque DWORDs and tracked
// BYTE in native order; leave destination padding11..13 untouched. A nonzero
// tracked byte registers the SAME destination address in the actual owner.
// No destination release, self-copy guard, Lua push or registry reference.
// Requires valid source storage and the owner's existing <50/<5 slot domain.
NativeLuaObjectStorage* copy_construct_native_lua_object_00b66fa0(
    void* fresh, const NativeLuaObjectStorage& source) noexcept;

// Full BD5790 normal body over actual storage. Original ECX fresh output,
// EDX parent, stack tag/value DWORDs, EAX output, RET8. Tag0 is a C-string
// address; tag1 signed integer bits; tag2 float32 bits converted with the
// existing CVTTSS2SI helper. Other tags leave a freshly unbound output.
// The raw Lua providers retain their nonlocal-transfer contract. This is not
// a protected reader, registry projection, native FH3 handler or full reader.
NativeLuaObjectStorage* lookup_native_lua_reader_child_00bd5790(
    void* fresh, NativeLuaObjectStorage& parent,
    std::uint32_t key_kind, std::uint32_t key_bits);
} // namespace bsp
