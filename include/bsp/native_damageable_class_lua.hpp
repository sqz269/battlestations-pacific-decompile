#pragma once
#include <cstdint>

namespace bsp {
// Complete007149D0..00714A0E[63]. Original ECX=input C string, EAX=signed
// category index or-1, RET. Borrow the actual null-terminated pointer table
// at00E08138; preserve its ordered loads and the00438E10 comparison calls.
// The comparison provider uses the host CRT _stricmp. Original CRT locale
// state, binary ABI replacement and asynchronous table mutation are unproved.
std::int32_t native_mesh_category_from_name_007149d0(
    const char* name, const char* const volatile* actual_categories_00e08138);

// 0087CA80 is deliberately source absent. Its complete body, field, ABI,
// callsite and EH audit is in docs/NATIVE_DAMAGEABLE_CLASS_LUA_ORCH4.md.
} // namespace bsp
