#include "bsp/native_material_pass_states.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t reference_table=0x00ceb130,render_table=0x00d61a2c,
    sampler_table=0x00d61a34,third_table=0x00d61a3c;
struct BaseCleanup {
    NativeMaterialStateOwnerStorage& owner;
    ~BaseCleanup(){owner.vtable_00=reference_table;}
};
void require_extent(const NativeMaterialStateArray& rows,std::int32_t limit) {
    if(rows.count_04<0 || rows.count_04>limit || (rows.count_04 && !rows.data_00))
        throw std::logic_error("native material state rows exceed accessible extent");
}
void reserve(NativeMaterialStateArray& rows,std::int32_t requested,std::uint32_t stride) {
    if(requested<1)requested=1;
    if(rows.capacity_08>=requested)return;
    if(static_cast<std::uint32_t>(requested)>std::numeric_limits<std::uint32_t>::max()/stride)
        throw std::bad_alloc();
    require_extent(rows,requested);
    const auto bytes=static_cast<std::size_t>(requested)*stride;
    auto* const fresh=static_cast<std::byte*>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    // Reload source/count after the allocation boundary. Publication follows
    // free, matching the previously omitted instruction tail.
    try {
        require_extent(rows,requested);
        for(std::int32_t i=0;i<rows.count_04;++i)
            std::memcpy(fresh+static_cast<std::size_t>(i)*stride,
                static_cast<std::byte*>(rows.data_00)+static_cast<std::size_t>(i)*stride,stride);
    } catch(...) {singleton_lifetime_free(fresh);throw;}
    singleton_lifetime_free(rows.data_00);
    rows.data_00=fresh;
    rows.capacity_08=requested;
}
NativeMaterialStateOwnerStorage& pass_owner(void* pass,std::size_t offset) {
    if(!pass)throw std::invalid_argument("native material state operation requires actual pass");
    NativeMaterialStateOwnerStorage* owner;
    std::memcpy(&owner,static_cast<std::byte*>(pass)+offset,sizeof(owner));
    if(!owner)throw std::invalid_argument("native material pass requires initialized state owner");
    return *owner;
}
std::int32_t doubled_capacity(std::int32_t old) noexcept {
    const auto bits=static_cast<std::uint32_t>(old)*2u;
    std::int32_t value;std::memcpy(&value,&bits,sizeof(value));
    return value>1?value:1;
}
void destroy(NativeMaterialStateOwnerStorage& owner,std::uint32_t stride) {
    const BaseCleanup base{owner};
    auto& rows=owner.rows_08;
    if(rows.capacity_08<0)reserve(rows,0,stride);
    while(rows.count_04>0)--rows.count_04;
    rows.count_04=0;
    singleton_lifetime_free(rows.data_00);
}
} // namespace

NativeMaterialStateOwnerStorage* initialize_native_material_state_owner_00b5f720_fragment(
    void* slot,NativeMaterialStateKind kind) {
    if(!slot || reinterpret_cast<std::uintptr_t>(slot)%alignof(NativeMaterialStateOwnerStorage))
        throw std::invalid_argument("native state initialization requires aligned14h storage");
    std::uint32_t table;
    switch(kind) {
    case NativeMaterialStateKind::render:table=render_table;break;
    case NativeMaterialStateKind::sampler:table=sampler_table;break;
    case NativeMaterialStateKind::third:table=third_table;break;
    default:throw std::invalid_argument("unknown native material state profile");
    }
    auto* owner=::new(slot) NativeMaterialStateOwnerStorage;
    owner->vtable_00=reference_table;
    owner->references_04.store(1,std::memory_order_relaxed);
    owner->vtable_00=table;
    owner->rows_08={nullptr,0,0};
    return owner;
}
void reserve_native_material_render_states_00b40ac0(NativeMaterialStateArray& rows,std::int32_t capacity){reserve(rows,capacity,8);}
void reserve_native_material_third_states_00b40b40(NativeMaterialStateArray& rows,std::int32_t capacity){reserve(rows,capacity,12);}
void reserve_native_material_sampler_states_00b40be0(NativeMaterialStateArray& rows,std::int32_t capacity){reserve(rows,capacity,12);}

void set_native_material_render_state_00b5ec40(void* pass,std::uint32_t state,std::uint32_t value) {
    auto& rows=pass_owner(pass,0x18).rows_08;
    require_extent(rows,rows.capacity_08);
    const auto count=rows.count_04;
    auto* const data=static_cast<NativeMaterialRenderState*>(rows.data_00);
    for(std::int32_t i=0;i!=count;++i)if(data[i].state==state) {
        if(data[i].value!=value)data[i]={state,value};
        return;
    }
    if(rows.count_04==rows.capacity_08)reserve_native_material_render_states_00b40ac0(rows,doubled_capacity(rows.capacity_08));
    require_extent(rows,rows.capacity_08-1);
    static_cast<NativeMaterialRenderState*>(rows.data_00)[rows.count_04]={state,value};
    ++rows.count_04;
}
void set_native_material_sampler_state_00b5ed60(void* pass,std::uint32_t slot,std::uint32_t state,std::uint32_t value) {
    auto& rows=pass_owner(pass,0x20).rows_08;
    require_extent(rows,rows.capacity_08);
    const auto count=rows.count_04;
    auto* const data=static_cast<NativeMaterialSamplerState*>(rows.data_00);
    for(std::int32_t i=0;i!=count;++i)if(data[i].slot==slot && data[i].state==state) {
        if(data[i].value!=value)data[i]={slot,state,value};
        return;
    }
    if(rows.count_04==rows.capacity_08)reserve_native_material_sampler_states_00b40be0(rows,doubled_capacity(rows.capacity_08));
    require_extent(rows,rows.capacity_08-1);
    static_cast<NativeMaterialSamplerState*>(rows.data_00)[rows.count_04]={slot,state,value};
    ++rows.count_04;
}
void remove_native_material_render_state_00b5ee00(void* pass,std::uint32_t state) {
    auto& rows=pass_owner(pass,0x18).rows_08;
    require_extent(rows,rows.capacity_08);
    const auto count=rows.count_04;
    auto* const data=static_cast<NativeMaterialRenderState*>(rows.data_00);
    for(std::int32_t i=0;i!=count;++i)if(data[i].state==state) {
        if(i!=count-1) {
            auto* const current=static_cast<NativeMaterialRenderState*>(rows.data_00);
            current[i]=current[rows.count_04-1];
        }
        auto& current=pass_owner(pass,0x18).rows_08;
        require_extent(current,current.capacity_08);
        const auto target=current.count_04-1;
        if(target<0)throw std::logic_error("native material state count changed during removal");
        if(current.capacity_08<target)reserve_native_material_render_states_00b40ac0(current,target);
        while(target<current.count_04)--current.count_04;
        current.count_04=target;
        return;
    }
}
void destroy_native_material_render_states_00b420c0(NativeMaterialStateOwnerStorage& owner){destroy(owner,8);}
void destroy_native_material_sampler_states_00b42140(NativeMaterialStateOwnerStorage& owner){destroy(owner,12);}
void destroy_native_material_third_states_00b421c0(NativeMaterialStateOwnerStorage& owner){destroy(owner,12);}
NativeMaterialStateOwnerStorage* delete_native_material_render_states_00b422f0(NativeMaterialStateOwnerStorage* owner,std::uint32_t flags) {
    destroy_native_material_render_states_00b420c0(*owner);if(flags&1)singleton_lifetime_free(owner);return owner;
}
NativeMaterialStateOwnerStorage* delete_native_material_sampler_states_00b42310(NativeMaterialStateOwnerStorage* owner,std::uint32_t flags) {
    destroy_native_material_sampler_states_00b42140(*owner);if(flags&1)singleton_lifetime_free(owner);return owner;
}
NativeMaterialStateOwnerStorage* delete_native_material_third_states_00b42330(NativeMaterialStateOwnerStorage* owner,std::uint32_t flags) {
    destroy_native_material_third_states_00b421c0(*owner);if(flags&1)singleton_lifetime_free(owner);return owner;
}

NativeMaterialStateReference::NativeMaterialStateReference(NativeMaterialStateOwnerStorage& owner,
    const volatile std::uint32_t* profile,NativeMaterialStateCompanionDisposal disposal)
    :RenderCommandReference(owner.references_04),storage_(owner),profile_(profile),disposal_(disposal),bound_table_(owner.vtable_00) {
    if(!disposal.retire || owner.references_04.load(std::memory_order_relaxed)<=0)
        throw std::invalid_argument("native material state reference requires live storage and retirement");
    require_current_profile();
}
NativeMaterialStateReference::~NativeMaterialStateReference(){if(phase_!=Phase::retired)std::terminate();}
void NativeMaterialStateReference::require_current_profile() const noexcept {
    if(storage_.vtable_00!=bound_table_ || !profile_ || profile_[0]!=0x00bd30e0)std::terminate();
    if(bound_table_==render_table){if(profile_[1]!=0x00b422f0)std::terminate();}
    else if(bound_table_==sampler_table){if(profile_[1]!=0x00b42310)std::terminate();}
    else if(bound_table_==third_table){if(profile_[1]!=0x00b42330)std::terminate();}
    else std::terminate();
}
void NativeMaterialStateReference::release_zero_references() noexcept {
    if(phase_!=Phase::bound)std::terminate();
    require_current_profile();phase_=Phase::destroying;
    const auto disposal=disposal_;
    try {
        if(bound_table_==render_table)delete_native_material_render_states_00b422f0(&storage_,1);
        else if(bound_table_==sampler_table)delete_native_material_sampler_states_00b42310(&storage_,1);
        else delete_native_material_third_states_00b42330(&storage_,1);
    } catch(...) {std::terminate();}
    phase_=Phase::retired;disposal.retire(disposal.context,*this);
}
} // namespace bsp
