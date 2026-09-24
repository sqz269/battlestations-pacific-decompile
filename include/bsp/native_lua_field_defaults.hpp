#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// Complete BD61C0 normal body. Original ECX is an 8-byte {tag,destination}
// pair; stack points to an 8-byte fallback pair; RET4. Dispatch uses ONLY
// the field tag, never the fallback tag. Pointers/headers are actual storage.
// Tags2/5/6/A preserve sequential x87 FLD/FSTP effects and overlap; tag7 uses
// the existing exact matrix entry. Tag8 copies four DWORDs sequentially.
// Tag0 captures its C string before resizing the actual header through the
// canonical raw pool, then copies the CURRENT length to CURRENT data.
// Unknown tags (including9) do nothing. No Lua read, numeric conversion,
// protected-call boundary, rollback, native ABI or SEH claim is introduced.
void store_native_lua_field_default_00bd61c0(void* actual_field_pair,
    const void* actual_fallback_pair, NativeStringRawPoolContext&);
} // namespace bsp
