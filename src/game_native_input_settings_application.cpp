#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "bsp/game_native_input_settings_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_lua_services.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp::game {
namespace {
template<class T> T read(const void* p,std::size_t offset) noexcept {
    T value;std::memcpy(&value,static_cast<const char*>(p)+offset,sizeof value);return value;
}
}
struct GameNativeInputSettingsApplication::Impl {
    enum class Phase {prepared,loading,ready,drained,failed};
    GameHostLog& log;GameSingletonHost& singletons;GameNativeLuaServices& lua;
    const bool sse2{IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)!=FALSE};
    const volatile float& one;
    const volatile float& base_replacement;
    const GameInputSettingsStackPolicy policy;
    GameInputSettingsRuntime runtime;
    Phase phase{Phase::prepared};void* owner{};
    Impl(GameHostLog& l,GameSingletonHost& s,GameNativeLuaServices& lua_services,
        GameNativeReadOnlyData& data,const GameNativeVfsRawServices& vfs,
        GameInputSettingsStackPolicy stack)
        :log(l),singletons(s),lua(lua_services),
        one(*static_cast<const volatile float*>(data.data_at(0xd7a24c,4))),
        base_replacement(*static_cast<const volatile float*>(data.data_at(0xcf7fe8,4))),
        policy(stack),runtime({s.input_settings_publication_00e198e8(),s.manager_publication_01090aa0(),
            vfs.strings,lua.bootstrap(),lua.binding().files(),sse2,one,base_replacement,policy}) {
        if(s.input_settings_publication_00e198e8()||s.native_deletion_bindings().input_settings)
            throw std::logic_error("application input settings already have an owner or binding");
        s.bind_input_settings(&runtime.context());
    }
};
GameNativeInputSettingsApplication::GameNativeInputSettingsApplication(GameHostLog& log,
    GameSingletonHost& s,GameNativeLuaServices& lua,GameNativeReadOnlyData& data,
    const GameNativeVfsRawServices& vfs,GameInputSettingsStackPolicy policy)
    :impl_(std::make_unique<Impl>(log,s,lua,data,vfs,policy)){}
GameNativeInputSettingsApplication::~GameNativeInputSettingsApplication()=default;
NativeInputSettingsLifetimeContext& GameNativeInputSettingsApplication::context() noexcept{return impl_->runtime.context();}
void GameNativeInputSettingsApplication::load_tables() {
    auto& p=*impl_;if(p.phase!=Impl::Phase::prepared)throw std::logic_error("application input settings cannot be loaded twice");
    p.phase=Impl::Phase::loading;
    try {
        NativeLuaServiceBindings::Activation activation(p.lua.binding());
        p.owner=p.runtime.get(); // 0073DA94 -> 005547D0 -> 006AB6B0/006A7BE0
        auto* const before=read<lua_State*>(p.owner,0x7c);
        load_native_input_settings_tables_006a7be0(p.owner,p.runtime.context().tables); // 0073DA9B
        if(p.owner!=p.singletons.input_settings_publication_00e198e8()||
            before!=read<lua_State*>(p.owner,0x7c))
            throw std::logic_error("input table guard changed the published owner or interpreter");
        p.phase=Impl::Phase::ready;
        const auto result=summary();
        p.log.notef("native input settings: storage=raw540h publication=same manager=shared strings=actual_pool "
            "devices=%u input_names=%u controller_names=%u presets_lua=%p tables_started=%d runtime_settings_loaded=%d guarded_reload=same",
            result.devices,result.input_names,result.controller_names,result.presets_lua,
            result.tables_started?1:0,result.runtime_settings_loaded?1:0);
        p.log.notef("native input stack policy: descriptor=%08x preset=%08x opaque=%08x sensitivity=%08x "
            "source_preimages=explicit native_stack_identity=unproven sse2=%d",
            p.policy.descriptor_flag_stack_preimage,p.policy.descriptor_flag_stack_preimage,
            p.policy.vector_opaque_stack_preimage,p.policy.sensitivity_default_stack_preimage,p.sse2?1:0);
    }catch(...){p.phase=Impl::Phase::failed;throw;}
}
void* GameNativeInputSettingsApplication::actual_settings() const {
    const auto& p=*impl_;if(p.phase!=Impl::Phase::ready||p.owner!=p.singletons.input_settings_publication_00e198e8())
        throw std::logic_error("application input settings are not live");
    return p.owner;
}
GameNativeInputSettingsSummary GameNativeInputSettingsApplication::summary() const {
    const void* const p=actual_settings();
    return {read<std::uint32_t>(p,0x10),read<std::uint32_t>(p,0x2c),read<std::uint32_t>(p,0x68),
        read<std::uint8_t>(p,4)!=0,read<std::uint8_t>(p,5)!=0,read<lua_State*>(p,0x7c)};
}
bool GameNativeInputSettingsApplication::requires_process_retention() const noexcept {
    return impl_->phase==Impl::Phase::loading||impl_->phase==Impl::Phase::failed;
}
void GameNativeInputSettingsApplication::after_singleton_drain() {
    auto& p=*impl_;if(p.phase==Impl::Phase::drained)return;
    if(requires_process_retention()||p.singletons.input_settings_publication_00e198e8())
        throw std::logic_error("native input settings survived or interrupted the shared drain");
    p.singletons.bind_input_settings(nullptr);p.owner=nullptr;p.phase=Impl::Phase::drained;
    p.log.note("native input settings after raw drain: publication=null binding=retired persistent_lua=closed");
}
} // namespace bsp::game
