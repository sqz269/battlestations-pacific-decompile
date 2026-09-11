#pragma once

#include <cstdint>

namespace bsp {
class SizedStoragePool;
struct SingletonLifetimeCallbacks;

// Borrowed actual immutable native tables, each at least four DWORDs. Numeric
// profile identities stored in the resource are mapped to these actual views;
// they are never treated as callable host addresses or copied resource owners.
struct NativeRenderResourceAccountingTables {
    const volatile std::uint32_t* texture_2d_00d61948;
    const volatile std::uint32_t* texture_cube_00d61870;
    const volatile std::uint32_t* texture_volume_00d618b0;
};

// Complete four-byte ECX-self/EAX-result/plain-RET leaf: read actual DWORD+24.
std::uint32_t native_resource_accounted_size_00b3ce30(const void* actual_resource) noexcept;
// Complete XOR EAX,EAX; RET. Does not read ECX or any resource bytes.
std::uint32_t native_resource_zero_accounted_size_00a82250() noexcept;

// Full 00B31DC0, native ECX container, stack original eight-byte name, RET4,
// no semantic return. Container is actual renderer+1A74: records+4, count+8,
// capacity+C, accounting+10. Uses the actual 2Ch records and 10h alias nodes.
//
// Match-time domain: resource's current profile is D61948, D61870 or D618B0;
// its selected borrowed table is valid/immutable, with exact +0C entry B3CE30
// (2D) or A82250 (cube/volume). Unsupported profiles/entries are outside this
// interface's preconditions. No fallback behavior is supplied. Unused tables
// need not be available, including all tables for a not-found call.
// Actual names/list storage and callback repairs must remain readable at native
// accesses. Comparison is the real host CRT _stricmp and its current locale.
// Does not retain/release the matched resource or free the record array.
// Native EH registration and game execution are separate validation boundaries.
void remove_native_render_resource_by_alias_00b31dc0(void* actual_container,
    const void* actual_name_header, SizedStoragePool& actual_string_pool,
    const SingletonLifetimeCallbacks&, const NativeRenderResourceAccountingTables&);

} // namespace bsp
