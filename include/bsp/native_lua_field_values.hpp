#pragma once
#include "bsp/native_lua_objects.hpp"

namespace bsp {
// Actual callable cells installed by BD4FC0. Both native calls pass their
// sole argument in ECX and return a DWORD in EAX. Read the current cell at
// each call; neither null validation nor a logical Lua registry is introduced.
using NativeLuaFieldNumberResolver = std::uint32_t (__fastcall*)(std::int32_t);
using NativeLuaFieldTableResolver = std::uint32_t (__fastcall*)(NativeLuaObjectStorage*);
struct NativeLuaFieldValueBindings {
    NativeStringRawPoolContext& strings;
    const bool& crt_sse2_conversion;
    NativeLuaFieldNumberResolver const volatile& resolver_0109ced4;
    NativeLuaFieldTableResolver const volatile& resolver_0109ced8;
};
// Bytes corresponding to native ESP+10h..83h after the four saved registers.
// Supply initialized caller-owned storage and keep its address stable. No
// blanket initialization: child-object padding and other unwritten bytes keep
// their preimage. The five temporary objects share the REAL owner ref slots.
struct alignas(4) NativeLuaFieldValueScratch { std::byte bytes[0x74]; };
// Complete BD63B0 normal body [BD63B0,BD6827). Native ECX actual LuaObject,
// EDX actual8h {tag,destination} pair, RET. New explicit context/scratch ABI.
// Tag A copies atol's signed32 result BITS, not its floating numeric value.
// Tags9/unknown and unsuccessful tag4 classification do not write the field.
// Lua/CRT nonlocal failures, invalid raw pointers, native FH3 cleanup and the
// installed resolver implementations remain caller/library boundaries. This
// entry supplies no protected Lua call, rollback or native exception handler.
void store_native_lua_field_value_00bd63b0(NativeLuaObjectStorage&,
    void* actual_field_pair,NativeLuaFieldValueScratch&,NativeLuaFieldValueBindings&);
} // namespace bsp
