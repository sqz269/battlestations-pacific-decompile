#include "bsp/native_frame_clock_lifetime.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <cstring>
#include <exception>
namespace bsp {
namespace {
struct Guard final { std::uint32_t profile; CRITICAL_SECTION* section; };
static_assert(sizeof(Guard)==8 && sizeof(CRITICAL_SECTION)==0x18);
void store(void* owner,std::size_t offset,std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(static_cast<std::byte*>(owner)+offset)=value;
}
volatile std::uint32_t& depth(CRITICAL_SECTION* section) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(reinterpret_cast<std::byte*>(section)+0x18);
}
Guard enter_first_manager(NativeFrameClockLifetimeContext& context) {
    void* const manager=get_native_singleton_manager_00415350(context.actual_manager_publication_01090aa0);
    CRITICAL_SECTION* captured;
    std::memcpy(&captured,static_cast<std::byte*>(manager)+0x10,4);
    Guard guard{0x00ce37fcu,captured};
    if(captured) {EnterCriticalSection(captured);depth(captured)=depth(captured)+1u;}
    return guard;
}
struct RootCleanup final {
    void* owner;
    bool armed=true;
    ~RootCleanup() noexcept {if(armed)destroy_native_generic_singleton_base_00412430(owner);}
};
struct GuardCleanup final {
    Guard& guard;
    bool armed=true;
    ~GuardCleanup() noexcept {
        if(armed)try {destroy_native_singleton_guard_00411ee0(&guard);}catch(...) {std::terminate();}
    }
};
void leave(GuardCleanup& cleanup) {
    destroy_native_singleton_guard_00411ee0(&cleanup.guard);
    cleanup.armed=false;
}
struct BaseCleanup final {
    void* owner;
    NativeFrameClockLifetimeContext& context;
    bool armed=true;
    ~BaseCleanup() noexcept {
        if(armed)try {destroy_native_frame_clock_base_00bedea0(owner,context);}catch(...) {std::terminate();}
    }
};
} // namespace

void* __fastcall construct_native_frame_clock_base_00bede00(
    void* owner, NativeFrameClockLifetimeContext& context) {
    RootCleanup root{owner}; // Native state0 -> CC7500 -> 00412430.
    store(owner,0,0x00d68d20u);
    Guard guard=enter_first_manager(context);
    GuardCleanup guard_cleanup{guard}; // Native state1 after entry/depth.
    context.actual_clock_publication_01090ab0=owner;
    void* const current_manager=get_native_singleton_manager_00415350(context.actual_manager_publication_01090aa0);
    void* const current_clock=context.actual_clock_publication_01090ab0;
    register_native_singleton_object_00bd0c30(current_manager,nullptr,current_clock);
    leave(guard_cleanup);
    root.armed=false;
    return owner;
}
void __fastcall destroy_native_frame_clock_base_00bedea0(
    void* owner, NativeFrameClockLifetimeContext& context) {
    store(owner,0,0x00d68d20u);
    RootCleanup root{owner}; // Native state0 -> CC7520 -> 00412430.
    Guard guard=enter_first_manager(context);
    GuardCleanup guard_cleanup{guard};
    void* const current_manager=get_native_singleton_manager_00415350(context.actual_manager_publication_01090aa0);
    void* const current_clock=context.actual_clock_publication_01090ab0;
    unregister_native_singleton_object_00bcfca0(current_manager,nullptr,current_clock);
    context.actual_clock_publication_01090ab0=nullptr;
    leave(guard_cleanup);
    destroy_native_generic_singleton_base_00412430(owner);
    root.armed=false;
}
void* __fastcall construct_native_frame_clock_00bedfb0(
    void* owner, NativeFrameClockLifetimeContext& context) {
    construct_native_frame_clock_base_00bede00(owner,context);
    store(owner,0x04,0); // Native XORPS/MOVSS writes positive-zero bits first.
    store(owner,0x00,0x00d68d50u);
    store(owner,0x08,0); store(owner,0x0c,0);
    store(owner,0x10,0); store(owner,0x14,0); store(owner,0x1c,0);
    store(owner,0x18,1);
    store(owner,0x28,1); store(owner,0x20,0); store(owner,0x24,0); store(owner,0x2c,0);
    store(owner,0x38,1); store(owner,0x30,0); store(owner,0x34,0); store(owner,0x3c,0);
    store(owner,0x48,1); store(owner,0x40,0); store(owner,0x44,0); store(owner,0x4c,0);
    store(owner,0x58,1); store(owner,0x50,0); store(owner,0x54,0); store(owner,0x5c,0);
    BaseCleanup base{owner,context}; // Native state0 -> CC7540 -> BEDEA0.
    auto* bytes=static_cast<volatile std::uint8_t*>(owner);
    bytes[0x68]=0; bytes[0x69]=0;
    initialize_native_frame_clock_00bedbd0(owner,&context.methods);
    base.armed=false;
    return owner;
}
// Complete 36-byte instruction schedule; original two calls relocated only.
__declspec(naked) void* __fastcall delete_native_frame_clock_00bee110(
    void*, NativeFrameClockLifetimeContext&, std::uint32_t) {
    __asm {
        push esi
        mov esi,ecx
        mov dword ptr [esi],00d68d50h
        call destroy_native_frame_clock_base_00bedea0
        test byte ptr [esp+8],1
        jz keep_owner
        push esi
        call singleton_lifetime_free
        add esp,4
    keep_owner:
        mov eax,esi
        pop esi
        ret 4
    }
}
} // namespace bsp
