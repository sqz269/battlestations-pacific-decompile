#pragma once
#include "bsp/native_enum_lua_declaration.hpp"
#include "bsp/game_native_lua_display_string.hpp"

namespace bsp::game {
class GameNativeStringProcess;
class GameNativePhysicalPoolProcess;
class GameNativeSettingsProcess;
class GameNativeReadOnlyData;
class GameNativeMutableCrtData;

// Concrete composition only. All services are genuine retained canonical
// owners; initialized E17578 and mapped D162D8/CE8ED0 spans already exist.
// Borrows the E188A8 CELL, not its current value. No world/Lua constructor,
// startup/publication, default or consumer-selected formatter is supplied.
class GameNativeEnumLuaDeclarationBindings final {
public:
    GameNativeEnumLuaDeclarationBindings(GameNativeStringProcess&,
        GameNativePhysicalPoolProcess&,GameNativeSettingsProcess&,
        GameNativeReadOnlyData&,const GameNativeMutableCrtData&);
    GameNativeEnumLuaDeclarationBindings(const GameNativeEnumLuaDeclarationBindings&)=delete;
    GameNativeEnumLuaDeclarationBindings& operator=(const GameNativeEnumLuaDeclarationBindings&)=delete;
    GameNativeEnumLuaDeclarationBindings(GameNativeEnumLuaDeclarationBindings&&)=delete;
    GameNativeEnumLuaDeclarationBindings& operator=(GameNativeEnumLuaDeclarationBindings&&)=delete;
    void declare(void* actual_raw_19c_enum,NativeString& consumed_key);
private:
    NativeEnumSymbolDeclarationContext declaration_;
    GameNativeLuaDisplayStringBindings display_;
    NativeEnumLuaDeclarationContext context_;
};
} // namespace bsp::game
