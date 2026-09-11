#include "bsp/native_material_pass_base.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
void require_storage(void* slot) {
    if(!slot || reinterpret_cast<std::uintptr_t>(slot)%alignof(NativeMaterialPassBaseStorage))
        throw std::invalid_argument("actual material pass requires aligned fresh storage");
}
struct RootCleanup {
    NativeMaterialPassRootStorage& root;bool armed=true;
    ~RootCleanup(){if(armed)root.vtable_00=0x00ceb130;}
};
struct ArrayCleanup {
    NativeMaterialStateArray& rows;void (*destroy)(NativeMaterialStateArray&);bool armed=true;
    ~ArrayCleanup() noexcept(false){if(armed)destroy(rows);}
};
using Reserve=void (*)(NativeMaterialStateArray&,std::int32_t);
void destroy_rows(NativeMaterialStateArray& rows,Reserve reserve) {
    if(rows.capacity_08<0)reserve(rows,0);
    while(rows.count_04>0)--rows.count_04;
    rows.count_04=0;singleton_lifetime_free(rows.data_00);
}
NativeMaterialStateOwnerStorage* create_state(NativeMaterialStateKind kind) {
    return initialize_native_material_state_owner_00b5f720_fragment(
        singleton_lifetime_allocate({SingletonAllocationKind::object,20,20}),kind);
}
template<class T> void release_clear(T*& field,NativeRenderActualOwners& owners) {
    T* const old=field;if(!old)return;
    release_native_render_actual_owner(owners,old);field=nullptr;
}
} // namespace

NativeMaterialPassRootStorage* initialize_native_material_pass_root_00b17940(void* slot) {
    require_storage(slot);auto* root=::new(slot) NativeMaterialPassRootStorage;
    root->vtable_00=0x00ceb130;root->references_04.store(1,std::memory_order_relaxed);
    root->vtable_00=0x00d5e508;root->word_08=0;root->word_0c=0;root->word_10=0;root->borrowed_effect_14=nullptr;
    return root;
}
NativeMaterialPassBaseStorage* initialize_native_material_pass_base_00b5f720(void* slot) {
    require_storage(slot);auto* pass=::new(slot) NativeMaterialPassBaseStorage;
    initialize_native_material_pass_root_00b17940(&pass->root);
    pass->root.vtable_00=0x00d62a80;
    pass->pairs_24={nullptr,0,0};pass->render_rows_30={nullptr,0,0};
    pass->third_rows_3c={nullptr,0,0};pass->sampler_rows_48={nullptr,0,0};
    pass->retained_54=nullptr;pass->retained_58=nullptr;
    RootCleanup root{pass->root};
    ArrayCleanup pairs{pass->pairs_24,destroy_native_material_pass_pairs_00b5f3f0};
    ArrayCleanup render{pass->render_rows_30,destroy_native_material_pass_render_rows_00b41bd0};
    ArrayCleanup third{pass->third_rows_3c,destroy_native_material_pass_third_rows_00b41ca0};
    ArrayCleanup sampler{pass->sampler_rows_48,destroy_native_material_pass_sampler_rows_00b41d90};
    pass->render_18=create_state(NativeMaterialStateKind::render);
    pass->sampler_20=create_state(NativeMaterialStateKind::sampler);
    pass->third_1c=create_state(NativeMaterialStateKind::third);
    constexpr NativeMaterialRenderState defaults[]{
        {0xc3,0},{0xaf,0},{7,1},{0x0e,1},{0x17,2},{0x0f,0},{0x1b,0},{0xab,1},{0xce,0},{0xd1,1},
        {0x16,3},{8,3},{0xa8,0x0f},{0xbe,0x0f},{0xbf,0x0f},{0xc0,0x0f},{0xb5,0},{0x9a,0x304d3241},{0xa1,1}};
    for(const auto& row:defaults)set_native_material_render_state_00b5ec40(pass,row.state,row.value);
    for(std::uint32_t slot_index=0;slot_index<16;++slot_index) {
        set_native_material_sampler_state_00b5ed60(pass,slot_index,6,2);
        set_native_material_sampler_state_00b5ed60(pass,slot_index,5,2);
        set_native_material_sampler_state_00b5ed60(pass,slot_index,7,1);
        set_native_material_sampler_state_00b5ed60(pass,slot_index,1,1);
        set_native_material_sampler_state_00b5ed60(pass,slot_index,2,1);
        set_native_material_sampler_state_00b5ed60(pass,slot_index,8,0);
    }
    set_native_material_render_state_00b5ec40(pass,0x34,0);
    sampler.armed=third.armed=render.armed=pairs.armed=root.armed=false;
    return pass;
}
void reserve_native_material_pass_pairs_00b40c80(NativeMaterialStateArray& rows,std::int32_t request) {
    // B40C80 and B40AC0 differ only in relocated instruction addresses/calls.
    reserve_native_material_render_states_00b40ac0(rows,request);
}
void resize_native_material_pass_pairs_00b40e00(NativeMaterialStateArray& rows,std::int32_t request) {
    if(request<0 || rows.count_04<0 || (rows.count_04 && !rows.data_00))
        throw std::logic_error("native pass pair resize requires valid nonnegative extent");
    if(rows.capacity_08<request)reserve_native_material_pass_pairs_00b40c80(rows,request);
    for(auto i=rows.count_04;i<request;++i) {
        if(!rows.data_00)throw std::logic_error("native pass pair resize requires actual array storage");
        std::memset(static_cast<std::byte*>(rows.data_00)+static_cast<std::size_t>(i)*8,0,8);
    }
    while(request<rows.count_04)--rows.count_04;
    rows.count_04=request;
}
void destroy_native_material_pass_pairs_00b5f3f0(NativeMaterialStateArray& rows) {
    resize_native_material_pass_pairs_00b40e00(rows,0);singleton_lifetime_free(rows.data_00);
}
void destroy_native_material_pass_render_rows_00b41bd0(NativeMaterialStateArray& rows){destroy_rows(rows,reserve_native_material_render_states_00b40ac0);}
void destroy_native_material_pass_third_rows_00b41ca0(NativeMaterialStateArray& rows){destroy_rows(rows,reserve_native_material_third_states_00b40b40);}
void destroy_native_material_pass_sampler_rows_00b41d90(NativeMaterialStateArray& rows){destroy_rows(rows,reserve_native_material_sampler_states_00b40be0);}

void destroy_native_material_pass_base_00b5f510(NativeMaterialPassBaseStorage& pass,NativeRenderActualOwners& owners) {
    pass.root.vtable_00=0x00d62a80;
    const RootCleanup root{pass.root};
    const ArrayCleanup pairs{pass.pairs_24,destroy_native_material_pass_pairs_00b5f3f0};
    const ArrayCleanup render{pass.render_rows_30,destroy_native_material_pass_render_rows_00b41bd0};
    const ArrayCleanup third{pass.third_rows_3c,destroy_native_material_pass_third_rows_00b41ca0};
    const ArrayCleanup sampler{pass.sampler_rows_48,destroy_native_material_pass_sampler_rows_00b41d90};
    release_clear(pass.render_18,owners);release_clear(pass.sampler_20,owners);release_clear(pass.third_1c,owners);
    release_clear(pass.retained_54,owners);release_clear(pass.retained_58,owners);
}
NativeMaterialPassBaseStorage* delete_native_material_pass_base_00b5f990(
    NativeMaterialPassBaseStorage* pass,NativeRenderActualOwners& owners,std::uint32_t flags) {
    destroy_native_material_pass_base_00b5f510(*pass,owners);if(flags&1)singleton_lifetime_free(pass);return pass;
}
NativeMaterialPassBaseReference::NativeMaterialPassBaseReference(NativeMaterialPassBaseStorage& pass,NativeRenderActualOwners& owners,
    const volatile std::uint32_t* profile,NativeMaterialPassBaseCompanionDisposal disposal)
    :RenderCommandReference(pass.root.references_04),storage_(pass),owners_(owners),profile_(profile),disposal_(disposal) {
    if(!disposal.retire || pass.root.references_04.load(std::memory_order_relaxed)<=0)
        throw std::invalid_argument("native pass base reference requires live owner and explicit retirement");
    require_current_profile();
}
NativeMaterialPassBaseReference::~NativeMaterialPassBaseReference(){if(phase_!=Phase::retired)std::terminate();}
void NativeMaterialPassBaseReference::require_current_profile() const noexcept {
    if(storage_.root.vtable_00!=0x00d62a80 || !profile_ || profile_[0]!=0x00bd30e0 || profile_[1]!=0x00b5f990)std::terminate();
}
void NativeMaterialPassBaseReference::release_zero_references() noexcept {
    if(phase_!=Phase::bound)std::terminate();require_current_profile();phase_=Phase::destroying;
    const auto disposal=disposal_;
    try {delete_native_material_pass_base_00b5f990(&storage_,owners_,1);}catch(...){std::terminate();}
    phase_=Phase::retired;disposal.retire(disposal.context,*this);
}
} // namespace bsp
