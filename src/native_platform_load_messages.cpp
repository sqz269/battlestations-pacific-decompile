#include "bsp/native_platform_load_messages.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Op=NativePlatformLoadMessagesOperation;
template<class T> T read(const void* p,std::uint32_t n=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const char*>(p)+n);
}
void loading_update(NativeInputCursorContext& c) {
    void* const backend=c.backend_00f8bbf4;
    const volatile float* const step=&c.loading_step_00d7a2f0;
    float seconds;std::uint32_t profile;
    __asm {
        mov eax, step
        fld dword ptr [eax]
        mov eax, backend
        mov eax, dword ptr [eax]
        mov profile, eax
        fstp seconds
    }
    c.calls.backend_vslot04(backend,profile,seconds);
}
void cursor(void* platform,std::uint8_t loading,NativePlatformLoadMessagesContext& c,Op& a) {
    if(!c.actual_online_00f8abe8)return;
    void* const backend=c.actual_input_00f8bbf4;
    if(!backend)return;
    if(!c.input||&c.input->backend_00f8bbf4!=&c.actual_input_00f8bbf4)
        throw std::logic_error("load cursor requires the actual input publication context");
    auto& input=*c.input;
    a.native_site=0x00becb43;
    if(!input.calls.call_004ba6d0(backend,1,0))return;
    if(loading) {
        a.native_site=0x00becb5d;
        a.captured_online=c.actual_online_00f8abe8;
        if(!c.online||a.captured_online!=&c.online->notifications.manager)
            throw std::logic_error("load cursor requires the captured actual online pump context");
        pump_native_online_00a409f0(*c.online);
    }
    const auto focused=read<std::uint8_t>(platform,0x41);
    const auto system_ui=read<std::uint8_t>(c.actual_online_00f8abe8,0x3e8);
    const bool should_show=focused==0||system_ui!=0;
    bool reset=false;
    auto& globals=input.globals;
    if(system_ui&&!globals.previous_system_ui_0109db90) {
        a.native_site=0x00becb9d;reset_native_input_focus_00beca40(input);reset=true;
        a.native_site=0x00becbae;
        void* const mouse=input.calls.call_004ba6d0(input.backend_00f8bbf4,1,0);
        a.native_site=0x00becbb7;input.calls.call_00a9a140(mouse,6);
        if(loading){a.native_site=0x00becbd8;loading_update(input);}
    }
    if(!system_ui&&globals.previous_system_ui_0109db90&&should_show)
        globals.focus_reset_pending_0109db8f=1;
    if(!should_show&&(globals.previous_system_ui_0109db90||globals.focus_reset_pending_0109db8f)) {
        a.native_site=0x00becbfc;reset_native_input_focus_00beca40(input);reset=true;
        if(loading){a.native_site=0x00becc1f;loading_update(input);}
        globals.focus_reset_pending_0109db8f=0;
    }
    if(should_show) {
        if(!globals.cursor_shown_0109db8e) {
            const auto show=input.show_cursor_00ce2344;
            globals.cursor_shown_0109db8e=1;
            a.native_site=0x00becc52;while(show(1)<0){}
        }
    } else if(globals.cursor_shown_0109db8e) {
        if(!reset) {
            a.native_site=0x00becc7b;reset_native_input_focus_00beca40(input);
            if(loading){a.native_site=0x00becc9b;loading_update(input);}
        }
        const auto show=input.show_cursor_00ce2344;
        globals.cursor_shown_0109db8e=0;
        a.native_site=0x00beccb2;while(show(0)>=0){}
    }
    globals.previous_system_ui_0109db90=system_ui;
}
void begin(void* platform,NativePlatformLoadMessagesContext& c,Op& a) {
    if(a.phase!=Op::Phase::fresh)throw std::logic_error("platform load operation is one-shot");
    a.captured_platform=platform;a.context=&c;a.phase=Op::Phase::running;
}
}
NativePlatformLoadMessagesOperation::~NativePlatformLoadMessagesOperation(){
    if(phase==Phase::running||phase==Phase::failed)std::terminate();
}
void NativePlatformLoadMessagesOperation::acknowledge_diagnostic_cleanup() noexcept {
    if(phase==Phase::failed)phase=Phase::diagnostic_retired;
}
void update_native_platform_load_cursor_00becb20(void* platform,std::uint8_t loading,
    NativePlatformLoadMessagesContext& c,Op& a) {
    begin(platform,c,a);
    try{cursor(platform,loading,c,a);a.phase=Op::Phase::complete;}
    catch(...){a.phase=Op::Phase::failed;throw;}
}
void pump_native_platform_load_messages_00beccd0(void* platform,
    NativePlatformLoadMessagesContext& c,Op& a) {
    begin(platform,c,a);
    const auto dispatch=&DispatchMessageA;
    const auto peek=&PeekMessageA;
    const auto translate=&TranslateMessage;
    try {
        for(;;) {
            a.native_site=0x00beccfd;
            if(!peek(&a.message,nullptr,0,0,PM_REMOVE))break;
            a.native_site=0x00becd0d;
            if(!c.xlive.pretranslate(a.message)) {
                a.native_site=0x00becd1b;translate(&a.message);
                a.native_site=0x00becd22;dispatch(&a.message);
            }
        }
        a.native_site=0x00becd2a;cursor(platform,1,c,a);
        a.phase=Op::Phase::complete;
    }catch(...){a.phase=Op::Phase::failed;throw;}
}
} // namespace bsp
