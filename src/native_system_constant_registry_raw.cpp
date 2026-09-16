#include "bsp/native_system_constant_registry.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_service_base.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_string_pool_storage.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <exception>
#include <new>
namespace bsp {
namespace {
using Owner=NativeSystemConstantRegistryStorage;
using Array=NativeCompiledShaderConstants;
using Record=NativeCompiledShaderConstantStorage;
struct Literal { const char* name; std::uint32_t second,first,array_count,id; };
constexpr Literal literals[]={
#include "shader_system_registry.inc"
};
static_assert(std::size(literals)==52);
Record* row(Record* data,std::int32_t index) noexcept {
    return reinterpret_cast<Record*>(reinterpret_cast<std::uintptr_t>(data)
        +static_cast<std::uint32_t>(index)*0x20u);
}
}
namespace {
using RawContext=NativeSystemConstantRegistryRawContext;
struct RawGuard {
    std::uint32_t profile;
    CRITICAL_SECTION* section;
};
static_assert(sizeof(RawGuard)==8);
static_assert(sizeof(CRITICAL_SECTION)==0x18);
struct RawRootCleanup {
    Owner& owner;
    bool armed=true;
    ~RawRootCleanup() noexcept {
        if(armed)destroy_native_generic_singleton_base_00412430(&owner);
    }
};
RawGuard enter_raw_section(void* manager) {
    CRITICAL_SECTION* section;
    std::memcpy(&section,static_cast<const char*>(manager)+0x10,sizeof(section));
    RawGuard guard{0x00ce37fc,section};
    if(section) {
        EnterCriticalSection(section);
        auto& depth=*reinterpret_cast<volatile std::uint32_t*>(
            reinterpret_cast<char*>(section)+0x18);
        depth=depth+1u;
    }
    return guard;
}
struct RawGuardCleanup {
    RawGuard& guard;
    bool armed=true;
    ~RawGuardCleanup() noexcept {
        if(armed) {
            try {destroy_native_singleton_guard_00411ee0(&guard);}
            catch(...) {std::terminate();}
        }
    }
};
void leave_raw_section(RawGuardCleanup& cleanup) {
    destroy_native_singleton_guard_00411ee0(&cleanup.guard);
    cleanup.armed=false;
}
struct RawStringCleanup {
    void* header;
    NativeStringRawPoolContext& strings;
    bool armed=true;
    ~RawStringCleanup() noexcept {
        if(armed) {
            try {destroy_native_string_header_0041dd20(header,strings);}
            catch(...) {std::terminate();}
        }
    }
};
void release_captured_name(char* data,std::uint32_t size,NativeStringRawPoolContext& strings) {
    if(!data)return;
    auto* pool=native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,data,size,
        strings.actual_small_returns_disabled_01090aa4);
}
// Own no allocations. Each even/odd state belongs only to the current literal;
// earlier completed records are represented by the CURRENT published count.
struct RawConstructorFrame {
    Owner& owner;
    RawContext& context;
    NativeString name;
    Record record;
    int state=-1;
    bool completed=false;
    ~RawConstructorFrame() noexcept {
        if(completed)return;
        try {
            while(state>=0) {
                if(state>=3 && (state&1)) {
                    --state; // CC0DDB+10h*n -> B34CC0, next even state.
                    destroy_native_string_header_0041dd20(&record.name_length_14,context.strings);
                } else if(state>=2) {
                    state=1; // CC0DD3+10h*n -> CURRENT name, not captured EDI.
                    destroy_native_string_header_0041dd20(&name,context.strings);
                } else if(state==1) {
                    state=0; // CC0DC8 -> owner+4, CURRENT data/count.
                    destroy_native_system_constant_array_00b5bf50(owner.constants_04,context.strings);
                } else {
                    state=-1; // CC0DC0 -> captured owner.
                    destroy_native_system_constant_base_00b5ba80(owner,context);
                }
            }
        } catch(...) {std::terminate();} // Secondary C++ cleanup exception boundary.
    }
};
} // namespace

Owner* construct_native_system_constant_base_00b5b9e0(Owner& owner,RawContext& context) {
    RawRootCleanup root{owner}; // Own state0, before D626F4 publication.
    owner.vtable_00=0x00d626f4;
    void* manager=get_native_singleton_manager_00415350(
        context.strings.actual_manager_publication_01090aa0);
    RawGuard guard=enter_raw_section(manager);
    RawGuardCleanup lock{guard}; // Own state1 only after enter/depth completes.
    context.current_registry_0108fe94=&owner;
    manager=get_native_singleton_manager_00415350(
        context.strings.actual_manager_publication_01090aa0);
    register_native_singleton_object_00bd0c30(manager,nullptr,context.current_registry_0108fe94);
    leave_raw_section(lock);
    root.armed=false;
    return &owner;
}
void destroy_native_system_constant_base_00b5ba80(Owner& owner,RawContext& context) {
    owner.vtable_00=0x00d626f4;
    RawRootCleanup root{owner};
    void* manager=get_native_singleton_manager_00415350(
        context.strings.actual_manager_publication_01090aa0);
    RawGuard guard=enter_raw_section(manager);
    RawGuardCleanup lock{guard};
    manager=get_native_singleton_manager_00415350(
        context.strings.actual_manager_publication_01090aa0);
    unregister_native_singleton_object_00bcfca0(manager,nullptr,context.current_registry_0108fe94);
    context.current_registry_0108fe94=nullptr;
    leave_raw_section(lock);
    // State0 root cleanup writes CE3818 after the captured first-section leave.
}
Record* construct_native_system_constant_00b5bbc0(void* fresh,const NativeString& name,
    std::uint32_t id,std::uint32_t second,std::uint32_t first,std::uint32_t array_count,
    NativeStringRawPoolContext& strings) {
    auto* record=::new(fresh) Record;
    record->name_length_14=0;record->name_data_18=nullptr;
    RawStringCleanup cleanup{&record->name_length_14,strings}; // B5BBF2, child state0.
    record->words_00[0]=0xffff;record->words_00[1]=second*first;
    if(static_cast<const void*>(&record->name_length_14)!=static_cast<const void*>(&name)) {
        resize_native_string_header_0041dd40(&record->name_length_14,strings,name.length(),true);
        if(name.length()!=0)std::memcpy(record->name_data_18,name.data(),record->name_length_14);
    }
    record->words_00[3]=first;record->words_00[2]=second;
    record->words_00[4]=array_count;record->word_1c=id;
    cleanup.armed=false;
    return record;
}
void reserve_native_system_constants_00b5bd10(Array& a,std::int32_t request,
    NativeStringRawPoolContext& strings) {
    if(request<1)request=1;
    if(request<=a.capacity_08)return;
    const auto bytes=static_cast<std::uint32_t>(request)*0x20u;
    auto* const data=static_cast<Record*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object,bytes,bytes}));
    for(std::int32_t i=0;i<a.count_04;++i) {
        auto* const destination=row(data,i);
        // Native state0 calls 401130 on failure: one RET, no cleanup. Fresh
        // buffer and partial/completed copies remain unpublished and can leak.
        if(destination)copy_native_compiled_shader_constant_00b38310(destination,*row(a.data_00,i),strings);
    }
    for(std::int32_t i=0;i<a.count_04;++i)
        destroy_native_string_header_0041dd20(&row(a.data_00,i)->name_length_14,strings);
    singleton_lifetime_free(a.data_00);
    a.data_00=data;a.capacity_08=request;
}
void append_native_system_constant_00b5bed0(Array& a,const Record& source,
    NativeStringRawPoolContext& strings) {
    if(a.count_04==a.capacity_08) {
        auto request=static_cast<std::int32_t>(static_cast<std::uint32_t>(a.capacity_08)*2u);
        if(request<2)request=1;
        reserve_native_system_constants_00b5bd10(a,request,strings);
    }
    auto* const destination=row(a.data_00,a.count_04);
    // Placement cleanup 401130 is also a no-op here. Failed copies remain
    // outside CURRENT count, even if their string has acquired storage.
    if(destination)copy_native_compiled_shader_constant_00b38310(destination,source,strings);
    ++a.count_04;
}
void resize_native_system_constants_00b5be10(Array& a,std::int32_t request,
    NativeStringRawPoolContext& strings) {
    if(a.capacity_08<request)reserve_native_system_constants_00b5bd10(a,request,strings);
    for(std::int32_t i=a.count_04;i<request;i=static_cast<std::int32_t>(static_cast<std::uint32_t>(i)+1u)) {
        auto* const destination=row(a.data_00,i);
        // Native state0 placement cleanup is RET-only; no added rollback.
        if(destination)initialize_native_compiled_shader_constant_00b5bb40(destination,strings);
    }
    while(request<a.count_04) {
        a.count_04=static_cast<std::int32_t>(static_cast<std::uint32_t>(a.count_04)-1u);
        destroy_native_string_header_0041dd20(&row(a.data_00,a.count_04)->name_length_14,strings);
    }
    a.count_04=request;
}
void destroy_native_system_constant_array_00b5bf50(Array& a,NativeStringRawPoolContext& strings) {
    resize_native_system_constants_00b5be10(a,0,strings);
    singleton_lifetime_free(a.data_00);
}
Owner* construct_native_system_constant_registry_00b5bf70(void* fresh,RawContext& context) {
    auto* const owner=::new(fresh) Owner;
    RawConstructorFrame frame{*owner,context};
    construct_native_system_constant_base_00b5b9e0(*owner,context); // Outer state -1.
    owner->vtable_00=0x00d62a3c;
    frame.state=0;
    owner->constants_04={nullptr,0,0};
    frame.state=1;
    int name_state=2;
    for(const auto& value:literals) {
        ::new(&frame.name) NativeString;
        resize_native_string_header_0041dd40(&frame.name,context.strings,
            static_cast<std::uint32_t>(std::strlen(value.name)),true);
        char* const captured_name=frame.name.data();
        if(captured_name)std::memcpy(captured_name,value.name,frame.name.length()+1u);
        frame.state=name_state; // Arm only after resize and literal copy.
        construct_native_system_constant_00b5bbc0(&frame.record,frame.name,value.id,
            value.second,value.first,value.array_count,context.strings);
        frame.state=name_state+1; // Arm only after child constructor returns.
        append_native_system_constant_00b5bed0(owner->constants_04,frame.record,context.strings);
        frame.state=name_state; // Disarm BEFORE the normal record release.
        destroy_native_string_header_0041dd20(&frame.record.name_length_14,context.strings);
        frame.state=1; // Disarm BEFORE captured EDI/current-length release.
        release_captured_name(captured_name,frame.name.length()+1u,context.strings);
        name_state+=2;
    }
    frame.completed=true; // Native unlinks its frame with state1 still stored.
    return owner;
}
} // namespace bsp
