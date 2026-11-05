#include "bsp/native_game_settings_owner.hpp"
#include "bsp/native_string_vector.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/sound_lifetime_access.hpp"
#include "bsp/game_settings.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native game settings require MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
using Op=NativeGameSettingsOperation;
void* at(void* p,U offset) noexcept {return reinterpret_cast<void*>(reinterpret_cast<U>(p)+offset);}
template<class T=U> T get(void* p,U offset=0) noexcept {T value;std::memcpy(&value,at(p,offset),sizeof value);return value;}
void word(void* p,U offset,U value) noexcept {std::memcpy(at(p,offset),&value,4);}
void byte(void* p,U offset,std::uint8_t value) noexcept {*static_cast<std::uint8_t*>(at(p,offset))=value;}
void float_word(void* p,U offset,const float& value) noexcept {
    std::memcpy(at(p,offset),&value,4);
}
// The real 00F88980 settings owner remains authoritative. These temporary
// projections carry only reset outputs back to its native sparse store set;
// they do not replace the owner, its clan string, or its opaque bytes.
void constructor_audio_defaults(void* p,U image_bits) noexcept {
    GameSettingsBlock values;
    reset_audio_defaults_008d41f0(values,image_bits);
    float_word(p,0x20,values.audio.master_20);
    float_word(p,0x28,values.audio.music_28);
    float_word(p,0x2c,values.audio.effects_2c);
    float_word(p,0x30,values.audio.speech_30);
}
void constructor_video_defaults(void* p,U image_bits) noexcept {
    GameSettingsBlock values;
    reset_video_defaults_008d4520(values,true,false,image_bits);
    const auto& v=values.options_file;
    const auto& x=values.presentation;
    word(p,0x78,v.resolution_index_78);word(p,0x14,v.width_14);word(p,0x18,v.height_18);
    word(p,0x5c,v.antialias_index_5c);word(p,0x58,v.antialias_58);byte(p,0x60,v.vsync_60);
    float_word(p,0x64,x.gamma_64);word(p,0x68,v.texture_detail_68);
    byte(p,0x1d,v.hires_shadow_1d);byte(p,0x1c,v.no_lod_1c);word(p,0x54,v.object_detail_54);
    byte(p,0x84,v.shadow_84);byte(p,0x85,v.shadow_85);byte(p,0x6c,v.reflection_6c);
    word(p,0x70,x.unknown_70);byte(p,0x74,v.clouds_74);byte(p,0x7c,values.gameplay.water_drops_7c);
    byte(p,0x8c,x.shader_flag_8c);byte(p,0x8d,x.motion_blur_8d);word(p,0x90,x.old_film_effect_90);
    byte(p,0x1e,v.fullscreen_1e);float_word(p,0x34,x.unknown_34);
    byte(p,0x86,v.foliage_86);word(p,0x88,v.shader_model_88);
}
void begin(void* p,NativeGameSettingsContext& c,Op& op) {
    if(op.phase!=Op::Phase::fresh)throw std::logic_error("native settings operation cannot replay");
    op.owner=p;op.context=&c;op.phase=Op::Phase::running;
}
void destroy_body(void* p,NativeGameSettingsContext& c,Op& op) {
    word(p,0,0x00d15d58);op.unwind_state=2;op.native_site=0x008d78fc;
    release_native_settings_input_008d4950(c.input,c.calls);
    op.unwind_state=1;op.native_site=0x008d791d;
    destroy_native_string_header_0041dd20(at(p,0xb4),c.raw_strings);
    op.unwind_state=0;op.native_site=0x008d7938;
    resize_native_string_vector_00427110(*static_cast<NativeStringVectorStorage*>(at(p,0xa4)),0,c.strings);
    op.native_site=0x008d7940;c.calls.free_00bf6989(get<void*>(p,0xa4));
    op.unwind_state=-1;op.native_site=0x008d795a;
    resize_native_string_vector_00427110(*static_cast<NativeStringVectorStorage*>(at(p,0x98)),0,c.strings);
    op.native_site=0x008d7962;c.calls.free_00bf6989(get<void*>(p,0x98));
}
}
U NativeGameSettingsCalls::online_state_00a3e500(NativeOnlineManagerStorage* p){return get(p,0x28);}
std::uint8_t NativeGameSettingsCalls::selected_user_00a3e510(NativeOnlineManagerStorage* p){return get<std::uint8_t>(p,0x119);}
void NativeGameSettingsCalls::import_profile_008d45d0(void* p,NativeProfileSettingsContext& c){import_native_profile_control_settings_008d45d0(p,c,*this);}
void NativeGameSettingsCalls::delete_current_input(void* p,U flags,NativeInputSettingsLifetimeContext& c) {
    switch(get(p)) {
    case 0x00cf81cc:(void)scalar_delete_native_input_settings_006ab800(p,flags,c);return;
    case 0x00ce3818:(void)delete_native_singleton_base_00412440(p,nullptr,static_cast<std::uint8_t>(flags));return;
    default:throw std::logic_error("native settings input has an unsupported current scalar profile");
    }
}
int NativeGameSettingsCalls::register_shutdown_00bf6ff5(void (*shutdown)()){return std::atexit(shutdown);}
void NativeGameSettingsCalls::free_00bf6989(void* p){singleton_lifetime_free(p);}
Op::~NativeGameSettingsOperation(){if(phase==Phase::running||phase==Phase::failed)std::terminate();}
void Op::acknowledge_diagnostic_cleanup() noexcept {phase=Phase::diagnostic_retired;}
NativeGameSettingsInputReference::NativeGameSettingsInputReference(NativeInputSettingsLifetimeContext& c) noexcept
    :publication_00e198e8(c.publication_00e198e8),manager_publication_01090aa0(c.manager_publication_01090aa0),fixed_(&c){}
NativeGameSettingsInputReference::NativeGameSettingsInputReference(void* volatile& p,void* volatile& m,
    NativeInputSettingsLifetimeContext*& live) noexcept
    :publication_00e198e8(p),manager_publication_01090aa0(m),live_(&live){}
NativeInputSettingsLifetimeContext& NativeGameSettingsInputReference::require_context() const {
    auto* const context=live_?*live_:fixed_;
    if(!context||&context->publication_00e198e8!=&publication_00e198e8||
        &context->manager_publication_01090aa0!=&manager_publication_01090aa0)
        throw std::logic_error("native settings input has no matching live source context");
    return *context;
}
void release_native_settings_input_008d4950(NativeInputSettingsLifetimeContext& c,NativeGameSettingsCalls& calls) {
    release_native_settings_input_008d4950(NativeGameSettingsInputReference(c),calls);
}
void release_native_settings_input_008d4950(const NativeGameSettingsInputReference& c,NativeGameSettingsCalls& calls) {
    if(!c.publication_00e198e8)return;
    SoundLifetimeAccess lifetime(c.manager_publication_01090aa0);
    CapturedSoundLifetimeSection section(lifetime);
    if(c.publication_00e198e8) {
        auto manager=lifetime.get_manager_00415350();
        manager->unregister_object(c.publication_00e198e8);
        if(void* const current=c.publication_00e198e8)calls.delete_current_input(current,1,c.require_context());
        c.publication_00e198e8=nullptr;
    }
}
void* construct_native_game_settings_008d7710(void* p,NativeGameSettingsContext& c,Op& op) {
    begin(p,c,op);
    try {
        word(p,0,0x00d15d58);word(p,4,0);byte(p,0x24,1);byte(p,0x38,0);word(p,0x3c,1);
        byte(p,0x49,0);byte(p,0x4a,1);byte(p,0x4b,0);byte(p,0x4c,0);word(p,0x50,2);
        byte(p,0x94,0);byte(p,0x95,0);word(p,0x98,0);word(p,0x9c,0);word(p,0xa0,0);
        op.unwind_state=0;word(p,0xa4,0);word(p,0xa8,0);word(p,0xac,0);
        op.unwind_state=1;byte(p,0xb0,0);byte(p,0xb1,0);byte(p,0xb2,1);op.native_site=0x008d77b4;
        construct_native_string_header_0041e870(at(p,0xb4),c.raw_strings,c.empty_00ce3a0c);
        const U audio=c.audio_bits_00ce3800;
        // bsp.py show 008D7710 --asm: 008D77C4..008D77D3 inlines 008D41F0's
        // four audio stores. Call its existing body with the captured cell.
        constructor_audio_defaults(p,audio);
        const U video=c.video_bits_00ce7d20;
        byte(p,8,1);byte(p,9,0);word(p,0xc,2);byte(p,0x10,1);byte(p,0x4a,1);byte(p,0xb2,1);
        // 008D7801..008D7871 is 008D4520's complete store set with
        // reset_resolution=true, keep_fullscreen=false. Keep +80h's
        // gameplay store separate; no whole-owner copy or second run.
        word(p,0x80,0);
        constructor_video_defaults(p,video);
        byte(p,0x42,0);byte(p,0x43,0);byte(p,0x41,1);
        byte(p,0x40,0);byte(p,0x44,1);
        auto* const manager=c.profile.current_manager_00f8abe8;op.unwind_state=2;
        if(manager) {
            op.native_site=0x008d789b;
            if(c.calls.online_state_00a3e500(manager)==2) {
                op.native_site=0x008d78aa;
                if(c.calls.selected_user_00a3e510(c.profile.current_manager_00f8abe8)!=0) {
                    op.native_site=0x008d78b5;c.calls.import_profile_008d45d0(p,c.profile);
                }
            }
        }
        op.phase=Op::Phase::complete;return p;
    }catch(...){op.phase=Op::Phase::failed;throw;}
}
void destroy_native_game_settings_008d78d0(void* p,NativeGameSettingsContext& c,Op& op) {
    begin(p,c,op);try{destroy_body(p,c,op);op.phase=Op::Phase::complete;}
    catch(...){op.phase=Op::Phase::failed;throw;}
}
void* delete_native_game_settings_008d7980(void* p,U flags,NativeGameSettingsContext& c,Op& op) {
    begin(p,c,op);
    try{destroy_body(p,c,op);if(flags&1u){op.native_site=0x008d7990;c.calls.free_00bf65ac(p);}op.phase=Op::Phase::complete;return p;}
    catch(...){op.phase=Op::Phase::failed;throw;}
}
int initialize_native_game_settings_static_00cd2d80(NativeGameSettingsStorage& p,NativeGameSettingsContext& c,Op& op,void (*shutdown)()) {
    construct_native_game_settings_008d7710(&p,c,op);op.native_site=0x00cd2d8f;
    try{return c.calls.register_shutdown_00bf6ff5(shutdown);}
    catch(...){op.phase=Op::Phase::failed;throw;}
}
void destroy_native_game_settings_static_00cdeec0(NativeGameSettingsStorage& p,NativeGameSettingsContext& c,Op& op) {
    destroy_native_game_settings_008d78d0(&p,c,op);
}
} // namespace bsp
