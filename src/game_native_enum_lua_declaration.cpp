#include "bsp/game_native_enum_lua_declaration.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/game_native_settings_process.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_mutable_crt_data.hpp"

namespace bsp::game {
GameNativeEnumLuaDeclarationBindings::GameNativeEnumLuaDeclarationBindings(
    GameNativeStringProcess& strings,GameNativePhysicalPoolProcess& physical,
    GameNativeSettingsProcess& settings,GameNativeReadOnlyData& data,
    const GameNativeMutableCrtData& crt)
    : declaration_{strings.raw_context(),physical.enum_symbol_pool_storage_00e17578(),
        strings.enum_empty_00e186ed(),strings.query_empty_00e17654()},
      display_{strings},
      context_{declaration_,display_,settings.game_00e188a8(),crt.feature_word(),
        strings.clone_empty_00f89450(),data.data_at(0x00d162d8u,6),
        static_cast<const char*>(data.data_at(0x00ce8ed0u,5))} {}

void GameNativeEnumLuaDeclarationBindings::declare(void* owner,NativeString& consumed_key) {
    declare_native_enum_symbols_from_lua_008f2f70(owner,consumed_key,context_);
}
} // namespace bsp::game
