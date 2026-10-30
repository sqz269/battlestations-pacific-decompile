#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// 00AA0D30: original ADD ECX,4; JMP 00450540. Borrow the actual manager,
// source 8-byte native string header and SAME retained raw string-pool cells.
// This ordinary Source API adds an explicit context; it is not the original
// tail-stack/register ABI. It only registers a table name, not loaded data.
// The existing checked append owns all native reads, allocation and failures.
// No local header/owner, validation, catch cleanup or startup is introduced.
void register_native_locale_table_name_00aa0d30(void* actual_manager,
    const void* actual_name_header8, NativeStringRawPoolContext& strings);
} // namespace bsp
