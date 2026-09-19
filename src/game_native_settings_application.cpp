#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "bsp/game_native_settings_application.hpp"
#include "bsp/game_native_settings_process.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/vfs_locale_runtime.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp::game {
namespace {
using U=std::uint32_t;
template<class T> T read(const void* p,std::size_t offset){T value;std::memcpy(&value,static_cast<const char*>(p)+offset,sizeof value);return value;}
bool flag(const void* p,std::size_t offset){return read<std::uint8_t>(p,offset)!=0;}
std::string raw_text(const void* header){
    const auto* p=read<const char*>(header,4);return p?std::string(p,read<U>(header,0)):std::string();
}
std::int32_t __stdcall decrement(volatile std::int32_t* value){return InterlockedDecrement(reinterpret_cast<volatile LONG*>(value));}
struct TextCalls final:NativeSettingsTextCalls {
    const std::string personal_root;std::string options_path;bool options_present{};
    explicit TextCalls(std::string root):personal_root(std::move(root)){
        if(personal_root.size()>=260)throw std::invalid_argument("settings personal root does not fit native MAX_PATH buffer");
    }
    int personal_folder_00ce22f0(char* buffer) override {
        if(personal_root.empty())return NativeSettingsTextCalls::personal_folder_00ce22f0(buffer);
        std::memcpy(buffer,personal_root.c_str(),personal_root.size()+1);return 1;
    }
    void* open_00bf838e(const char* path,const char* mode) override {
        void* const result=NativeSettingsTextCalls::open_00bf838e(path,mode);
        if(mode[0]=='r'){options_path=path;options_present=result!=nullptr;}
        return result;
    }
};
}
void copy_native_game_settings_read_view(const NativeGameSettingsStorage& storage,
    const std::vector<LanguageEntry>& languages,GameSettingsBlock& view){
    const void* p=&storage;auto& o=view.options_file;auto& g=view.gameplay;auto& a=view.audio;
    auto& c=view.control;auto& v=view.presentation;
    o.width_14=read<int>(p,0x14);o.height_18=read<int>(p,0x18);
    o.no_lod_1c=flag(p,0x1c);o.hires_shadow_1d=flag(p,0x1d);o.fullscreen_1e=flag(p,0x1e);
    o.object_detail_54=read<int>(p,0x54);o.antialias_58=read<int>(p,0x58);o.antialias_index_5c=read<int>(p,0x5c);
    o.vsync_60=flag(p,0x60);o.texture_detail_68=read<int>(p,0x68);o.reflection_6c=flag(p,0x6c);
    o.clouds_74=flag(p,0x74);o.resolution_index_78=read<int>(p,0x78);o.shadow_84=flag(p,0x84);
    o.shadow_85=flag(p,0x85);o.foliage_86=flag(p,0x86);o.shader_model_88=read<int>(p,0x88);o.firewall_94=flag(p,0x94);
    g.language_index_04=read<int>(p,4);o.language=language_name_008d4870(languages,static_cast<std::size_t>(g.language_index_04));
    o.unknown_tokens.clear();g.imperial_08=flag(p,8);g.subtitle_09=flag(p,9);g.hints_0c=read<int>(p,0xc);
    g.camera_shake_10=flag(p,0x10);g.target_indicator_4a=flag(p,0x4a);g.water_drops_7c=flag(p,0x7c);
    g.marker_alpha_80=read<float>(p,0x80);g.cockpit_mode_b2=flag(p,0xb2);g.show_safe_area_b1=flag(p,0xb1);
    g.clan_text_b4=raw_text(storage.bytes+0xb4);
    a.master_20=read<float>(p,0x20);a.enabled_24=flag(p,0x24);a.music_28=read<float>(p,0x28);
    a.effects_2c=read<float>(p,0x2c);a.speech_30=read<float>(p,0x30);
    c.rumble_off_40=flag(p,0x40);c.swap_sticks_41=flag(p,0x41);c.invert_camera_y_42=flag(p,0x42);
    c.invert_plane_y_43=flag(p,0x43);c.swap_map_sticks_44=flag(p,0x44);c.xbox_compatibility_b0=flag(p,0xb0);
    v.gamma_64=read<float>(p,0x64);v.unknown_34=read<float>(p,0x34);v.unknown_70=read<int>(p,0x70);
    v.shader_flag_8c=read<std::uint8_t>(p,0x8c);v.motion_blur_8d=flag(p,0x8d);v.old_film_effect_90=read<int>(p,0x90);
    v.hardware_reported_95=flag(p,0x95);view.downloaded_content.clear();
    const auto* entries=read<const char*>(p,0x98);const auto count=read<int>(p,0x9c);
    for(int i=0;i<count;++i)view.downloaded_content.push_back(raw_text(entries+static_cast<std::size_t>(i)*8));
}
struct GameNativeSettingsApplication::Impl {
    enum class Phase{prepared,loading,ready,failed};
    GameHostLog& log;GameNativeSettingsProcess& process;GameNativeRendererApplication& renderer;
    GameNativeReadOnlyData& data;GameNativeVfsRawServices vfs;NativeStringRawPoolContext& raw;
    const GameNativeSettingsStackPolicy policy;Phase phase{Phase::prepared};
    NativeLanguageResourceEnumerationCalls enumeration_calls;NativeLanguageCatalogProducerCalls producer_calls;
    NativeStreamTextScannerVfsCalls scanner_calls;NativeStreamTextScannerContext scanner;
    NativeLanguageCatalogProducerContext catalog;
    TextCalls text_calls;NativeSettingsPathContext path;NativeSettingsTextContext text;
    NativeSettingsChoiceCalls choices;NativeSettingsLanguageContext language;
    NativeSceneTokenizerCalls token_calls;NativeSceneTokenizerContext tokenizer;
    NativeSceneTokenValueCalls value_calls;NativeSceneTokenValueContext values;
    NativeSettingsLoaderCalls loader_calls;NativeSettingsDecrement const decrement_import=&decrement;
    NativeSettingsLoaderContext loader;NativeSettingsLoaderOperation operation;
    VfsLocaleRuntime locale;std::vector<LanguageEntry> languages;
    const char* str(std::uintptr_t a){return static_cast<const char*>(data.data_at(a,1));}
    Impl(GameHostLog& log_in,GameNativeSettingsProcess& owner,GameSingletonHost& singletons,
        GameVfsHost& files,GameNativeRendererApplication& graphics,GameNativeReadOnlyData& image,
        const std::vector<std::string>& suffixes,std::string personal_root,const GameNativeSettingsStackPolicy& stack)
        :log(log_in),process(owner),renderer(graphics),data(image),vfs(files.borrow_raw_services()),
        raw(files.raw_strings()),policy(stack),scanner_calls(vfs.bindings),
        scanner{raw,scanner_calls,vfs.actual_vfs_publication_0109ceec,owner.scanner_whitespace(),owner.scanner_delimiters(),str(0xd15f34)},
        catalog{owner.catalog_header(),raw,vfs.strings,vfs.invalid_parameters,vfs.enumeration,enumeration_calls,
            scanner,owner.hints(),owner.catalog_calls(),producer_calls,str(0xd15e10),str(0xd15e08),
            {str(0xd15df4),str(0xd15de0),str(0xd15dcc),str(0xd15db8),str(0xd15da4),str(0xd15d88)},
            {str(0xd15d80),str(0xd15d74),str(0xd15d68),str(0xd15d5c)},policy.scanner},
        text_calls(std::move(personal_root)),path{raw,text_calls,str(0xd15af0),str(0xd15ae0),owner.empty_00f88a3c()},
        text{path,vfs.strings,owner.catalog_data(),
            {str(0xd15b3c),str(0xd15b48),str(0xd15b54),str(0xd15b60),str(0xd15b68),str(0xd15b78),str(0xd15b84),str(0xd15b8c),str(0xd15b98),str(0xd15ba0),str(0xd15bac),str(0xd15bbc),str(0xd15bcc),str(0xd15bdc),str(0xd15be8)},
            str(0xce4390),str(0xce3a90),str(0xd15b38),owner.null_integer_format_01090ab4(),policy.personal_buffer},
        language{raw,choices,owner.catalog_data(),owner.catalog_count()},
        tokenizer{raw,vfs.retained_memory,token_calls,owner.scene_whitespace(),owner.scene_delimiters(),str(0xd15fe8)},
        values{tokenizer,value_calls,str(0xce3a34),str(0xd15f34)},
        loader{catalog,text,language,values,choices,loader_calls,graphics.publication_00f8d394(),
            owner.resolution_header(),owner.antialias_header(),static_cast<const volatile U*>(image.data_at(0xd5f0a8,0x108)),
            decrement_import,policy.loader,
            {str(0xd15f1c),str(0xd15f10),str(0xd15f04),str(0xd15efc),str(0xcf3a78),str(0xd15ef4),str(0xd15ee8),str(0xd15edc),str(0xd15ed4),str(0xd15ecc),str(0xd15ec4),str(0xd15eb8),str(0xd15ea8),str(0xd15e98),str(0xd15e88),str(0xd15e7c)},
            {str(0xd15b14),str(0xce4308),str(0xce431c),str(0xce432c),str(0xce4324)},
            str(0xd15f28),str(0xd15e60),str(0xd15e4c),str(0xd15e24),str(0xd15e18)},
        operation(policy.loader),locale(files.context(),files.search_registrations(),suffixes,[&owner]{
            NativeProfileHintsOwnerOperation op;return read<U>(get_native_profile_hints_owner_004c1e90(owner.hints(),op),8);}) {
        auto*& binding=singletons.native_deletion_bindings().native_profile_hints;
        if(binding&&binding!=&owner.hints())throw std::logic_error("application already has a different native hints lifetime");
        binding=&owner.hints();
    }
};
GameNativeSettingsApplication::GameNativeSettingsApplication(GameHostLog& log,GameNativeSettingsProcess& owner,
    GameSingletonHost& singletons,GameVfsHost& vfs,GameNativeRendererApplication& renderer,
    GameNativeReadOnlyData& data,const std::vector<std::string>& suffixes,std::string personal_root,
    const GameNativeSettingsStackPolicy& policy)
    :impl_(std::make_unique<Impl>(log,owner,singletons,vfs,renderer,data,suffixes,std::move(personal_root),policy)){}
GameNativeSettingsApplication::~GameNativeSettingsApplication()=default;
void GameNativeSettingsApplication::load(){
    auto& p=*impl_;if(p.phase!=Impl::Phase::prepared)throw std::logic_error("native application settings loader cannot replay");
    p.phase=Impl::Phase::loading;
    try {
        load_native_game_settings_008d8190(&p.process.settings(),p.loader,p.operation);
        const auto count=static_cast<std::int32_t>(p.process.catalog_count());
        const auto* rows=static_cast<const char*>(p.process.catalog_data());
        for(std::int32_t i=0;i<count;++i){const auto* row=rows+static_cast<std::size_t>(i)*32;
            p.languages.push_back({raw_text(row),raw_text(row+8),raw_text(row+16),raw_text(row+24)});
        }
        p.phase=Impl::Phase::ready;
        p.log.notef("native settings: storage=rawBCh catalog=raw0Ch entries=%u resolutions=%u antialias=%u loader=008D8190 read_view=copy CRT=registered",
            p.process.catalog_count(),read<U>(p.process.resolution_header(),4),read<U>(p.process.antialias_header(),4));
        p.log.note("native settings stack policy: tokenizer/rectangle/registry/key/type/list/scanner/folder preimages=zero native_stack_identity=unproven");
    }catch(...){p.phase=Impl::Phase::failed;throw;}
}
bool GameNativeSettingsApplication::requires_process_retention() const noexcept{return impl_->phase==Impl::Phase::loading||impl_->phase==Impl::Phase::failed;}
std::uint32_t GameNativeSettingsApplication::failure_site() const noexcept{return impl_->operation.native_site;}
void GameNativeSettingsApplication::copy_read_view(GameSettingsBlock& view) const{
    if(impl_->phase!=Impl::Phase::ready)throw std::logic_error("native settings read view requires completed load");
    copy_native_game_settings_read_view(impl_->process.settings(),impl_->languages,view);
}
const std::vector<LanguageEntry>& GameNativeSettingsApplication::language_catalog() const noexcept{return impl_->languages;}
VfsLocaleRuntime& GameNativeSettingsApplication::locale_source() noexcept{return impl_->locale;}
IDirect3D9& GameNativeSettingsApplication::renderer_api() const{return impl_->renderer.api();}
const std::string& GameNativeSettingsApplication::options_path() const noexcept{return impl_->text_calls.options_path;}
bool GameNativeSettingsApplication::options_file_present() const noexcept{return impl_->text_calls.options_present;}
} // namespace bsp::game
