#include "bsp/native_game_settings_owner.hpp"
#include "bsp/native_string_vector.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/sound_lifetime_access.hpp"
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
void release_native_settings_input_008d4950(NativeInputSettingsLifetimeContext& c,NativeGameSettingsCalls& calls) {
    if(!c.publication_00e198e8)return;
    SoundLifetimeAccess lifetime(c.manager_publication_01090aa0);
    CapturedSoundLifetimeSection section(lifetime);
    if(c.publication_00e198e8) {
        auto manager=lifetime.get_manager_00415350();
        manager->unregister_object(c.publication_00e198e8);
        if(void* const current=c.publication_00e198e8)calls.delete_current_input(current,1,c);
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
        word(p,0x20,audio);word(p,0x28,audio);word(p,0x2c,audio);word(p,0x30,audio);
        const U video=c.video_bits_00ce7d20;
        byte(p,8,1);byte(p,9,0);word(p,0xc,2);byte(p,0x10,1);byte(p,0x4a,1);byte(p,0xb2,1);
        word(p,0x80,0);word(p,0x78,0);word(p,0x14,0x280);word(p,0x18,0x1e0);
        word(p,0x5c,0);word(p,0x58,0);byte(p,0x60,1);word(p,0x64,0);word(p,0x68,2);
        byte(p,0x1d,0);byte(p,0x1c,0);word(p,0x54,2);byte(p,0x84,1);byte(p,0x85,0);
        byte(p,0x6c,1);word(p,0x70,0);byte(p,0x74,1);byte(p,0x7c,1);byte(p,0x8c,1);
        byte(p,0x8d,1);word(p,0x90,1);byte(p,0x1e,1);word(p,0x34,video);
        byte(p,0x86,1);word(p,0x88,1);byte(p,0x42,0);byte(p,0x43,0);byte(p,0x41,1);
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
