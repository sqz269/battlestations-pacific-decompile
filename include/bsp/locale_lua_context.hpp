#pragma once

#include "bsp/locale_text_lookup.hpp"

struct lua_State;

namespace bsp {

// 00b68460's fixed type names; 00b68550 delegates here after lua_type for
// tracked objects (unbound object => -1; global pseudo-object => table).
const char* locale_lua_type_name_00b68460(int type) noexcept;

// Value conversion from00b69130. Does not call Lua tostring or __tostring.
// Reads the supplied live stack/pseudo-index without consuming it. Numbers
// use the corrected00b66a60 predicate, then the float32/int32 or %f path.
// Current Win32 CRT printf formatting is used; VS2005 nonfinite spelling and
// historical decimal tie behavior are not claimed byte-identical.
std::string locale_lua_value_string_00b69130(lua_State& state, int index);

// The caller selects the CURRENT owner from native00e1ae90 virtual+14 and
// supplies its live Lua state. This adapter borrows it, never closes it, and
// restores the prior stack top. Context selection is required, not guessed.
// Lua errors during __index become exceptions at a protected host boundary.
class LocaleLuaContext final : public LocaleTextRuntimeHost {
public:
    explicit LocaleLuaContext(lua_State& state) noexcept : state_(state) {}
    std::string context_string_00b692c0(const std::string& path) override;
    char16_t crt_uppercase_00c0391c(char16_t code_unit) override;

private:
    lua_State& state_;
};

} // namespace bsp
