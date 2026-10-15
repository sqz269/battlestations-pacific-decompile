#pragma once
#include "bsp/native_enum_symbol_declaration.hpp"
#include <cstdint>

namespace bsp::game { class GameNativeLuaDisplayStringBindings; }
namespace bsp {
// Borrow actual initialized owners and the fixed canonical display binding.
// All raw strings and that binding must use the SAME GameNativeStringProcess.
// This creates no Lua state, publication, pool, fallback cell or callback.
struct NativeEnumLuaDeclarationContext {
    NativeEnumSymbolDeclarationContext& actual_enum_declaration;
    game::GameNativeLuaDisplayStringBindings& actual_fixed_display;
    void* volatile& actual_world_publication_00e188a8;
    const volatile std::uint32_t& actual_crt_feature_word_0109eea4;
    const char* actual_clone_empty_00f89450;
    const void* actual_type_bytes_00d162d8; // LUA_S NUL, six accessible bytes
    const char* actual_name_key_00ce8ed0;  // Name NUL
};

// Whole008F2F70 [008F2F70,008F3199),553B/152 instructions. New C++ API.
// Original ECX=genuine19Ch enum; stack owning8B NativeString by value, RET8;
// no semantic EAX count/bool. consumed_key transfers its actual buffer here:
// release once, LAST, after Lua locals. Its released header is not cleared.
void declare_native_enum_symbols_from_lua_008f2f70(void* actual_raw_19c_enum,
    NativeString& consumed_key, NativeEnumLuaDeclarationContext&);

// Copy through NUL into genuine128B enum+11C name area (must fit), then
// resize type114/118 to5 preservefalse and copy its CURRENT pointer/length.
// Only THEN read current E188A8, using SAME actual world+1A0C constructed4C8h
// Lua owner. All14h temporaries have stable tracked addresses on that owner.
// Use raw Lua5.1.1 APIs, actual key/value iteration and Name/display/declaration;
// no filters, duplicate bypass, protected-reader adapter or substitute owner.
// The owning symbol map uses distinct E17578/E186ED/E17654 roles; F89450 is
// the SAME separate stable clone-empty role, not another literal/default.
//
// Genuine current-CRT enum/raw-string/pool domains, stable/disjoint coherent
// accessible storage, finite valid ownership/chains, closed ASCII/C-locale
// declaration keys, fitting names/results, nonthrowing successful services,
// live current Lua state and available tracking capacity are caller contracts.
// Keep all processes/generations/owners live; serialize shared mutations.
// Fixed display uses CURRENT-CRT sprintf("%f") after float32 rounding and
// double materialization; its52B accessible scratch/result-fit requirement and
// all private-CRT/numeric/locale/FP nonclaims carry through without new filters.
// Integer conversion samples actual fullDWORD0109EEA4!=0 AFTER lua_tonumber,
// then uses the accepted float32/SSE2/x87 conversion tail with a genuine bool.
// Unlike Original, its sample precedes that tail's float32 spill/reload: require
// a stable feature word, no FP traps/reentry/concurrent mutation. No bool alias,
// CPU guess, numeric cast/saturation or literal x87/read/FP-status parity.
// Only evidenced completed-object C++ cleanup is supplied; no enum/type/name
// rollback, failed-output cleanup, native FS/EH/Lua-longjmp/fault equivalence,
// Original ABI/class/privateCRT, app startup, whole parser or game/runtime credit.
} // namespace bsp
