#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua integer keys require MSVC Win32.
#endif

namespace bsp {

struct NativeStringRawPoolContext;

// Whole game-owned 007814E0, ordinary void Source API; no Native EAX promise.
// Read CURRENT LEN+4 first. LEN0 does not access DATA as a field, call services
// or clear LEN/DATA: late actual DWORD HOME -> number+0C -> kind0 only.
// Nonzero LEN captures CURRENT DATA; a nonnull block and initial LEN+1 wrap
// are retained across the genuine raw-cell getter/return. Then late HOME ->
// DATA0 -> LEN0 -> captured number+0C -> full kind0. NULL DATA skips services.
// A late HOME alias is a distinct input role, not an eager DATA-field read.
//
// Caller retains real key16/backing and initialized readable4B HOME, actual
// RawCtx AA8/AA0/AA4 cells, pool/manager/code/allocator and synchronization.
// No signedness/range interpretation, type birth, semantic string/key cast,
// owner/Frame/default, field repair, guard, cleanup, catch, refund or retry.
// Getter escape stops before HOME/stores; lower return noexcept/CRT/gate
// limits and actual partial effects remain. Completion grants no ownership,
// private-home/Native ABI/FH3/fault/runtime or parent-builder/VM readiness.
void set_native_lua_key_integer_007814e0(
    void* actual_key16, const void* actual_input_word_home,
    NativeStringRawPoolContext& actual_strings);

} // namespace bsp
