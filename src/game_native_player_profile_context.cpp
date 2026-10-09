#include "bsp/game_native_player_profile_context.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_settings_process.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_string_pool_storage.hpp"

namespace bsp::game {
GameNativePlayerProfileContext::GameNativePlayerProfileContext(
    const GameNativeVfsRawServices& vfs,GameNativeSettingsProcess& settings,
    const GameNativeReadOnlyData& data)
    :context_{vfs.strings,calls_,&settings.settings(),
        static_cast<const char*>(data.data_at(0x00ce3a0c,1)),
        static_cast<const char*>(data.data_at(0x00cef794,18)),
        static_cast<const char*>(data.data_at(0x00cef15c,5)),
        settings.profile_context()} {}

NativePlayerProfileContext& GameNativePlayerProfileContext::borrow_construction_context() noexcept {
    return context_;
}
} // namespace bsp::game
