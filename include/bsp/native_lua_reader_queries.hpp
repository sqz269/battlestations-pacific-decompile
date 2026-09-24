#pragma once
#include "bsp/native_lua_reader.hpp"

namespace bsp {

// Actual BD5F50 output producer: 250 eight-byte records, then a DWORD count.
// tag0 contains a borrowed C-string address, tag1 signed integer bits, tag2
// float32 bits. Unused records retain the caller's preimage. Native writes are
// unchecked: the successful domain requires at most 250 supported keys.
struct NativeLuaReaderKey {
    volatile std::uint32_t tag;
    volatile std::uint32_t bits;
};
struct NativeLuaReaderKeys {
    NativeLuaReaderKey keys[250];
    volatile std::uint32_t count_7d0;
};
static_assert(sizeof(NativeLuaReaderKey) == 8);
static_assert(offsetof(NativeLuaReaderKeys, count_7d0) == 0x7d0);
static_assert(sizeof(NativeLuaReaderKeys) == 0x7d4);

// Caller-owned fresh scratch supplies the preimage for opaque0C/padding11..13.
// The routine constructs table, key, value in that order; assignment copies
// only owner/kind/index/tracked. Their actual addresses enter owner tracking.
struct NativeLuaReaderQueryScratch {
    NativeLuaObjectStorage table;
    NativeLuaObjectStorage key;
    NativeLuaObjectStorage value;
};

// BD5EB0: ECX reader, stacked tag/bits, bool AL, RET8. scratch is fresh, not a
// live owner reference; it is initialized by BD5790 then destroyed. This is
// !IsNil, so an unsupported tag's unbound result returns true.
bool native_lua_reader_has_key_00bd5eb0(NativeLuaReaderStorage& reader,
    NativeLuaObjectStorage& scratch, std::uint32_t tag, std::uint32_t bits);

// BD5F50: ECX reader, stacked output pointer, RET4. Exact STRING precedes
// integer-number predicate/accessor and remaining NUMBER float32 conversion.
// crt_sse2_conversion aliases the live native0109EEA4 decision (no default).
// Require nonempty/stable reader, valid table, <=250 supported keys, valid
// owner slot/ref capacity, and successful raw Lua operations. No protected
// adapter, registry cursor, native FH3/invalid-parameter/ABI or game claim.
// Scratch, output, reader and owner storage must not alias each other.
void native_lua_reader_enumerate_keys_00bd5f50(NativeLuaReaderStorage& reader,
    NativeLuaReaderKeys& output, NativeLuaReaderQueryScratch& scratch,
    const bool& crt_sse2_conversion);

} // namespace bsp
