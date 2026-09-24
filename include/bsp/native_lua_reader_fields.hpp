#pragma once
#include "bsp/native_lua_field_values.hpp"
#include "bsp/native_lua_reader.hpp"

namespace bsp {
// Explicit storage for the wrapper's native temporary and StoreValue's frame.
// The lookup storage must be initialized and unbound. All storage must remain
// at stable addresses until the call returns; unwritten bytes keep their
// preimage. The value frame is used only when StoreValue is selected.
struct NativeLuaReaderFieldScratch {
    NativeLuaObjectStorage lookup;
    NativeLuaFieldValueScratch value;
};

// Complete normal paths for a valid, nonempty actual reader. Native ECX is
// the reader, followed by by-value key/field pairs (and a fallback pair for
// BD68D0), RET10h/18h. These new interfaces take explicit pointers to the
// actual 8-byte argument pairs and caller-owned frame storage, changing ABI.
// Lookup is followed by StoreValue even for nil or an unsupported key tag.
void read_native_lua_field_00bd6830(NativeLuaReaderStorage&,
    std::uint32_t key_kind, std::uint32_t key_bits, void* actual_field_pair,
    NativeLuaReaderFieldScratch&, NativeLuaFieldValueBindings&);
// Select StoreDefault only when B65FB0 reports actual Lua nil. In particular,
// a freshly unbound result from an unsupported key tag is NOT nil.
void read_native_lua_field_or_default_00bd68d0(NativeLuaReaderStorage&,
    std::uint32_t key_kind, std::uint32_t key_bits, void* actual_field_pair,
    const void* actual_fallback_pair, NativeLuaReaderFieldScratch&,
    NativeLuaFieldValueBindings&);
// Both entries destroy the lookup temporary after a successful store. Empty
// or corrupt vectors, escaping callee exceptions/nonlocal transfers, native
// FH3 cleanup and binary stack/register compatibility remain outside this
// normal-path contract. No protected Lua call or rollback is introduced.
} // namespace bsp
