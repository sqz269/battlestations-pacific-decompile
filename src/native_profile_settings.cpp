#include "bsp/native_profile_settings.hpp"
#include "bsp/xlive_library.hpp"
#include <cstring>
#include <stdexcept>
#include <windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native profile settings require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4);
void* at(void* p,Word n) noexcept {return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+n);}
template<class T>T read(void* p,Word n=0) noexcept {return *static_cast<volatile T*>(at(p,n));}
template<class T>void put(void* p,Word n,T value) noexcept {*static_cast<volatile T*>(at(p,n))=value;}
NativeReadProfileSettings resolve(const XLiveLibrary& library){
    auto module=static_cast<HMODULE>(library.module_handle());
    if(!module)throw std::invalid_argument("Profile settings need loaded XLive");
    const FARPROC address=GetProcAddress(module,MAKEINTRESOURCEA(5331));
    if(!address)throw std::runtime_error("Missing XLive profile-settings ordinal 5331");
    NativeReadProfileSettings result;static_assert(sizeof(result)==sizeof(address));
    std::memcpy(&result,&address,sizeof result);return result;
}
} // namespace
NativeProfileSettingsSdkRuntime::NativeProfileSettingsSdkRuntime(const XLiveLibrary& library)
    :NativeProfileSettingsSdkRuntime(resolve(library)){}
NativeProfileSettingsSdkRuntime::NativeProfileSettingsSdkRuntime(NativeReadProfileSettings read)
    :read_(read){if(!read_)throw std::invalid_argument("Null profile-settings import");}
std::uint32_t NativeProfileSettingsSdkRuntime::read_profile(Word title,Word user,Word count,
    Word* ids,Word* bytes,void* result,void* overlapped){
    return read_(title,user,count,ids,bytes,result,overlapped);
}
void reset_native_profile_game_defaults_008d41c0(void* settings) noexcept {
    put<std::uint8_t>(settings,8,1);put<std::uint8_t>(settings,9,0);
    put<Word>(settings,0xc,2);put<std::uint8_t>(settings,0x10,1);
    put<std::uint8_t>(settings,0x4a,1);put<std::uint8_t>(settings,0xb2,1);
    put<std::uint8_t>(settings,0x7c,1);put<Word>(settings,0x80,0);
}
void import_native_profile_control_settings_008d45d0(void* settings,
    NativeProfileSettingsContext& context,NativeProfileCollectionCalls& memory){
    Word ids[4]={0x10040015u,0x10040024u,0x10040002u,0x10040003u};Word bytes=0;
    (void)context.sdk.read_profile(0,0,4,ids,&bytes,nullptr,nullptr);
    void* const result=memory.allocate_00bf55be(bytes);
    const Word user=read<Word>(context.current_manager_00f8abe8,0x11c);
    if(context.sdk.read_profile(0,user,4,ids,&bytes,result,nullptr)!=0)return;
    const Word difficulty=read<Word>(read<void*>(result,4),0x20);
    const Word mapped=difficulty==2?2u:static_cast<Word>(difficulty!=1);
    put<Word>(context.current_game_00e188a8,0x6ac,mapped);
    put<std::uint8_t>(settings,0x41,static_cast<std::uint8_t>(read<Word>(read<void*>(result,4),0x48)==0));
    const auto invert=static_cast<std::uint8_t>(read<Word>(read<void*>(result,4),0x70)!=0);
    put(settings,0x43,invert);put(settings,0x42,invert);
    put<std::uint8_t>(settings,0x40,static_cast<std::uint8_t>(read<Word>(read<void*>(result,4),0x98)==0));
    memory.free_00bf65ac(result);
}
void reset_native_profile_control_defaults_008d4820(void* settings,
    NativeProfileSettingsContext& context,NativeProfileCollectionCalls& memory){
    put<std::uint8_t>(settings,0x42,0);put<std::uint8_t>(settings,0x43,0);
    put<std::uint8_t>(settings,0x41,1);put<std::uint8_t>(settings,0x40,0);put<std::uint8_t>(settings,0x44,1);
    auto* const phase_receiver=context.current_manager_00f8abe8;
    if(phase_receiver&&read<Word>(phase_receiver,0x28)==2&&
        read<std::uint8_t>(context.current_manager_00f8abe8,0x119)!=0)
        import_native_profile_control_settings_008d45d0(settings,context,memory);
}
} // namespace bsp
