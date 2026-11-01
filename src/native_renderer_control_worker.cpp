#include "bsp/native_renderer_control_worker.hpp"
#include "bsp/native_renderer_begin_frame.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <new>
#include <stdexcept>
#include <cstdlib>
#include "bsp/native_mesh_canonical.hpp"

namespace bsp {
namespace {
void* volatile process_context{};
const std::uint64_t milliseconds_00ce47a0=0x408f400000000000ull;
const std::uint32_t unsigned_correction_00ce3978=0x4f800000u;
const std::uint32_t negative_zero_00d7a208=0x80000000u;
NativeRendererControlWorkerContext& context() {
    auto* current=static_cast<NativeRendererControlWorkerContext*>(
        InterlockedCompareExchangePointer(&process_context,nullptr,nullptr));
    if(!current)throw std::logic_error("renderer worker process context is not bound");
    return *current;
}
std::uint32_t word(const void* owner) noexcept {
    return *static_cast<const volatile std::uint32_t*>(owner);
}
void require_slot(const void* owner,std::uint32_t expected_profile,
    const volatile std::uint32_t* table,std::size_t index,std::uint32_t target) {
    if(word(owner)!=expected_profile || !table || table[index]!=target)
        throw std::logic_error("renderer worker reached an unsupported actual virtual profile");
}
void signal(NativeEventOwnerStorage* event,NativeRendererControlWorkerContext& c) {
    require_slot(event,0x00d6821c,c.actual_event_profile_00d6821c,1,0x00bd1910);
    (void)signal_native_event_owner_00bd1910(event);
}
void wait(NativeEventOwnerStorage* event,NativeRendererControlWorkerContext& c) {
    require_slot(event,0x00d6821c,c.actual_event_profile_00d6821c,2,0x00bd17c0);
    (void)wait_native_event_owner_00bd17c0(event);
}
void delete_event(NativeEventOwnerStorage* event,NativeRendererControlWorkerContext& c) {
    require_slot(event,0x00d6821c,c.actual_event_profile_00d6821c,0,0x00bd19b0);
    delete_native_event_owner_00bd19b0(event,1);
}
void sample(NativeRendererControlWorkerContext& c,ClockTimestamp& output) {
    void* const clock=c.actual_clock_01090ab0;
    require_slot(clock,0x00d68d50,c.actual_clock_profile_00d68d50,8,0x00bee080);
    sample_native_frame_clock_00bee080(clock,nullptr,&output);
}
void begin_frame(NativeRendererControlWorkerContext& c) {
    void* const renderer=c.actual_renderer_00f8d394;
    require_slot(renderer,0x00d5f0a8,c.actual_renderer_profile_00d5f0a8,3,0x00b2b200);
    if(!c.begin_frame)throw std::logic_error("renderer worker actual begin-frame context is missing");
    (void)begin_native_renderer_frame_00b2b200(renderer,*c.begin_frame);
}
void end_frame(NativeRendererControlWorkerContext& c) {
    void* const renderer=c.actual_renderer_00f8d394;
    require_slot(renderer,0x00d5f0a8,c.actual_renderer_profile_00d5f0a8,5,0x00b2f4a0);
    if(!c.end_frame_00b2f4a0)
        throw std::logic_error("renderer worker requires actual B2F4A0/B2D8E0 EndFrame provider");
    c.end_frame_00b2f4a0(renderer,c.end_frame_context);
}
} // namespace

void bind_native_renderer_control_worker_process_context(NativeRendererControlWorkerContext& value) {
    void* const previous=InterlockedCompareExchangePointer(&process_context,&value,nullptr);
    if(previous && previous!=&value)
        throw std::logic_error("renderer worker process context cannot be rebound");
}

__declspec(naked) ClockTimestamp* __fastcall sample_native_frame_clock_00bee080(
    const void*,void*,ClockTimestamp*) {
    __asm {
        sub esp,8
        push esi
        mov esi,ecx
        cmp byte ptr [esi+69h],0
        jz actual_counter
        mov eax,dword ptr [esp+10h]
        mov ecx,dword ptr [esi+20h]
        mov edx,dword ptr [esi+24h]
        mov dword ptr [eax],ecx
        mov ecx,dword ptr [esi+28h]
        mov dword ptr [eax+4],edx
        mov edx,dword ptr [esi+2ch]
        mov dword ptr [eax+8],ecx
        mov dword ptr [eax+0ch],edx
        pop esi
        add esp,8
        ret 4
    actual_counter:
        lea eax,[esp+4]
        push eax
        call QueryPerformanceCounter
        mov eax,dword ptr [esp+10h]
        mov ecx,dword ptr [esp+4]
        mov edx,dword ptr [esp+8]
        mov dword ptr [eax],ecx
        mov ecx,dword ptr [esi+60h]
        mov dword ptr [eax+4],edx
        mov edx,dword ptr [esi+64h]
        mov dword ptr [eax+8],ecx
        mov dword ptr [eax+0ch],edx
        pop esi
        add esp,8
        ret 4
    }
}

bool native_renderer_worker_interval_00b33c72(const ClockTimestamp& stamp,
    const volatile std::uint32_t& previous,const volatile std::uint32_t& rate,
    std::uint32_t& output) noexcept {
    float seconds,delta,magnitude;
    unsigned char ready;
    __asm {
        mov eax,stamp
        fild qword ptr [eax]
        fild qword ptr [eax+8]
        fdivp st(1),st(0)
        fstp seconds
        fld seconds
        fld qword ptr [milliseconds_00ce47a0]
        fmul st(1),st(0)
        fxch st(1)
        mov eax,output
        fstp dword ptr [eax]
        fld dword ptr [eax]
        mov eax,previous
        fsub dword ptr [eax]
        fstp delta
        fldz
        fld delta
        fcomip st(0),st(1)
        fstp st(0)
        jbe reverse_delta
        movss xmm0,delta
        jmp magnitude_ready
    reverse_delta:
        movss xmm0,dword ptr [negative_zero_00d7a208]
        subss xmm0,delta
    magnitude_ready:
        mov ecx,rate
        mov edx,dword ptr [ecx]
        test edx,edx
        movss magnitude,xmm0
        fld magnitude
        fild dword ptr [ecx]
        jge unsigned_rate_ready
        fadd dword ptr [unsigned_correction_00ce3978]
    unsigned_rate_ready:
        fdivp st(2),st(0)
        fcomip st(0),st(1)
        fstp st(0)
        seta ready
    }
    return ready!=0;
}

static unsigned long __stdcall legacy_native_renderer_control_worker_thread_00b33c20(void* raw) {
    auto& owner=*static_cast<NativeRendererControlWorkerStorage*>(raw);
    auto& c=context();
    wait(owner.wake_0c,c);
    while(!owner.shutdown_05) {
        while(owner.run_04) {
            // Raw storage avoids default ClockTimestamp writes before the
            // current clock's virtual sample fills its native 10h destination.
            alignas(ClockTimestamp) std::byte stamp_storage[sizeof(ClockTimestamp)];
            auto& stamp=*reinterpret_cast<ClockTimestamp*>(stamp_storage);
            sample(c,stamp);
            std::uint32_t current;
            if(native_renderer_worker_interval_00b33c72(stamp,c.actual_time_bits_0108d6e4,owner.rate_18,current)) {
                c.actual_time_bits_0108d6e4=current;
                auto callback=owner.callback_1c;
                if(callback)callback(owner.callback_context_20);
                while(native_renderer_device_lifecycle_busy_00b20220(c.actual_renderer_00f8d394))
                    Sleep(10);
                // The native body performs this extra current-publication busy
                // test after leaving the wait loop before beginning a frame.
                if(!native_renderer_device_lifecycle_busy_00b20220(c.actual_renderer_00f8d394)) {
                    begin_frame(c);
                    end_frame(c);
                }
            } else SwitchToThread();
        }
        signal(owner.idle_10,c);
        wait(owner.wake_0c,c);
    }
    ExitThread(0);
}

static NativeRendererControlWorkerStorage* legacy_construct_native_renderer_control_worker_00b33da0(void* fresh) {
    auto& c=context();
    auto* owner=::new(fresh) NativeRendererControlWorkerStorage;
    owner->profile_00=0x00d5f1f0;
    owner->run_04=0;owner->shutdown_05=0;owner->thread_08=nullptr;owner->thread_id_14=0;
    c.actual_time_bits_0108d6e4=0;
    owner->wake_0c=create_native_event_owner_00bd1970(0);
    owner->idle_10=create_native_event_owner_00bd1970(0);
    owner->thread_08=CreateThread(nullptr,0,&native_renderer_control_worker_thread_00b33c20,
        owner,CREATE_SUSPENDED,reinterpret_cast<DWORD*>(const_cast<std::uint32_t*>(&owner->thread_id_14)));
    SetThreadPriority(owner->thread_08,-2);
    ResumeThread(owner->thread_08);
    return owner;
}

static void legacy_destroy_native_renderer_control_worker_00b33b50(NativeRendererControlWorkerStorage& owner) {
    auto& c=context();
    auto* const wake=owner.wake_0c; // B33B53 captures before profile/shutdown.
    owner.profile_00=0x00d5f1f0;owner.shutdown_05=1;
    signal(wake,c);
    WaitForSingleObject(owner.thread_08,INFINITE);
    CloseHandle(owner.thread_08);owner.thread_08=nullptr;
    if(auto* current=owner.wake_0c){delete_event(current,c);owner.wake_0c=nullptr;}
    if(auto* current=owner.idle_10){delete_event(current,c);owner.idle_10=nullptr;}
}

NativeRendererControlWorkerStorage* delete_native_renderer_control_worker_00b33c00(
    NativeRendererControlWorkerStorage* owner,std::uint32_t flags) {
    destroy_native_renderer_control_worker_00b33b50(*owner);
    if(flags&1)singleton_lifetime_free(owner);
    return owner;
}

NativeRendererRawFailureBridge::NativeRendererRawFailureBridge(
    NativeMeshRetentionState* state,NativeRendererControlWorkerContext& context) noexcept
    : retention_(state),context_(&context) {
    if(state) context.raw_failure_bridge=this;
}
bool NativeRendererRawFailureBridge::has_native_attempt() const noexcept {
    return actual_owner_.load(std::memory_order_acquire)!=nullptr;
}
bool NativeRendererRawFailureBridge::requires_native_join() const noexcept {
    return has_native_attempt() && !joined_.load(std::memory_order_acquire);
}
void NativeRendererRawFailureBridge::source_failure() noexcept {
    if(retention_) retention_->fail();
}
[[noreturn]] void NativeRendererRawFailureBridge::fail_and_exit() noexcept {
    source_failure();
    std::_Exit(1);
}
void NativeRendererRawFailureBridge::constructor_entered(
    void* actual,NativeRendererControlWorkerContext& context) noexcept {
    NativeRendererControlWorkerStorage* expected=nullptr;
    if(!retention_ || &context!=context_ || context.raw_failure_bridge!=this || !actual ||
        !actual_owner_.compare_exchange_strong(expected,
            static_cast<NativeRendererControlWorkerStorage*>(actual),std::memory_order_acq_rel))
        fail_and_exit();
    // This is only the actual ctor's opaque input identity/entered fact. It is
    // NOT an allocation receipt, completed Native birth or deletion authority.
}
void NativeRendererRawFailureBridge::thread_created(void* actual,
    NativeRendererControlWorkerContext& context,void* actual_CreateThread_result) noexcept {
    if(&context!=context_ || actual_owner_.load(std::memory_order_acquire)!=actual ||
        !actual_CreateThread_result) fail_and_exit();
    // Genuine Win32 return, captured before ResumeThread. No Native field
    // resample, adopted handle, extra OS query, retain or allocation.
    actual_thread_.store(actual_CreateThread_result,std::memory_order_release);
}
void NativeRendererRawFailureBridge::constructor_returned() noexcept {
    constructor_returned_.store(true,std::memory_order_release);
}
void NativeRendererRawFailureBridge::worker_entered(
    void* actual,NativeRendererControlWorkerContext& context) noexcept {
    if(&context!=context_ || context.raw_failure_bridge!=this ||
        actual_owner_.load(std::memory_order_acquire)!=actual ||
        !actual_thread_.load(std::memory_order_acquire) ||
        worker_entered_.exchange(true,std::memory_order_acq_rel)) fail_and_exit();
    // ResumeThread may schedule this before the actual ctor returns. The
    // genuine CreateThread continuation already followed its Native stores.
}
void NativeRendererRawFailureBridge::joined(void* actual,
    NativeRendererControlWorkerContext& context,void* actual_waited_thread,unsigned long result) noexcept {
    if(&context!=context_ || actual_owner_.load(std::memory_order_acquire)!=actual ||
        !actual_waited_thread || actual_thread_.load(std::memory_order_acquire)!=actual_waited_thread ||
        !constructor_returned_.load(std::memory_order_acquire) || result!=WAIT_OBJECT_0)
        fail_and_exit();
    joined_.store(true,std::memory_order_release);
}

unsigned long __stdcall native_renderer_control_worker_thread_00b33c20(void* raw) {
    auto* current=static_cast<NativeRendererControlWorkerContext*>(
        InterlockedCompareExchangePointer(&process_context,nullptr,nullptr));
    auto* bridge=current?current->raw_failure_bridge:nullptr;
    if(!bridge) return legacy_native_renderer_control_worker_thread_00b33c20(raw);
    try {
        bridge->worker_entered(raw,*current);
        return legacy_native_renderer_control_worker_thread_00b33c20(raw);
    } catch(...) {
        // Atomic metadata only. No logging/current_exception/provider unwind,
        // idle signal/run repair, allocation or graph destruction is added.
        bridge->fail_and_exit();
    }
}
NativeRendererControlWorkerStorage* construct_native_renderer_control_worker_00b33da0(void* fresh) {
    auto* current=static_cast<NativeRendererControlWorkerContext*>(
        InterlockedCompareExchangePointer(&process_context,nullptr,nullptr));
    auto* bridge=current?current->raw_failure_bridge:nullptr;
    if(!bridge) return legacy_construct_native_renderer_control_worker_00b33da0(fresh);
    bridge->constructor_entered(fresh,*current); // before any original ctor effect/CreateThread
    try {
        auto& c=*current;
        auto* owner=::new(fresh) NativeRendererControlWorkerStorage;
        owner->profile_00=0x00d5f1f0;
        owner->run_04=0;owner->shutdown_05=0;owner->thread_08=nullptr;owner->thread_id_14=0;
        c.actual_time_bits_0108d6e4=0;
        owner->wake_0c=create_native_event_owner_00bd1970(0);
        owner->idle_10=create_native_event_owner_00bd1970(0);
        void* const actual_created_thread=CreateThread(nullptr,0,&native_renderer_control_worker_thread_00b33c20,
            owner,CREATE_SUSPENDED,reinterpret_cast<DWORD*>(const_cast<std::uint32_t*>(&owner->thread_id_14)));
        owner->thread_08=actual_created_thread;
        bridge->thread_created(owner,c,actual_created_thread);
        SetThreadPriority(owner->thread_08,-2);
        ResumeThread(owner->thread_08);
        bridge->constructor_returned();
        return owner;
    } catch(...) { bridge->source_failure(); throw; }
}
void destroy_native_renderer_control_worker_00b33b50(NativeRendererControlWorkerStorage& owner) {
    auto& c=context();
    auto* bridge=c.raw_failure_bridge;
    if(!bridge) { legacy_destroy_native_renderer_control_worker_00b33b50(owner); return; }
    if(bridge->actual_owner_.load(std::memory_order_acquire)!=&owner ||
        !bridge->constructor_returned_.load(std::memory_order_acquire)) bridge->fail_and_exit();
    try {
        auto* const wake=owner.wake_0c;
        owner.profile_00=0x00d5f1f0;owner.shutdown_05=1;
        signal(wake,c);
        void* const actual_waited_thread=owner.thread_08;
        const DWORD actual_wait_result=WaitForSingleObject(actual_waited_thread,INFINITE);
        bridge->joined(&owner,c,actual_waited_thread,actual_wait_result); // genuine return, no later idle inference
        CloseHandle(owner.thread_08);owner.thread_08=nullptr;
        if(auto* current=owner.wake_0c){delete_event(current,c);owner.wake_0c=nullptr;}
        if(auto* current=owner.idle_10){delete_event(current,c);owner.idle_10=nullptr;}
    } catch(...) { bridge->source_failure(); throw; }
}

} // namespace bsp
