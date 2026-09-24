#include "bsp/native_shadow_depth_target_owner.hpp"
#include "bsp/native_material_compiler_providers.hpp"
#include "bsp/native_renderer_format_check.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/native_singleton_removal_reorder.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <new>
#include <stdexcept>
#include <vector>

namespace bsp {
namespace {
using U=std::uint32_t;
void require(bool value,const char* message) {if(!value)throw std::logic_error(message);}
void* at(void* p,U offset) noexcept {return static_cast<char*>(p)+offset;}
U word(const void* p,U offset=0) noexcept {return *reinterpret_cast<const volatile U*>(static_cast<const char*>(p)+offset);}
void put(void* p,U offset,U value) noexcept {*reinterpret_cast<volatile U*>(at(p,offset))=value;}
void byte(void* p,U offset,std::uint8_t value) noexcept {*static_cast<volatile std::uint8_t*>(at(p,offset))=value;}
void* child(void* p,U offset) noexcept {return reinterpret_cast<void*>(word(p,offset));}
void entry(void* p,U profile,const volatile U* table,U offset,U target) {
    const U current=word(p);
    const U captured=table?table[offset/4]:0;
    require(current==profile&&captured==target,"shadow target reached an unsupported current native table entry");
}
template<class Body> void perform(NativeShadowDepthTargetContext& c,NativeShadowDepthTargetCall kind,
    void* owner,Body body) {
    auto& a=c.begin(kind,owner);
    try {body(a);a.phase=NativeShadowDepthTargetOperation::Phase::complete;}
    catch(...) {a.phase=NativeShadowDepthTargetOperation::Phase::failed;throw;}
}
void enter_section(NativeShadowDepthTargetOperation& a,NativeShadowDepthTargetContext& c) {
    if(!a.captured_section)return;
    a.captured_enter=c.bindings.enter_00ce2218;
    require(a.captured_enter!=nullptr,"shadow target requires current EnterCriticalSection import");
    a.captured_enter(a.captured_section);a.section_entered=true;
    put(a.captured_section,0x18,word(a.captured_section,0x18)+1u);a.section_incremented=true;
}
void leave_section(NativeShadowDepthTargetOperation& a,NativeShadowDepthTargetContext& c) {
    if(!a.captured_section)return;
    put(a.captured_section,0x18,word(a.captured_section,0x18)-1u);a.section_incremented=false;
    a.captured_leave=c.bindings.leave_00ce2210;
    require(a.captured_leave!=nullptr,"shadow target requires current LeaveCriticalSection import");
    a.captured_leave(a.captured_section);a.section_entered=false;
}
void base_construct(void* owner,NativeShadowDepthTargetContext& c,NativeShadowDepthTargetOperation& a) {
    const auto& b=c.bindings;
    a.singleton_exception_state=0;put(owner,0,0x00d5b554);
    a.native_site=0x00a8a84e;
    void* manager=get_native_singleton_manager_00415350(b.manager_01090aa0);
    a.captured_section=child(manager,0x10);a.native_site=0x00a8a867;enter_section(a,c);
    a.singleton_exception_state=1;
    b.publication_00f8bbf0=owner;a.publication_written=true;
    a.native_site=0x00a8a87c;manager=get_native_singleton_manager_00415350(b.manager_01090aa0);
    a.registration_argument=b.publication_00f8bbf0;
    a.native_site=0x00a8a88a;register_native_singleton_object_00bd0c30(manager,nullptr,a.registration_argument);
    a.native_site=0x00a8a898;leave_section(a,c);
}
void base_destroy(void* owner,NativeShadowDepthTargetContext& c,NativeShadowDepthTargetOperation& a) {
    const auto& b=c.bindings;
    put(owner,0,0x00d5b554);a.singleton_exception_state=0;
    a.native_site=0x00a8a8ee;
    void* manager=get_native_singleton_manager_00415350(b.manager_01090aa0);
    a.captured_section=child(manager,0x10);a.native_site=0x00a8a907;enter_section(a,c);
    a.singleton_exception_state=1;
    a.native_site=0x00a8a916;manager=get_native_singleton_manager_00415350(b.manager_01090aa0);
    a.registration_argument=b.publication_00f8bbf0;
    a.native_site=0x00a8a924;unregister_native_singleton_object_00bcfca0(manager,nullptr,a.registration_argument);
    b.publication_00f8bbf0=nullptr;a.publication_written=true;
    a.native_site=0x00a8a93c;leave_section(a,c);put(owner,0,0x00ce3818);
}
void extents_construct(void* owner,const volatile NativeShadowDepthTargetDimensions& input,
    NativeShadowDepthTargetContext& c,NativeShadowDepthTargetOperation& a) {
    base_construct(owner,c,a);
    const U width=input.width;const U height=input.height;
    put(owner,4,width);put(owner,0,0x00d5b558);put(owner,8,height);byte(owner,0x0e,0);
}
void extents_destroy(void* owner,NativeShadowDepthTargetContext& c,NativeShadowDepthTargetOperation& a) {
    put(owner,0,0x00d5b558);base_destroy(owner,c,a);
}
const void* capabilities(NativeShadowDepthTargetContext& c) {
    const auto& b=c.bindings;void* renderer=b.renderer_00f8d394;
    entry(renderer,0x00d5f0a8,b.renderer_profile_00d5f0a8,0x104,0x00b1ff50);
    return get_native_compiler_renderer_capabilities_00b1ff50(renderer);
}
bool check_format(NativeShadowDepthTargetContext& c,U format) {
    const auto& b=c.bindings;void* renderer=b.renderer_00f8d394;
    entry(renderer,0x00d5f0a8,b.renderer_profile_00d5f0a8,0xf8,0x00b21ec0);
    return check_native_renderer_device_format_00b21ec0(renderer,0x16,0x100,3,format);
}
void release_child(void* owner,U offset,void* captured,NativeShadowDepthTargetContext& c,
    NativeShadowDepthTargetOperation& a,U site) {
    if(!captured)return;
    a.captured_child=captured;a.native_site=site;
    if(a.captured_decrement(static_cast<volatile long*>(at(captured,4)))==0)
        dispatch_native_render_resource_zero_terminal(captured,c.bindings.terminals,c.bindings.direct);
    put(owner,offset,0);
}
void destroy_target(void* owner,NativeShadowDepthTargetContext& c,NativeShadowDepthTargetOperation& a) {
    put(owner,0,0x00d5b5e8);
    void* const first=child(owner,0x14);
    a.captured_decrement=c.bindings.terminals.textures.decrement_iat_00ce2220;
    require(a.captured_decrement!=nullptr,"shadow target requires the actual current decrement import");
    a.exception_state=0;
    release_child(owner,0x14,first,c,a,0x00a9002d);
    release_child(owner,0x10,child(owner,0x10),c,a,0x00a90049);
    release_child(owner,0x1c,child(owner,0x1c),c,a,0x00a90065);
    release_child(owner,0x18,child(owner,0x18),c,a,0x00a90081);
    a.exception_state=0xffffffffu;a.native_site=0x00a9009c;extents_destroy(owner,c,a);
}
} // namespace

NativeShadowDepthTargetOperation::NativeShadowDepthTargetOperation(NativeShadowDepthTargetCall k,void* p) noexcept
    :call(k),receiver(p) {}
NativeShadowDepthTargetOperation::~NativeShadowDepthTargetOperation() {
    if(phase!=Phase::complete)std::terminate();
}
struct NativeShadowDepthTargetContext::Impl {std::vector<std::unique_ptr<NativeShadowDepthTargetOperation>> operations;};
NativeShadowDepthTargetContext::NativeShadowDepthTargetContext(NativeShadowDepthTargetBindings b)
    :bindings(b),impl_(std::make_unique<Impl>()) {
    require(&b.textures==&b.direct.holders.textures&&&b.surfaces==&b.direct.holders.levels
        && &b.terminals.textures.actual_owners==&b.direct.actual_owners
        && static_cast<const volatile void*>(&b.renderer_00f8d394)
            ==static_cast<const volatile void*>(&b.textures.construction.owners.renderer_notification.actual_renderer_00f8d394)
        && b.texture_profile_00d61948==b.terminals.textures.actual_profile_00d61948,
        "shadow target requires the same retained renderer and native producer/terminal domains");
}
NativeShadowDepthTargetContext::~NativeShadowDepthTargetContext()=default;
const NativeShadowDepthTargetOperation* NativeShadowDepthTargetContext::operation(std::size_t i) const noexcept {
    return i<impl_->operations.size()?impl_->operations[i].get():nullptr;
}
std::size_t NativeShadowDepthTargetContext::operation_count() const noexcept {return impl_->operations.size();}
NativeShadowDepthTargetOperation& NativeShadowDepthTargetContext::begin(NativeShadowDepthTargetCall kind,void* owner) {
    require(owner!=nullptr,"shadow target entry requires actual readable owner storage");
    // Reserve BEFORE allocating a running frame: vector growth failure must
    // not destroy an unattached running record with no native effects yet.
    impl_->operations.reserve(impl_->operations.size()+1u);
    auto record=std::make_unique<NativeShadowDepthTargetOperation>(kind,owner);
    auto& result=*record;impl_->operations.push_back(std::move(record));return result;
}
void* construct_native_shadow_target_singleton_00a8a820(void* owner,NativeShadowDepthTargetContext& c) {
    perform(c,NativeShadowDepthTargetCall::base_construct,owner,[&](auto& a){base_construct(owner,c,a);});return owner;
}
void destroy_native_shadow_target_singleton_00a8a8c0(void* owner,NativeShadowDepthTargetContext& c) {
    perform(c,NativeShadowDepthTargetCall::base_destroy,owner,[&](auto& a){base_destroy(owner,c,a);});
}
void* construct_native_shadow_target_extents_00a8a980(void* owner,
    const volatile NativeShadowDepthTargetDimensions& input,NativeShadowDepthTargetContext& c) {
    perform(c,NativeShadowDepthTargetCall::extents_construct,owner,[&](auto& a){extents_construct(owner,input,c,a);});return owner;
}
void destroy_native_shadow_target_extents_00a8a9d0(void* owner,NativeShadowDepthTargetContext& c) {
    perform(c,NativeShadowDepthTargetCall::extents_destroy,owner,[&](auto& a){extents_destroy(owner,c,a);});
}
NativeShadowDepthTargetStorage* construct_native_shadow_depth_target_00a8fe30(void* owner,
    const volatile NativeShadowDepthTargetDimensions& input,NativeShadowDepthTargetContext& c) {
    perform(c,NativeShadowDepthTargetCall::construct,owner,[&](auto& a){
        ::new(owner) NativeShadowDepthTargetStorage;
        a.saved_dimensions.height=input.height;a.saved_dimensions.width=input.width;
        a.native_site=0x00a8fe5a;extents_construct(owner,a.saved_dimensions,c,a);
        put(owner,0,0x00d5b5e8);
        for(U offset:{0x10u,0x14u,0x18u,0x1cu})put(owner,offset,0);
        byte(owner,0x20,0);byte(owner,0x0c,0);byte(owner,0x0d,0);put(owner,0x24,0);put(owner,0x28,0);
        a.exception_state=0;a.native_site=0x00a8fe94;
        if(word(capabilities(c),0x28)<0x200)return;
        a.native_site=0x00a8febb;
        if(check_format(c,0x36314644)) {put(owner,0x24,0x17);put(owner,0x28,0x36314644);byte(owner,0x0e,0);return;}
        a.native_site=0x00a8ff00;
        if(check_format(c,0x50)) {put(owner,0x24,0x17);put(owner,0x28,0x50);byte(owner,0x0e,1);}
    });return static_cast<NativeShadowDepthTargetStorage*>(owner);
}
void create_native_shadow_depth_target_00a8ff30(NativeShadowDepthTargetStorage& target,
    std::uint8_t enabled,NativeShadowDepthTargetContext& c) {
    void* owner=&target;
    perform(c,NativeShadowDepthTargetCall::create,owner,[&](auto& a){
        if(*static_cast<const volatile std::uint8_t*>(at(owner,0x20)))return;
        U height=word(owner,8);U width=word(owner,4);byte(owner,0x0c,enabled);
        if(!enabled)width=height=8;
        byte(owner,0x0d,0);a.native_site=0x00a8ff69;
        if(word(capabilities(c),0x28)>=0x200) {
            const U color_format=word(owner,0x24);
            if(color_format) {
                const auto& b=c.bindings;void* renderer=b.renderer_00f8d394;
                entry(renderer,0x00d5f0a8,b.renderer_profile_00d5f0a8,0x88,0x00b2a070);
                a.color_arguments={width,height,1,color_format,0x10};a.native_site=0x00a8ff90;
                void* color=create_native_runtime_texture_2d_00b2a070(renderer,a.color_arguments,b.textures,a.color_texture);
                put(owner,0x10,reinterpret_cast<U>(color));
                entry(color,0x00d61948,b.texture_profile_00d61948,0x30,0x00b3fd80);
                a.native_site=0x00a8ffa0;
                auto* surface=get_native_texture_surface_00b3fd80(color,0,0,b.surfaces,a.color_surface);
                put(owner,0x14,reinterpret_cast<U>(surface));
                const U depth_format=word(owner,0x28);renderer=b.renderer_00f8d394;
                entry(renderer,0x00d5f0a8,b.renderer_profile_00d5f0a8,0x88,0x00b2a070);
                a.depth_arguments={width,height,1,depth_format,0x100};a.native_site=0x00a8ffc0;
                void* depth=create_native_runtime_texture_2d_00b2a070(renderer,a.depth_arguments,b.textures,a.depth_texture);
                put(owner,0x18,reinterpret_cast<U>(depth));
                entry(depth,0x00d61948,b.texture_profile_00d61948,0x30,0x00b3fd80);
                a.native_site=0x00a8ffd0;
                surface=get_native_texture_surface_00b3fd80(depth,0,0,b.surfaces,a.depth_surface);
                put(owner,0x1c,reinterpret_cast<U>(surface));byte(owner,0x0d,1);
            }
        }
        byte(owner,0x20,1);
    });
}
void destroy_native_shadow_depth_target_00a8fff0(NativeShadowDepthTargetStorage& owner,NativeShadowDepthTargetContext& c) {
    perform(c,NativeShadowDepthTargetCall::destroy,&owner,[&](auto& a){destroy_target(&owner,c,a);});
}
void* delete_native_shadow_depth_target_00a900c0(void* owner,U flags,NativeShadowDepthTargetContext& c) {
    perform(c,NativeShadowDepthTargetCall::scalar_delete,owner,[&](auto& a){
        a.native_site=0x00a900c3;destroy_target(owner,c,a);
        if(flags&1u) {a.native_site=0x00a900d0;singleton_lifetime_free(owner);}
    });return owner;
}
NativeShadowDepthTargetFields view_native_shadow_depth_target(NativeShadowDepthTargetStorage& t) noexcept {
    return {&t,t.profile_00,t.width_04,t.height_08,t.enabled_0c,t.created_0d,t.color_texture_10,t.depth_texture_18};
}
} // namespace bsp
