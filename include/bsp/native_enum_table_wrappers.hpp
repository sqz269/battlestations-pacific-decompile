#pragma once

#include "bsp/game_native_string_process.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum table wrappers require MSVC Win32.
#endif

namespace bsp {

// Borrow the accepted lower APIs' actual bindings; construct/start nothing.
// E175E8 table and E17578 symbol pools must be genuine distinct initialized14h
// owners on the SAME allocator list/current generation for insertion. Never
// substitute property E17540/E175B0 pools or114h bags. The distinct E186ED enum
// empty cell is an explicit genuine stable borrowed authority: the retained
// string process has no accessor for it. Do not alias E18560 or any other role.
// Query E17654 and raw key allocation/return come from the SAME string process.
// Concrete enum-global startup/publication is outside this conditional binding.
class NativeEnumTableWrapperContext final {
public:
    NativeEnumTableWrapperContext(game::GameNativeStringProcess& actual_strings,
        void* actual_table_pool_00e175e8, void* actual_symbol_pool_00e17578,
        const char* actual_enum_empty_00e186ed) noexcept
        : strings_(actual_strings), table_pool_(actual_table_pool_00e175e8),
          symbol_pool_(actual_symbol_pool_00e17578),
          enum_empty_(actual_enum_empty_00e186ed) {}

    game::GameNativeStringProcess& strings() const noexcept { return strings_; }
    void* table_pool() const noexcept { return table_pool_; }
    void* symbol_pool() const noexcept { return symbol_pool_; }
    const char* enum_empty() const noexcept { return enum_empty_; }

private:
    game::GameNativeStringProcess& strings_;
    void* const table_pool_;
    void* const symbol_pool_;
    const char* const enum_empty_;
};

// Complete conditional ordinary Source through NEW C++ interfaces; not Original
// ECX/stack/RET/FS/EH transport. Require a coherent genuine10Ch owner/map at+4,
// stable closed ASCII CString/current C locale, live providers and matching
// current heap/pool generations. Owning graphs/keys/slots are disjoint; no
// alias/reentry/trim/mutation outside this call/overflow/provider failure/fault
// or exceptional unwind. Each wrapper creates a genuine local NativeString,
// fills its actual header with0041E870, then manually returns the key through
// the same process's raw0041DD20 composition. Its trivial destructor supplies
// no implicit release/repair or invented Original unwind equivalence.

// Whole [008F2B90,008F2C0B),123B/36 instructions. ECX outer owner, stack CString
// and incoming CEnum, RET8; no defined ordinary return is claimed. Call complete
// conditional008F17E0 on owner+4, IGNORE its count, then return the temp key.
// Both old/incoming nonnull values must be genuine unique current-CRT19Ch
// CEnums from008F4DD0/D16508 with scalar008F59C0; null is also supported. Honor
// the lower API's owning disjoint key-copy and payload lifetime qualifications.
void insert_native_cenum_table_name_008f2b90(void* actual_owner,
    const char* key, void* new_actual_cenum_or_null,
    NativeEnumTableWrapperContext& context);

// Whole [008F2C10,008F2C95),133B/41 instructions. ECX owner, stack CString,
// RET4; only AL is Boolean. Call whole0048D4E0 on owner+4 using a private
// nonalias bucket, capture node!=null BEFORE key cleanup, return Boolean.
// Missing keys normally return false. Never read mapped+8. This read starts
// no pool; the retained table/symbol bindings are not accessed by membership.
bool contains_native_enum_table_name_008f2c10(const void* actual_owner,
    const char* key, NativeEnumTableWrapperContext& context);

} // namespace bsp
