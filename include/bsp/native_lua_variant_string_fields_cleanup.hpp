#pragma once

#include <cstdint>

namespace bsp {

struct NativeStringPoolStorage;

// Qualified Source interface for 006EE020..006EE03D[30]. Borrow the selected
// actual N+4 length and N+8 buffer cells, plus the application's actual pool,
// manager and small-return gate cells. Caller supplies correctly typed and
// aligned C++ objects with live backing; no header/member layout is declared.
//
// Capture buffer first. Null returns without reading length or invoking either
// service. Otherwise capture length once, wrap length+1 as uint32, resolve the
// actual pool through its raw manager-publication overload, then return the
// captured block/size with the real live gate. Do not clear or reread fields.
// Native PUSH1 is an unused physical word, not an extra Source API parameter.
//
// All references must retain the real field/domain identities and lifetimes;
// no copied process cells or invented owner are supplied. The actual pool's
// canonical lifetime domain must already have its required deletion binding.
// A getter failure may escape this function. The inherited return helper is
// noexcept and keeps its existing pool/CRT/manager failure-policy boundaries.
// This C++ interface does not preserve Native register, flags, frame or stack-
// alias placement, and supplies no allocation, ownership or production binding.
void return_native_lua_variant_string_fields_006ee020(
    volatile std::uint32_t& actual_length_04,
    void* volatile& actual_buffer_08,
    NativeStringPoolStorage* volatile& actual_published_01090aa8,
    void* volatile& actual_manager_publication_01090aa0,
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4);

} // namespace bsp
