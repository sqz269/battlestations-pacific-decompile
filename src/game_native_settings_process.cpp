#include "bsp/game_native_settings_process.hpp"
#include "bsp/game_native_input_settings_process.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_language_catalog_lifetime.hpp"
#include "bsp/native_settings_vector_lifetime.hpp"
#include "bsp/xlive_library.hpp"
#include <cstdlib>
#include <cstdio>
#include <stdexcept>

namespace bsp::game {
namespace {
GameNativeSettingsProcess* process;
struct ProfileSdk final:NativeProfileSettingsSdkCalls {
    const XLiveLibrary* library{};
    std::uint32_t read_profile(std::uint32_t title,std::uint32_t user,std::uint32_t count,
        std::uint32_t* ids,std::uint32_t* bytes,void* result,void* overlapped) override {
        if(!library)throw std::logic_error("native settings profile import has no live XLive library");
        NativeProfileSettingsSdkRuntime runtime(*library);
        return runtime.read_profile(title,user,count,ids,bytes,result,overlapped);
    }
};
struct RawVector {
    void* volatile data{};volatile std::uint32_t count{};std::uint32_t capacity{};
};
static_assert(sizeof(RawVector)==12);
[[noreturn]] void crt_failed(const char* operation) noexcept {
    std::fprintf(stderr,"bsp_game: native settings CRT %s interrupted; retaining partial ownership\n",operation);
    std::fflush(stderr);std::_Exit(1);
}
}
struct GameNativeSettingsProcess::Impl {
    enum class Phase{fresh,initializing,ready,failed};
    GameNativeReadOnlyData& data;Phase phase{Phase::fresh};
    NativeGameSettingsStorage settings{};RawVector resolutions,antialias,catalog;
    NativeOnlineManagerStorage* volatile online{};void* volatile game{};
    ProfileSdk sdk;NativeProfileSettingsContext profile{online,game,sdk};
    NativeGameSettingsCalls settings_calls;NativeSettingsVectorLifetimeCalls vector_calls;
    NativeLanguageCatalogAllocationCalls catalog_allocation;NativeLanguageCatalogRegistrationCalls catalog_registration;
    NativeGameSettingsContext settings_context;
    NativeGameSettingsOperation settings_startup,settings_shutdown;
    NativeLanguageCatalogLifetimeOperation catalog_shutdown;
    NativeProfileHintsOwnerCalls hints_calls;void* volatile hints_publication{};
    NativeProfileHintsOwnerContext hints;
    const char* volatile scene_space;const char* volatile scene_separators;
    const char* volatile scanner_space;const char* volatile scanner_separators;
    const char empty{},null_integer_format{};
    std::array<int,4> registration{};
    const char* text(std::uintptr_t address){return static_cast<const char*>(data.data_at(address,1));}
    explicit Impl(GameNativeReadOnlyData& d):data(d),
        settings_context{game_native_string_process().strings(),game_native_string_process().raw_context(),profile,
            game_native_input_settings_process().settings_reference(),settings_calls,
            *static_cast<const volatile std::uint32_t*>(d.data_at(0xce3800,4)),
            *static_cast<const volatile std::uint32_t*>(d.data_at(0xce7d20,4)),text(0xce3a0c)},
        hints{game_native_string_process().raw_context(),hints_calls,hints_publication,text(0xce3a38)},
        scene_space(text(0xd15f2c)),scene_separators(text(0xce5698)),
        scanner_space(text(0xd15f2c)),scanner_separators(text(0xce5698)){}
};
GameNativeSettingsProcess::GameNativeSettingsProcess(GameNativeReadOnlyData& data):impl_(std::make_unique<Impl>(data)){}
GameNativeSettingsProcess::~GameNativeSettingsProcess()=default;
GameNativeSettingsProcess& game_native_settings_process(GameNativeReadOnlyData& data){
    if(!process)process=new GameNativeSettingsProcess(data);
    if(&process->impl_->data!=&data)throw std::logic_error("native settings process cannot change its retained data mapping");
    return *process;
}
std::array<int,4> GameNativeSettingsProcess::initialize_once(){
    auto& p=*impl_;if(p.phase==Impl::Phase::ready)return p.registration;
    if(p.phase!=Impl::Phase::fresh)throw std::logic_error("native settings CRT initialization cannot replay");
    p.phase=Impl::Phase::initializing;
    try {
        p.registration[0]=register_native_settings_resolutions_00cd2d60(p.vector_calls,&shutdown_resolutions);
        p.registration[1]=register_native_settings_antialias_00cd2d70(p.vector_calls,&shutdown_antialias);
        p.registration[2]=initialize_native_game_settings_static_00cd2d80(p.settings,p.settings_context,p.settings_startup,&shutdown_settings);
        p.registration[3]=register_native_language_catalog_shutdown_00cd2da0(p.catalog_registration,&shutdown_catalog);
        p.phase=Impl::Phase::ready;return p.registration;
    }catch(...){p.phase=Impl::Phase::failed;throw;}
}
NativeGameSettingsStorage& GameNativeSettingsProcess::settings(){
    if(impl_->phase!=Impl::Phase::ready)throw std::logic_error("native settings static owner is not initialized");
    return impl_->settings;
}
void* GameNativeSettingsProcess::resolution_header() noexcept{return &impl_->resolutions;}
void* GameNativeSettingsProcess::antialias_header() noexcept{return &impl_->antialias;}
void* GameNativeSettingsProcess::catalog_header() noexcept{return &impl_->catalog;}
void* const volatile& GameNativeSettingsProcess::catalog_data() noexcept{return impl_->catalog.data;}
const volatile std::uint32_t& GameNativeSettingsProcess::catalog_count() noexcept{return impl_->catalog.count;}
NativeLanguageCatalogAllocationCalls& GameNativeSettingsProcess::catalog_calls() noexcept{return impl_->catalog_allocation;}
NativeProfileHintsOwnerContext& GameNativeSettingsProcess::hints() noexcept{return impl_->hints;}
NativeOnlineManagerStorage* volatile& GameNativeSettingsProcess::online_00f8abe8() noexcept{return impl_->online;}
void* volatile& GameNativeSettingsProcess::game_00e188a8() noexcept{return impl_->game;}
NativeProfileSettingsContext& GameNativeSettingsProcess::profile_context() noexcept{return impl_->profile;}
void GameNativeSettingsProcess::bind_profile_sdk(const XLiveLibrary* library){
    if(library&&impl_->sdk.library&&impl_->sdk.library!=library)throw std::logic_error("native settings already borrow another XLive library");
    if(!library&&impl_->online)throw std::logic_error("XLive settings binding must survive the online owner");
    impl_->sdk.library=library;
}
const char* const volatile& GameNativeSettingsProcess::scene_whitespace() noexcept{return impl_->scene_space;}
const char* const volatile& GameNativeSettingsProcess::scene_delimiters() noexcept{return impl_->scene_separators;}
const char* const volatile& GameNativeSettingsProcess::scanner_whitespace() noexcept{return impl_->scanner_space;}
const char* const volatile& GameNativeSettingsProcess::scanner_delimiters() noexcept{return impl_->scanner_separators;}
const char* GameNativeSettingsProcess::empty_00f88a3c() const noexcept{return &impl_->empty;}
const char* GameNativeSettingsProcess::null_integer_format_01090ab4() const noexcept{return &impl_->null_integer_format;}
void GameNativeSettingsProcess::shutdown_resolutions(){try{auto& p=*process->impl_;shutdown_native_settings_resolutions_00cdee40(&p.resolutions,p.vector_calls);}catch(...){crt_failed("resolutions");}}
void GameNativeSettingsProcess::shutdown_antialias(){try{auto& p=*process->impl_;shutdown_native_settings_antialias_00cdee80(&p.antialias,p.vector_calls);}catch(...){crt_failed("antialias");}}
void GameNativeSettingsProcess::shutdown_settings(){try{auto& p=*process->impl_;destroy_native_game_settings_static_00cdeec0(p.settings,p.settings_context,p.settings_shutdown);}catch(...){crt_failed("settings");}}
void GameNativeSettingsProcess::shutdown_catalog(){try{auto& p=*process->impl_;shutdown_native_language_catalog_00cdeea0(&p.catalog,p.settings_context.raw_strings,p.catalog_allocation,p.catalog_shutdown);}catch(...){crt_failed("catalog");}}
} // namespace bsp::game
