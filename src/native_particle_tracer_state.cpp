#include "bsp/native_particle_tracer_state.hpp"
#include "bsp/native_particle_emission_state.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_particle_object_state.hpp"
#include "bsp/native_particle_preparation.hpp"
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_point_light_pool.hpp"
#include "bsp/native_point_light_owner.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_camera_world.hpp"
#include "bsp/native_hardware_layout_factory.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/random_threads.hpp"
#include <cstring>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle Tracer state requires MSVC Win32.
#endif
namespace bsp {
namespace {
template<class T> T read(const void* p, std::uint32_t n=0) noexcept {
    return *reinterpret_cast<const volatile T*>(static_cast<const std::byte*>(p)+n);
}
template<class T> void write(void* p, std::uint32_t n,T value) noexcept {
    *reinterpret_cast<volatile T*>(static_cast<std::byte*>(p)+n)=value;
}
void* at(void* p,std::uint32_t n) noexcept {return static_cast<std::byte*>(p)+n;}
struct PopulationGuard {
    TrackedCriticalSection* section;
    explicit PopulationGuard(TrackedCriticalSection* p):section(p) {
        if(section) {
            EnterCriticalSection(&section->native);
            write(section,0x18,read<std::uint32_t>(section,0x18)+1u);
        }
    }
    ~PopulationGuard() {
        if(section) {
            write(section,0x18,read<std::uint32_t>(section,0x18)-1u);
            LeaveCriticalSection(&section->native);
        }
    }
};
std::uint32_t slot(const NativeParticleTracerStateAccess& a,void* p,std::uint32_t n) noexcept {
    return a.application->table_slot(read<std::uint32_t>(p),n);
}
static_assert(offsetof(NativeParticleTracerStateAccess,common)==0);
static_assert(offsetof(NativeParticleTracerStateAccess,strings)==28);
static_assert(offsetof(NativeParticleTracerStateAccess,phase_max_00ce3d9c)==88);
static_assert(offsetof(NativeParticleTracerStateAccess,width_default_00d7a2f0)==92);
static_assert(offsetof(NativeParticleTracerStateAccess,segment_life_default_00d7a248)==96);
static_assert(offsetof(NativeParticleTracerStateAccess,tile_default_00ce38b8)==100);
static_assert(offsetof(NativeParticleTracerStateAccess,color_scale_00ce4b48)==104);
const char light_prefix[]="dynamic_light_";
const char traceline_name[]="Traceline";
LONG __stdcall increment_bridge(volatile LONG* p) {return InterlockedIncrement(p);}
LONG __stdcall decrement_bridge(volatile LONG* p) {return InterlockedDecrement(p);}

std::uint32_t __fastcall random_bridge(RandomStream s,const NativeParticleTracerStateAccess* a) {
    return a->common.random->next_00bd2fc0(s);
}
float __fastcall range_bridge(RandomStream s,const NativeParticleTracerStateAccess* a,float lo,float hi) {
    return native_particle_random_range_00bd2f10(s,a->unit_random,lo,hi);
}
void* __fastcall light_allocate_bridge(void*,const NativeParticleTracerStateAccess* a) {
    return a->point_lights->pool_0109011c.allocate_raw_slot_00b7b810();
}
void* __fastcall light_construct_bridge(void* p,const NativeParticleTracerStateAccess* a,const void* name) {
    auto storage=construct_native_point_light_00b7c710(p,NativePointLightPool::slot_bytes,
        *static_cast<const NativeString*>(name),a->point_lights->nodes.strings);
    adopt_constructed_native_point_light(*a->point_lights,storage);
    return p;
}
void* __fastcall string_bridge(void* p,const NativeParticleTracerStateAccess* a,const char* s) {
    return construct_native_string_cstring_0041e870(p,s,*a->strings);
}
void* __fastcall concat_bridge(const void* p,const NativeParticleTracerStateAccess* a,void* out,const void* r) {
    return concatenate_native_string_headers_004261a0(p,out,r,*a->strings);
}
NativeStringStorage* __fastcall storage_bridge(void*,const NativeParticleTracerStateAccess* a) {
    return a->strings;
}
void __fastcall string_release_bridge(NativeStringStorage* s,void*,char* p,std::uint32_t n,std::uint32_t) {
    s->release(p,n);
}
void* __fastcall scalar_allocate_bridge(void*,const NativeParticleTracerStateAccess* a,std::size_t n) {
    return a->allocate_00bf681b(n);
}
void* __fastcall array_allocate_bridge(void*,const NativeParticleTracerStateAccess* a,std::size_t n) {
    return a->allocate_00bf55be(n);
}
void __fastcall free_bridge(void*,const NativeParticleTracerStateAccess* a,void* p) {
    a->free_00bf6989(p);
}
void* __fastcall resources_bridge(void*,const NativeParticleTracerStateAccess* a) {
    return get_native_tracer_resources_00b0b5d0(*a);
}
void* __fastcall node_allocate_bridge(void*,const NativeParticleTracerStateAccess* a) {
    return allocate_native_traceline_slot_00af3430(*a);
}
void* __fastcall node_construct_bridge(void* p,const NativeParticleTracerStateAccess* a,const void* s) {
    return construct_native_traceline_00858260(p,s,*a);
}
std::uint32_t __fastcall capture_bridge(std::uint32_t p,const NativeParticleTracerStateAccess* a,std::uint32_t n) noexcept {
    return a->application->table_slot(p,n);
}
void __fastcall zero_owner_bridge(void* p,const NativeParticleTracerStateAccess* a,std::uint32_t target) {
    auto& r=a->retained_owners->resolve_actual(p);
    if (&r.reference_count!=at(p,4) || slot(*a,p,0)!=target)
        throw std::logic_error("Tracer retained owner must use its current actual count and target");
    r.release_zero_references();
}
void __fastcall node_attach_bridge(void* p,const NativeParticleTracerStateAccess* a,std::uint32_t target,void* data,void* root) {
    a->application->node_virtual5c(p,target,data,root);
}
void __fastcall initial_update_bridge(void* p,const NativeParticleTracerStateAccess* a,std::uint32_t target,
    void* state,float age,std::uint32_t word,const void* matrix,float delta) {
    a->application->definition_virtual28(p,target,state,age,word,matrix,delta);
}
} // namespace

std::uint8_t cleanup_native_particle_tracer_state_00b0a840(void* state,std::uint32_t force,
    const NativeParticleTracerStateAccess& a) {
    auto* owner=get_native_particle_population_lock_0072b740(*a.actual_manager_01090aa0,*a.actual_lock_0108ff50);
    const PopulationGuard guard(read<TrackedCriticalSection*>(owner,4));
    void* node=read<void*>(state,0x30);
    if(read<std::uint32_t>(node,0x190)!=0 && static_cast<std::uint8_t>(force)==0) {
        write<std::uint8_t>(node,0x195,1);
        write<std::uint8_t>(node,0x1a8,1);
        return 0;
    }
    if(void* payload=read<void*>(state,0x34)) {
        const auto target=slot(a,payload,0);
        a.application->payload_virtual00(payload,target,1);
        write<void*>(state,0x34,nullptr);
    }
    if((node=read<void*>(state,0x30))!=nullptr) {
        auto* lifetime=a.nodes->find_actual_node(reinterpret_cast<std::uint32_t>(node));
        if(!lifetime) throw std::logic_error("Traceline requires its existing canonical node lifetime");
        unlink_and_release_render_model_00b6dfa0(*lifetime);
        write<void*>(state,0x30,nullptr);
    }
    return 1;
}

void* construct_native_tracer_resources_00b0ab90(void* p,const NativeParticleTracerStateAccess& a) {
    write<std::uint32_t>(p,0,0x00d5e074);
    std::uint32_t name[2]{};
    resize_native_string_header_0041dd40(name,*a.strings,0x17,true);
    if(auto* data=read<void*>(name,4)) {
        std::memcpy(data,"pf43uf43ccuf44uf41.mvfm",read<std::uint32_t>(name)+1u);
    }
    try {
        void* renderer=*a.actual_renderer_00f8d394;
        const auto target=slot(a,renderer,0x38);
        write(p,4,a.application->renderer_virtual38(renderer,target,name));
    } catch(...) {destroy_native_string_header_0041dd20(name,*a.strings);throw;}
    destroy_native_string_header_0041dd20(name,*a.strings);
    std::uint32_t key[5];
    key[0]=read<std::uint32_t>(p,4);
    key[1]=a.layout_key_residue[0];key[2]=a.layout_key_residue[1];key[3]=a.layout_key_residue[2];
    key[4]=1;
    void* renderer=*a.actual_renderer_00f8d394;
    const auto target=slot(a,renderer,0x40);
    void* layout=target==0x00b2f710
        ? get_or_create_native_hardware_layout_00b2f710(key,*a.hardware_layouts)
        : a.application->renderer_virtual40(renderer,target,key);
    write(p,8,layout);
    write(p,0xc,find_native_particle_atlas_item_00aefb20(*a.actual_atlas_00f8c26c,
        "tr_glow_add.dds",*a.strings,a.null_atlas_pattern_00e17bf0));
    write(p,0x10,find_native_particle_atlas_item_00aefb20(*a.actual_atlas_00f8c26c,
        "tr_bullet.dds",*a.strings,a.null_atlas_pattern_00e17bf0));
    write(p,0x14,find_native_particle_atlas_item_00aefb20(*a.actual_atlas_00f8c26c,
        "cloud_snake.dds",*a.strings,a.null_atlas_pattern_00e17bf0));
    return p;
}
void* get_native_tracer_resources_00b0b5d0(const NativeParticleTracerStateAccess& a) {
    if(void* first=*a.actual_resources_00f8d38c) return first;
    void* manager=get_native_singleton_manager_00415350(*a.actual_manager_01090aa0);
    {
        const PopulationGuard guard(read<TrackedCriticalSection*>(manager,0x10));
        if(!*a.actual_resources_00f8d38c) {
            void* const raw=a.allocate_00bf681b(0x18);
            void* result=nullptr;
            try {if(raw)result=construct_native_tracer_resources_00b0ab90(raw,a);}
            catch(...) {a.free_00bf6989(raw);throw;}
            *a.actual_resources_00f8d38c=result;
            manager=get_native_singleton_manager_00415350(*a.actual_manager_01090aa0);
            register_native_singleton_object_00bd0c30(manager,nullptr,*a.actual_resources_00f8d38c);
        }
    }
    return *a.actual_resources_00f8d38c;
}
void* allocate_native_traceline_slot_00af3430(const NativeParticleTracerStateAccess& a) {
    return a.application->call_00af32f0(a.actual_pool_00f8c288);
}
void* construct_native_traceline_00858260(void* raw,const void* name,const NativeParticleTracerStateAccess& a) {
    auto& base=a.application->prepared_model(raw);
    if(&base.storage.node!=raw || base.environment.actual_names!=a.strings ||
        &base.environment.nodes.attachments!=a.nodes)
        throw std::logic_error("Traceline must use its same prepared model, actual strings and node domain");
    construct_native_model_00b75030(base,*static_cast<const NativeString*>(name));
    // The scalar read is MOVSS: copy its bits, including signaling NaN.
    void* child=read<void*>(raw,0x34);
    const auto one=read<std::uint32_t>(const_cast<const float*>(a.common.one_00d7a24c));
    write<std::uint32_t>(raw,0,0x00d0c8c8);
    write<std::uint32_t>(raw,0x188,0);
    write<std::uint8_t>(raw,0x194,0);write<std::uint8_t>(raw,0x195,0);
    write<std::uint32_t>(raw,0x198,0);write<std::uint32_t>(raw,0x1a4,0);
    write<std::uint8_t>(raw,0x1a8,0);write(raw,0x19c,one);
    write<std::uint32_t>(raw,0x1a0,0);write<std::uint32_t>(raw,0x1ac,0);
    write<std::uint32_t>(raw,0x1b4,0);write<std::uint32_t>(raw,0x1b0,0);
    write<std::uint32_t>(raw,0x48,1);
    while(child) {
        set_native_node_hierarchy_mask_007099c0(static_cast<NativeNodeStorage*>(child),nullptr,1);
        child=read<void*>(child,0x3c);
    }
    return raw;
}
__declspec(naked) const void* __fastcall native_particle_record_matrix_00afda80(const void*) {
    __asm {
        mov eax,dword ptr [ecx+0xa0]
        cmp dword ptr [eax+0x70],0
        jz attached
        lea eax,[ecx+0x60]
        ret
    attached:
        push esi
        mov esi,dword ptr [ecx+0xa4]
        cmp byte ptr [esi+0x1b0],0
        jz world
        mov ecx,esi
        pop esi
        jmp get_native_node_local_matrix_00b6db60
    world:
        test byte ptr [esi+0x5c],2
        jnz ready
        mov ecx,esi
        call refresh_native_camera_world_00b6db70
    ready:
        lea eax,[esi+0xf0]
        pop esi
        ret
    }
}

// INITIALIZER_ASSEMBLY
__declspec(naked) void __fastcall initialize_native_particle_tracer_state_00b0b6a0(
    void*,const NativeParticleTracerStateAccess*,void*,const void*) {
    __asm {
        push edx // added access slot; preserve native locals and all spills
        sub esp,0xc // native FH3 record space; host entry does not register native FH3
        sub esp, 0x7c // 00b0b6b5
        push ebx // 00b0b6b8
        push ebp // 00b0b6b9
        mov ebp, dword ptr [esp + 0x98] // 00b0b6ba
        push esi // 00b0b6c1
        xor ebx, ebx // 00b0b6c2
        mov esi, ecx // 00b0b6c4
        mov dword ptr [esp + 0x10], ebx // 00b0b6c6
        cmp byte ptr [esi + 0x64], bl // 00b0b6ca
        push edi // 00b0b6cd
        je l_00b0b7af // 00b0b6ce
        mov ecx, 0x1fc // 00b0b6d4
        mov edx,dword ptr [esp+0x98] // borrowed access
        call light_allocate_bridge // 00b0b6d9
        mov edi, eax // 00b0b6de
        mov dword ptr [esp + 0x30], edi // 00b0b6e0
        test edi, edi // 00b0b6e4
        mov dword ptr [esp + 0x94], ebx // 00b0b6e6
        je l_00b0b73e // 00b0b6ed
        push offset light_prefix // 00b0b6ef
        lea ecx, [esp + 0x48] // 00b0b6f4
        mov edx,dword ptr [esp+0x9c] // borrowed access
        call string_bridge // 00b0b6f8
        mov edx, dword ptr [esi + 0x14] // 00b0b6fd
        add edx, 8 // 00b0b700
        push edx // 00b0b703
        lea ecx, [esp + 0x40] // 00b0b704
        push ecx // 00b0b708
        mov ecx, eax // 00b0b709
        mov byte ptr [esp + 0x9c], 1 // 00b0b70b
        mov dword ptr [esp + 0x1c], 1 // 00b0b713
        mov edx,dword ptr [esp+0xa0] // borrowed access
        call concat_bridge // 00b0b71b
        mov ebx, 3 // 00b0b720
        push eax // 00b0b725
        mov ecx, edi // 00b0b726
        mov dword ptr [esp + 0x98], 2 // 00b0b728
        mov dword ptr [esp + 0x18], ebx // 00b0b733
        mov edx,dword ptr [esp+0x9c] // borrowed access
        call light_construct_bridge // 00b0b737
        jmp l_00b0b740 // 00b0b73c
    l_00b0b73e:
        xor eax, eax // 00b0b73e
    l_00b0b740:
        test bl, 2 // 00b0b740
        mov dword ptr [ebp + 0x60], eax // 00b0b743
        mov dword ptr [esp + 0x94], 3 // 00b0b746
        je l_00b0b779 // 00b0b751
        mov ecx, dword ptr [esp + 0x40] // 00b0b753
        and ebx, 0xfffffffd // 00b0b757
        test ecx, ecx // 00b0b75a
        mov dword ptr [esp + 0x14], ebx // 00b0b75c
        je l_00b0b779 // 00b0b760
        mov eax, dword ptr [esp + 0x3c] // 00b0b762
        push 1 // 00b0b766
        add eax, 1 // 00b0b768
        push eax // 00b0b76b
        push ecx // 00b0b76c
        mov edx,dword ptr [esp+0xa4] // borrowed access
        call storage_bridge // 00b0b76d
        mov ecx, eax // 00b0b772
        call string_release_bridge // 00b0b774
    l_00b0b779:
        test bl, 1 // 00b0b779
        mov dword ptr [esp + 0x94], 0xffffffff // 00b0b77c
        je l_00b0b7af // 00b0b787
        mov ecx, dword ptr [esp + 0x48] // 00b0b789
        and ebx, 0xfffffffe // 00b0b78d
        test ecx, ecx // 00b0b790
        mov dword ptr [esp + 0x14], ebx // 00b0b792
        je l_00b0b7af // 00b0b796
        mov eax, dword ptr [esp + 0x44] // 00b0b798
        push 1 // 00b0b79c
        add eax, 1 // 00b0b79e
        push eax // 00b0b7a1
        push ecx // 00b0b7a2
        mov edx,dword ptr [esp+0xa4] // borrowed access
        call storage_bridge // 00b0b7a3
        mov ecx, eax // 00b0b7a8
        call string_release_bridge // 00b0b7aa
    l_00b0b7af:
        mov eax, dword ptr [esi + 0x1c] // 00b0b7af
        movss xmm0, dword ptr [eax] // 00b0b7b2
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0b7b6
        pop edx
        lahf  // 00b0b7bd
        test ah, 0x44 // 00b0b7be
        movss dword ptr [esp + 0x2c], xmm0 // 00b0b7c1
        jp l_00b0b7d9 // 00b0b7c7
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0b7c9
        pop edx
        movss dword ptr [esp + 0x24], xmm0 // 00b0b7d1
        jmp l_00b0b7fc // 00b0b7d7
    l_00b0b7d9:
        xor ecx, ecx // 00b0b7d9
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0b7db
        mov dword ptr [esp + 0x30], eax // 00b0b7e0
        fild dword ptr [esp + 0x30] // 00b0b7e4
        fmul dword ptr [esp + 0x2c] // 00b0b7e8
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0b7ec
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0b7f2
        pop edx
        fstp dword ptr [esp + 0x24] // 00b0b7f8
    l_00b0b7fc:
        mov ecx, dword ptr [esi + 0x1c] // 00b0b7fc
        fldz  // 00b0b7ff
        xor edi, edi // 00b0b801
        cmp word ptr [ecx + 0xa], di // 00b0b803
        jne l_00b0b818 // 00b0b807
        movss xmm0, dword ptr [ecx + 4] // 00b0b809
        fstp st(0) // 00b0b80e
        movss dword ptr [esp + 0x18], xmm0 // 00b0b810
        jmp l_00b0b833 // 00b0b816
    l_00b0b818:
        cmp word ptr [ecx + 0xa], 1 // 00b0b818
        push ecx // 00b0b81d
        fstp dword ptr [esp] // 00b0b81e
        jne l_00b0b82a // 00b0b821
        call evaluate_native_particle_linear_curve_00affa70 // 00b0b823
        jmp l_00b0b82f // 00b0b828
    l_00b0b82a:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0b82a
    l_00b0b82f:
        fstp dword ptr [esp + 0x18] // 00b0b82f
    l_00b0b833:
        fld dword ptr [esp + 0x18] // 00b0b833
        sub esp, 8 // 00b0b837
        fmul dword ptr [esp + 0x2c] // 00b0b83a
        xor ecx, ecx // 00b0b83e
        fstp dword ptr [ebp + 0x44] // 00b0b840
        push edx
        mov edx,dword ptr [esp+0xa4]
        mov edx,dword ptr [edx+88] // current native global 00ce3d9c
        fld dword ptr [edx] // 00b0b843
        pop edx
        fstp dword ptr [esp + 4] // 00b0b849
        fldz  // 00b0b84d
        fstp dword ptr [esp] // 00b0b84f
        mov edx,dword ptr [esp+0xa0] // borrowed access
        call range_bridge // 00b0b852
        fstp dword ptr [esp + 0x30] // 00b0b857
        fld dword ptr [esp + 0x30] // 00b0b85b
        fstp dword ptr [ebp + 0x50] // 00b0b85f
        mov eax, dword ptr [esi + 0x2c] // 00b0b862
        movss xmm0, dword ptr [eax] // 00b0b865
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0b869
        pop edx
        lahf  // 00b0b870
        test ah, 0x44 // 00b0b871
        movss dword ptr [esp + 0x2c], xmm0 // 00b0b874
        jp l_00b0b88c // 00b0b87a
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0b87c
        pop edx
        movss dword ptr [esp + 0x24], xmm0 // 00b0b884
        jmp l_00b0b8af // 00b0b88a
    l_00b0b88c:
        xor ecx, ecx // 00b0b88c
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0b88e
        mov dword ptr [esp + 0x30], eax // 00b0b893
        fild dword ptr [esp + 0x30] // 00b0b897
        fmul dword ptr [esp + 0x2c] // 00b0b89b
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0b89f
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0b8a5
        pop edx
        fstp dword ptr [esp + 0x24] // 00b0b8ab
    l_00b0b8af:
        fld dword ptr [ebp + 0x18] // 00b0b8af
        fld dword ptr [esp + 0x24] // 00b0b8b2
        fld st(0) // 00b0b8b6
        fmulp st(2), st(0) // 00b0b8b8
        fxch st(1) // 00b0b8ba
        fstp dword ptr [ebp + 0x18] // 00b0b8bc
        fld dword ptr [ebp + 0x1c] // 00b0b8bf
        fmul st(0), st(1) // 00b0b8c2
        fstp dword ptr [ebp + 0x1c] // 00b0b8c4
        fmul dword ptr [ebp + 0x20] // 00b0b8c7
        fstp dword ptr [ebp + 0x20] // 00b0b8ca
        mov eax, dword ptr [esi + 0x30] // 00b0b8cd
        movss xmm0, dword ptr [eax] // 00b0b8d0
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0b8d4
        pop edx
        lahf  // 00b0b8db
        test ah, 0x44 // 00b0b8dc
        movss dword ptr [esp + 0x2c], xmm0 // 00b0b8df
        jp l_00b0b8f1 // 00b0b8e5
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0b8e7
        pop edx
        jmp l_00b0b91a // 00b0b8ef
    l_00b0b8f1:
        xor ecx, ecx // 00b0b8f1
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0b8f3
        mov dword ptr [esp + 0x30], eax // 00b0b8f8
        fild dword ptr [esp + 0x30] // 00b0b8fc
        fmul dword ptr [esp + 0x2c] // 00b0b900
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0b904
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0b90a
        pop edx
        fstp dword ptr [esp + 0x30] // 00b0b910
        movss xmm0, dword ptr [esp + 0x30] // 00b0b914
    l_00b0b91a:
        movss dword ptr [ebp + 0x48], xmm0 // 00b0b91a
        mov eax, dword ptr [esi + 0x34] // 00b0b91f
        movss xmm0, dword ptr [eax] // 00b0b922
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0b926
        pop edx
        lahf  // 00b0b92d
        test ah, 0x44 // 00b0b92e
        movss dword ptr [esp + 0x2c], xmm0 // 00b0b931
        jp l_00b0b943 // 00b0b937
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0b939
        pop edx
        jmp l_00b0b96c // 00b0b941
    l_00b0b943:
        xor ecx, ecx // 00b0b943
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0b945
        mov dword ptr [esp + 0x30], eax // 00b0b94a
        fild dword ptr [esp + 0x30] // 00b0b94e
        fmul dword ptr [esp + 0x2c] // 00b0b952
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0b956
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0b95c
        pop edx
        fstp dword ptr [esp + 0x30] // 00b0b962
        movss xmm0, dword ptr [esp + 0x30] // 00b0b966
    l_00b0b96c:
        push 0x80 // 00b0b96c
        movss dword ptr [ebp + 0x4c], xmm0 // 00b0b971
        mov edx,dword ptr [esp+0x9c] // borrowed access
        call scalar_allocate_bridge // 00b0b976
        cmp eax, edi // 00b0b97e
        je l_00b0b9ef // 00b0b980
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0b982
        pop edx
        mov dword ptr [eax], 0xd0d4a4 // 00b0b98a
        mov dword ptr [eax + 0x4c], edi // 00b0b990
        mov dword ptr [eax + 0x50], edi // 00b0b993
        mov dword ptr [eax + 0x54], edi // 00b0b996
        movss dword ptr [eax + 0x10], xmm0 // 00b0b999
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+92] // current native global 00d7a2f0
        movss xmm0, dword ptr [edx] // 00b0b99e
        pop edx
        movss dword ptr [eax + 0x14], xmm0 // 00b0b9a6
        xorps xmm0, xmm0 // 00b0b9ab
        movss dword ptr [eax + 0x18], xmm0 // 00b0b9ae
        movss dword ptr [eax + 0x1c], xmm0 // 00b0b9b3
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+96] // current native global 00d7a248
        movss xmm0, dword ptr [edx] // 00b0b9b8
        pop edx
        movss dword ptr [eax + 0x28], xmm0 // 00b0b9c0
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+100] // current native global 00ce38b8
        movss xmm0, dword ptr [edx] // 00b0b9c5
        pop edx
        mov byte ptr [eax + 4], 0 // 00b0b9cd
        mov byte ptr [eax + 5], 1 // 00b0b9d1
        mov dword ptr [eax + 8], edi // 00b0b9d5
        mov dword ptr [eax + 0xc], edi // 00b0b9d8
        mov dword ptr [eax + 0x20], 3 // 00b0b9db
        movss dword ptr [eax + 0x2c], xmm0 // 00b0b9e2
        mov dword ptr [eax + 0x40], edi // 00b0b9e7
        mov dword ptr [eax + 0x44], edi // 00b0b9ea
        jmp l_00b0b9f1 // 00b0b9ed
    l_00b0b9ef:
        xor eax, eax // 00b0b9ef
    l_00b0b9f1:
        mov dword ptr [ebp + 0x34], eax // 00b0b9f1
        mov ecx, dword ptr [esi + 0xd8] // 00b0b9f4
        movzx eax, word ptr [ecx + 0xa] // 00b0b9fa
        cmp ax, di // 00b0b9fe
        jne l_00b0ba10 // 00b0ba01
        movss xmm0, dword ptr [ecx + 4] // 00b0ba03
        movss dword ptr [esp + 0x18], xmm0 // 00b0ba08
        jmp l_00b0ba2c // 00b0ba0e
    l_00b0ba10:
        cmp ax, 1 // 00b0ba10
        fldz  // 00b0ba14
        push ecx // 00b0ba16
        fstp dword ptr [esp] // 00b0ba17
        jne l_00b0ba23 // 00b0ba1a
        call evaluate_native_particle_linear_curve_00affa70 // 00b0ba1c
        jmp l_00b0ba28 // 00b0ba21
    l_00b0ba23:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0ba23
    l_00b0ba28:
        fstp dword ptr [esp + 0x18] // 00b0ba28
    l_00b0ba2c:
        mov eax, dword ptr [esi + 0xd8] // 00b0ba2c
        movss xmm0, dword ptr [eax] // 00b0ba32
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0ba36
        pop edx
        lahf  // 00b0ba3d
        test ah, 0x44 // 00b0ba3e
        movss dword ptr [esp + 0x2c], xmm0 // 00b0ba41
        jp l_00b0ba59 // 00b0ba47
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0ba49
        pop edx
        movss dword ptr [esp + 0x24], xmm0 // 00b0ba51
        jmp l_00b0ba7c // 00b0ba57
    l_00b0ba59:
        xor ecx, ecx // 00b0ba59
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0ba5b
        mov dword ptr [esp + 0x30], eax // 00b0ba60
        fild dword ptr [esp + 0x30] // 00b0ba64
        fmul dword ptr [esp + 0x2c] // 00b0ba68
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0ba6c
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0ba72
        pop edx
        fstp dword ptr [esp + 0x24] // 00b0ba78
    l_00b0ba7c:
        fld dword ptr [esp + 0x24] // 00b0ba7c
        mov edx, dword ptr [ebp + 0x34] // 00b0ba80
        fmul dword ptr [esp + 0x18] // 00b0ba83
        fstp dword ptr [esp + 0x30] // 00b0ba87
        fld dword ptr [esp + 0x30] // 00b0ba8b
        fstp dword ptr [edx + 0x14] // 00b0ba8f
        mov ecx, dword ptr [esi + 0xdc] // 00b0ba92
        movzx eax, word ptr [ecx + 0xa] // 00b0ba98
        cmp ax, di // 00b0ba9c
        jne l_00b0baae // 00b0ba9f
        movss xmm0, dword ptr [ecx + 4] // 00b0baa1
        movss dword ptr [esp + 0x18], xmm0 // 00b0baa6
        jmp l_00b0baca // 00b0baac
    l_00b0baae:
        cmp ax, 1 // 00b0baae
        fldz  // 00b0bab2
        push ecx // 00b0bab4
        fstp dword ptr [esp] // 00b0bab5
        jne l_00b0bac1 // 00b0bab8
        call evaluate_native_particle_linear_curve_00affa70 // 00b0baba
        jmp l_00b0bac6 // 00b0babf
    l_00b0bac1:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0bac1
    l_00b0bac6:
        fstp dword ptr [esp + 0x18] // 00b0bac6
    l_00b0baca:
        mov eax, dword ptr [esi + 0xdc] // 00b0baca
        movss xmm0, dword ptr [eax] // 00b0bad0
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0bad4
        pop edx
        lahf  // 00b0badb
        test ah, 0x44 // 00b0badc
        movss dword ptr [esp + 0x2c], xmm0 // 00b0badf
        jp l_00b0baf7 // 00b0bae5
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0bae7
        pop edx
        movss dword ptr [esp + 0x24], xmm0 // 00b0baef
        jmp l_00b0bb1a // 00b0baf5
    l_00b0baf7:
        xor ecx, ecx // 00b0baf7
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0baf9
        mov dword ptr [esp + 0x30], eax // 00b0bafe
        fild dword ptr [esp + 0x30] // 00b0bb02
        fmul dword ptr [esp + 0x2c] // 00b0bb06
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0bb0a
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0bb10
        pop edx
        fstp dword ptr [esp + 0x24] // 00b0bb16
    l_00b0bb1a:
        fld dword ptr [esp + 0x24] // 00b0bb1a
        mov eax, dword ptr [ebp + 0x34] // 00b0bb1e
        fmul dword ptr [esp + 0x18] // 00b0bb21
        fstp dword ptr [esp + 0x30] // 00b0bb25
        fld dword ptr [esp + 0x30] // 00b0bb29
        fstp dword ptr [eax + 0x18] // 00b0bb2d
        mov ecx, dword ptr [esi + 0xe0] // 00b0bb30
        movzx eax, word ptr [ecx + 0xa] // 00b0bb36
        cmp ax, di // 00b0bb3a
        jne l_00b0bb4c // 00b0bb3d
        movss xmm0, dword ptr [ecx + 4] // 00b0bb3f
        movss dword ptr [esp + 0x18], xmm0 // 00b0bb44
        jmp l_00b0bb68 // 00b0bb4a
    l_00b0bb4c:
        cmp ax, 1 // 00b0bb4c
        fldz  // 00b0bb50
        push ecx // 00b0bb52
        fstp dword ptr [esp] // 00b0bb53
        jne l_00b0bb5f // 00b0bb56
        call evaluate_native_particle_linear_curve_00affa70 // 00b0bb58
        jmp l_00b0bb64 // 00b0bb5d
    l_00b0bb5f:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0bb5f
    l_00b0bb64:
        fstp dword ptr [esp + 0x18] // 00b0bb64
    l_00b0bb68:
        mov eax, dword ptr [esi + 0xe0] // 00b0bb68
        movss xmm0, dword ptr [eax] // 00b0bb6e
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0bb72
        pop edx
        lahf  // 00b0bb79
        test ah, 0x44 // 00b0bb7a
        movss dword ptr [esp + 0x2c], xmm0 // 00b0bb7d
        jp l_00b0bb95 // 00b0bb83
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0bb85
        pop edx
        movss dword ptr [esp + 0x24], xmm0 // 00b0bb8d
        jmp l_00b0bbb8 // 00b0bb93
    l_00b0bb95:
        xor ecx, ecx // 00b0bb95
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0bb97
        mov dword ptr [esp + 0x30], eax // 00b0bb9c
        fild dword ptr [esp + 0x30] // 00b0bba0
        fmul dword ptr [esp + 0x2c] // 00b0bba4
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0bba8
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0bbae
        pop edx
        fstp dword ptr [esp + 0x24] // 00b0bbb4
    l_00b0bbb8:
        fld dword ptr [esp + 0x24] // 00b0bbb8
        mov ecx, dword ptr [ebp + 0x34] // 00b0bbbc
        fmul dword ptr [esp + 0x18] // 00b0bbbf
        fstp dword ptr [esp + 0x30] // 00b0bbc3
        fld dword ptr [esp + 0x30] // 00b0bbc7
        fstp dword ptr [ecx + 0x1c] // 00b0bbcb
        cvttss2si edx, dword ptr [esi + 0x80] // 00b0bbce
        mov eax, dword ptr [ebp + 0x34] // 00b0bbd6
        mov dword ptr [eax + 0x20], edx // 00b0bbd9
        fld dword ptr [esi + 0x84] // 00b0bbdc
        mov ecx, dword ptr [ebp + 0x34] // 00b0bbe2
        fstp dword ptr [ecx + 0x24] // 00b0bbe5
        mov edx, dword ptr [ebp + 0x34] // 00b0bbe8
        fld dword ptr [esi + 0x88] // 00b0bbeb
        fstp dword ptr [edx + 0x28] // 00b0bbf1
        mov edx,dword ptr [esp+0x98] // borrowed access
        call resources_bridge // 00b0bbf4
        mov ecx, dword ptr [ebp + 0x34] // 00b0bbf9
        mov ebx, dword ptr [ecx + 0x40] // 00b0bbfc
        mov eax, dword ptr [eax + 4] // 00b0bbff
        mov edi,offset increment_bridge // 00b0bc02
        add ecx, 0x40 // 00b0bc08
        cmp ebx, eax // 00b0bc0b
        je l_00b0bc35 // 00b0bc0d
        test eax, eax // 00b0bc0f
        mov dword ptr [ecx], eax // 00b0bc11
        je l_00b0bc1b // 00b0bc13
        add eax, 4 // 00b0bc15
        push eax // 00b0bc18
        call edi // 00b0bc19
    l_00b0bc1b:
        test ebx, ebx // 00b0bc1b
        je l_00b0bc35 // 00b0bc1d
        lea eax, [ebx + 4] // 00b0bc1f
        push eax // 00b0bc22
        call decrement_bridge // 00b0bc23
        test eax, eax // 00b0bc29
        jne l_00b0bc35 // 00b0bc2b
        mov edx, dword ptr [ebx] // 00b0bc2d
        push edx
        push ecx
        mov ecx,edx
        mov edx,dword ptr [esp+0xa0]
        push 0x0
        call capture_bridge
        pop ecx
        pop edx
        mov ecx, ebx // 00b0bc31
        mov edx,dword ptr [esp+0x98] // borrowed access
        push eax // native captured target
        call zero_owner_bridge // 00b0bc33
    l_00b0bc35:
        mov edx,dword ptr [esp+0x98] // borrowed access
        call resources_bridge // 00b0bc35
        mov ecx, dword ptr [ebp + 0x34] // 00b0bc3a
        mov ebx, dword ptr [ecx + 0x44] // 00b0bc3d
        mov eax, dword ptr [eax + 8] // 00b0bc40
        add ecx, 0x44 // 00b0bc43
        cmp ebx, eax // 00b0bc46
        je l_00b0bc70 // 00b0bc48
        test eax, eax // 00b0bc4a
        mov dword ptr [ecx], eax // 00b0bc4c
        je l_00b0bc56 // 00b0bc4e
        add eax, 4 // 00b0bc50
        push eax // 00b0bc53
        call edi // 00b0bc54
    l_00b0bc56:
        test ebx, ebx // 00b0bc56
        je l_00b0bc70 // 00b0bc58
        lea ecx, [ebx + 4] // 00b0bc5a
        push ecx // 00b0bc5d
        call decrement_bridge // 00b0bc5e
        test eax, eax // 00b0bc64
        jne l_00b0bc70 // 00b0bc66
        mov edx, dword ptr [ebx] // 00b0bc68
        push edx
        push ecx
        mov ecx,edx
        mov edx,dword ptr [esp+0xa0]
        push 0x0
        call capture_bridge
        pop ecx
        pop edx
        mov ecx, ebx // 00b0bc6c
        mov edx,dword ptr [esp+0x98] // borrowed access
        push eax // native captured target
        call zero_owner_bridge // 00b0bc6e
    l_00b0bc70:
        mov ecx, dword ptr [esi + 0xbc] // 00b0bc70
        movzx eax, word ptr [ecx + 0xa] // 00b0bc76
        test ax, ax // 00b0bc7a
        jne l_00b0bc8c // 00b0bc7d
        movss xmm0, dword ptr [ecx + 4] // 00b0bc7f
        movss dword ptr [esp + 0x20], xmm0 // 00b0bc84
        jmp l_00b0bca8 // 00b0bc8a
    l_00b0bc8c:
        cmp ax, 1 // 00b0bc8c
        fldz  // 00b0bc90
        push ecx // 00b0bc92
        fstp dword ptr [esp] // 00b0bc93
        jne l_00b0bc9f // 00b0bc96
        call evaluate_native_particle_linear_curve_00affa70 // 00b0bc98
        jmp l_00b0bca4 // 00b0bc9d
    l_00b0bc9f:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0bc9f
    l_00b0bca4:
        fstp dword ptr [esp + 0x20] // 00b0bca4
    l_00b0bca8:
        mov ecx, dword ptr [esi + 0xb8] // 00b0bca8
        movzx eax, word ptr [ecx + 0xa] // 00b0bcae
        test ax, ax // 00b0bcb2
        jne l_00b0bcc4 // 00b0bcb5
        movss xmm0, dword ptr [ecx + 4] // 00b0bcb7
        movss dword ptr [esp + 0x1c], xmm0 // 00b0bcbc
        jmp l_00b0bce0 // 00b0bcc2
    l_00b0bcc4:
        cmp ax, 1 // 00b0bcc4
        fldz  // 00b0bcc8
        push ecx // 00b0bcca
        fstp dword ptr [esp] // 00b0bccb
        jne l_00b0bcd7 // 00b0bcce
        call evaluate_native_particle_linear_curve_00affa70 // 00b0bcd0
        jmp l_00b0bcdc // 00b0bcd5
    l_00b0bcd7:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0bcd7
    l_00b0bcdc:
        fstp dword ptr [esp + 0x1c] // 00b0bcdc
    l_00b0bce0:
        mov ecx, dword ptr [esi + 0xb4] // 00b0bce0
        movzx eax, word ptr [ecx + 0xa] // 00b0bce6
        test ax, ax // 00b0bcea
        jne l_00b0bcfc // 00b0bced
        movss xmm0, dword ptr [ecx + 4] // 00b0bcef
        movss dword ptr [esp + 0x18], xmm0 // 00b0bcf4
        jmp l_00b0bd18 // 00b0bcfa
    l_00b0bcfc:
        cmp ax, 1 // 00b0bcfc
        fldz  // 00b0bd00
        push ecx // 00b0bd02
        fstp dword ptr [esp] // 00b0bd03
        jne l_00b0bd0f // 00b0bd06
        call evaluate_native_particle_linear_curve_00affa70 // 00b0bd08
        jmp l_00b0bd14 // 00b0bd0d
    l_00b0bd0f:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0bd0f
    l_00b0bd14:
        fstp dword ptr [esp + 0x18] // 00b0bd14
    l_00b0bd18:
        mov eax, dword ptr [esi + 0xbc] // 00b0bd18
        movss xmm0, dword ptr [eax] // 00b0bd1e
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0bd22
        pop edx
        lahf  // 00b0bd29
        test ah, 0x44 // 00b0bd2a
        movss dword ptr [esp + 0x2c], xmm0 // 00b0bd2d
        jp l_00b0bd45 // 00b0bd33
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0bd35
        pop edx
        movss dword ptr [esp + 0x2c], xmm0 // 00b0bd3d
        jmp l_00b0bd68 // 00b0bd43
    l_00b0bd45:
        xor ecx, ecx // 00b0bd45
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0bd47
        mov dword ptr [esp + 0x30], eax // 00b0bd4c
        fild dword ptr [esp + 0x30] // 00b0bd50
        fmul dword ptr [esp + 0x2c] // 00b0bd54
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0bd58
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0bd5e
        pop edx
        fstp dword ptr [esp + 0x2c] // 00b0bd64
    l_00b0bd68:
        mov eax, dword ptr [esi + 0xb8] // 00b0bd68
        movss xmm0, dword ptr [eax] // 00b0bd6e
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0bd72
        pop edx
        lahf  // 00b0bd79
        test ah, 0x44 // 00b0bd7a
        movss dword ptr [esp + 0x28], xmm0 // 00b0bd7d
        jp l_00b0bd95 // 00b0bd83
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0bd85
        pop edx
        movss dword ptr [esp + 0x28], xmm0 // 00b0bd8d
        jmp l_00b0bdb8 // 00b0bd93
    l_00b0bd95:
        xor ecx, ecx // 00b0bd95
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0bd97
        mov dword ptr [esp + 0x30], eax // 00b0bd9c
        fild dword ptr [esp + 0x30] // 00b0bda0
        fmul dword ptr [esp + 0x28] // 00b0bda4
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0bda8
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0bdae
        pop edx
        fstp dword ptr [esp + 0x28] // 00b0bdb4
    l_00b0bdb8:
        mov eax, dword ptr [esi + 0xb4] // 00b0bdb8
        movss xmm0, dword ptr [eax] // 00b0bdbe
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0bdc2
        pop edx
        lahf  // 00b0bdc9
        test ah, 0x44 // 00b0bdca
        movss dword ptr [esp + 0x24], xmm0 // 00b0bdcd
        jp l_00b0bde5 // 00b0bdd3
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0bdd5
        pop edx
        movss dword ptr [esp + 0x24], xmm0 // 00b0bddd
        jmp l_00b0be08 // 00b0bde3
    l_00b0bde5:
        xor ecx, ecx // 00b0bde5
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0bde7
        mov dword ptr [esp + 0x30], eax // 00b0bdec
        fild dword ptr [esp + 0x30] // 00b0bdf0
        fmul dword ptr [esp + 0x24] // 00b0bdf4
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0bdf8
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0bdfe
        pop edx
        fstp dword ptr [esp + 0x24] // 00b0be04
    l_00b0be08:
        fld dword ptr [esp + 0x24] // 00b0be08
        fmul dword ptr [esp + 0x18] // 00b0be0c
        mov byte ptr [esp + 0x1b], 0xff // 00b0be10
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+104] // current native global 00ce4b48
        fld qword ptr [edx] // 00b0be15
        pop edx
        fnstcw word ptr [esp + 0x12] // 00b0be1b
        movzx eax, word ptr [esp + 0x12] // 00b0be1f
        fmul st(1), st(0) // 00b0be24
        or eax, 0xc00 // 00b0be26
        fxch st(1) // 00b0be2b
        mov dword ptr [esp + 0x30], eax // 00b0be2d
        fldcw word ptr [esp + 0x30] // 00b0be31
        fistp dword ptr [esp + 0x30] // 00b0be35
        mov cl, byte ptr [esp + 0x30] // 00b0be39
        mov byte ptr [esp + 0x1a], cl // 00b0be3d
        mov ecx, dword ptr [ebp + 0x34] // 00b0be41
        fldcw word ptr [esp + 0x12] // 00b0be44
        fld dword ptr [esp + 0x28] // 00b0be48
        fmul dword ptr [esp + 0x1c] // 00b0be4c
        fnstcw word ptr [esp + 0x12] // 00b0be50
        fmul st(0), st(1) // 00b0be54
        movzx eax, word ptr [esp + 0x12] // 00b0be56
        or eax, 0xc00 // 00b0be5b
        mov dword ptr [esp + 0x30], eax // 00b0be60
        fldcw word ptr [esp + 0x30] // 00b0be64
        fistp dword ptr [esp + 0x30] // 00b0be68
        mov dl, byte ptr [esp + 0x30] // 00b0be6c
        mov byte ptr [esp + 0x19], dl // 00b0be70
        fldcw word ptr [esp + 0x12] // 00b0be74
        fld dword ptr [esp + 0x2c] // 00b0be78
        fmul dword ptr [esp + 0x20] // 00b0be7c
        fnstcw word ptr [esp + 0x12] // 00b0be80
        fmulp st(1), st(0) // 00b0be84
        movzx eax, word ptr [esp + 0x12] // 00b0be86
        or eax, 0xc00 // 00b0be8b
        mov dword ptr [esp + 0x30], eax // 00b0be90
        fldcw word ptr [esp + 0x30] // 00b0be94
        fistp dword ptr [esp + 0x30] // 00b0be98
        mov al, byte ptr [esp + 0x30] // 00b0be9c
        mov byte ptr [esp + 0x18], al // 00b0bea0
        mov edx, dword ptr [esp + 0x18] // 00b0bea4
        mov dword ptr [ecx + 0x30], edx // 00b0bea8
        mov eax, dword ptr [esi + 0x9c] // 00b0beab
        fldcw word ptr [esp + 0x12] // 00b0beb1
        test eax, eax // 00b0beb5
        je l_00b0bf60 // 00b0beb7
        mov edi, dword ptr [ebp + 0x34] // 00b0bebd
        add edi, 0x4c // 00b0bec0
        mov dword ptr [edi + 4], 0 // 00b0bec3
        mov eax, dword ptr [esi + 0x9c] // 00b0beca
        mov ecx, dword ptr [edi + 8] // 00b0bed0
        cmp eax, ecx // 00b0bed3
        jbe l_00b0bf22 // 00b0bed5
        xor ecx, ecx // 00b0bed7
        mov dword ptr [edi + 8], eax // 00b0bed9
        mov edx, 4 // 00b0bedc
        mul edx // 00b0bee1
        seto cl // 00b0bee3
        neg ecx // 00b0bee6
        or ecx, eax // 00b0bee8
        push ecx // 00b0beea
        mov edx,dword ptr [esp+0x9c] // borrowed access
        call array_allocate_bridge // 00b0beeb
        cmp dword ptr [edi], 0 // 00b0bef3
        mov ebx, eax // 00b0bef6
        je l_00b0bf20 // 00b0bef8
        mov ecx, dword ptr [edi + 4] // 00b0befa
        xor eax, eax // 00b0befd
        test ecx, ecx // 00b0beff
        jbe l_00b0bf15 // 00b0bf01
    l_00b0bf03:
        mov edx, dword ptr [edi] // 00b0bf03
        mov ecx, dword ptr [edx + eax*4] // 00b0bf05
        mov dword ptr [ebx + eax*4], ecx // 00b0bf08
        mov edx, dword ptr [edi + 4] // 00b0bf0b
        add eax, 1 // 00b0bf0e
        cmp eax, edx // 00b0bf11
        jb l_00b0bf03 // 00b0bf13
    l_00b0bf15:
        mov eax, dword ptr [edi] // 00b0bf15
        push eax // 00b0bf17
        mov edx,dword ptr [esp+0x9c] // borrowed access
        call free_bridge // 00b0bf18
    l_00b0bf20:
        mov dword ptr [edi], ebx // 00b0bf20
    l_00b0bf22:
        mov eax, dword ptr [esi + 0x9c] // 00b0bf22
        mov dword ptr [edi + 4], eax // 00b0bf28
        mov ecx, dword ptr [esi + 0x9c] // 00b0bf2b
        xor eax, eax // 00b0bf31
        test ecx, ecx // 00b0bf33
        jbe l_00b0bfe0 // 00b0bf35
        jmp l_00b0bf40 // 00b0bf3b
    l_00b0bf40:
        mov ecx, dword ptr [esi + 0x98] // 00b0bf40
        mov edx, dword ptr [edi] // 00b0bf46
        mov ecx, dword ptr [ecx + eax*4] // 00b0bf48
        mov dword ptr [edx + eax*4], ecx // 00b0bf4b
        mov edx, dword ptr [esi + 0x9c] // 00b0bf4e
        add eax, 1 // 00b0bf54
        cmp eax, edx // 00b0bf57
        jb l_00b0bf40 // 00b0bf59
        jmp l_00b0bfe0 // 00b0bf5b
    l_00b0bf60:
        mov edx,dword ptr [esp+0x98] // borrowed access
        call resources_bridge // 00b0bf60
        mov eax, dword ptr [eax + 0x14] // 00b0bf65
        mov edi, dword ptr [ebp + 0x34] // 00b0bf68
        mov ecx, dword ptr [edi + 0x50] // 00b0bf6b
        add edi, 0x4c // 00b0bf6e
        mov dword ptr [esp + 0x30], eax // 00b0bf71
        mov eax, dword ptr [edi + 8] // 00b0bf75
        cmp ecx, eax // 00b0bf78
        jne l_00b0bfd0 // 00b0bf7a
        lea eax, [eax + eax + 2] // 00b0bf7c
        cmp eax, dword ptr [edi + 8] // 00b0bf80
        jbe l_00b0bfd0 // 00b0bf83
        xor ecx, ecx // 00b0bf85
        mov dword ptr [edi + 8], eax // 00b0bf87
        mov edx, 4 // 00b0bf8a
        mul edx // 00b0bf8f
        seto cl // 00b0bf91
        neg ecx // 00b0bf94
        or ecx, eax // 00b0bf96
        push ecx // 00b0bf98
        mov edx,dword ptr [esp+0x9c] // borrowed access
        call array_allocate_bridge // 00b0bf99
        cmp dword ptr [edi], 0 // 00b0bfa1
        mov ebx, eax // 00b0bfa4
        je l_00b0bfce // 00b0bfa6
        mov ecx, dword ptr [edi + 4] // 00b0bfa8
        xor eax, eax // 00b0bfab
        test ecx, ecx // 00b0bfad
        jbe l_00b0bfc3 // 00b0bfaf
    l_00b0bfb1:
        mov edx, dword ptr [edi] // 00b0bfb1
        mov ecx, dword ptr [edx + eax*4] // 00b0bfb3
        mov dword ptr [ebx + eax*4], ecx // 00b0bfb6
        mov edx, dword ptr [edi + 4] // 00b0bfb9
        add eax, 1 // 00b0bfbc
        cmp eax, edx // 00b0bfbf
        jb l_00b0bfb1 // 00b0bfc1
    l_00b0bfc3:
        mov eax, dword ptr [edi] // 00b0bfc3
        push eax // 00b0bfc5
        mov edx,dword ptr [esp+0x9c] // borrowed access
        call free_bridge // 00b0bfc6
    l_00b0bfce:
        mov dword ptr [edi], ebx // 00b0bfce
    l_00b0bfd0:
        mov eax, dword ptr [edi + 4] // 00b0bfd0
        mov ecx, dword ptr [edi] // 00b0bfd3
        mov edx, dword ptr [esp + 0x30] // 00b0bfd5
        mov dword ptr [ecx + eax*4], edx // 00b0bfd9
        add dword ptr [edi + 4], 1 // 00b0bfdc
    l_00b0bfe0:
        mov eax, dword ptr [ebp + 0x34] // 00b0bfe0
        mov cl, byte ptr [esi + 0x4c] // 00b0bfe3
        mov byte ptr [eax + 4], cl // 00b0bfe6
        mov edx, dword ptr [ebp + 0x34] // 00b0bfe9
        mov al, byte ptr [esi + 0x62] // 00b0bfec
        mov byte ptr [edx + 5], al // 00b0bfef
        mov ecx, dword ptr [ebp + 0x34] // 00b0bff2
        mov edx, dword ptr [esi + 0x50] // 00b0bff5
        mov dword ptr [ecx + 8], edx // 00b0bff8
        mov eax, dword ptr [ebp + 0x34] // 00b0bffb
        mov ecx, dword ptr [esi + 0x54] // 00b0bffe
        mov dword ptr [eax + 0xc], ecx // 00b0c001
        mov ecx, dword ptr [esi + 0x5c] // 00b0c004
        movzx eax, word ptr [ecx + 0xa] // 00b0c007
        test ax, ax // 00b0c00b
        jne l_00b0c01d // 00b0c00e
        movss xmm0, dword ptr [ecx + 4] // 00b0c010
        movss dword ptr [esp + 0x20], xmm0 // 00b0c015
        jmp l_00b0c039 // 00b0c01b
    l_00b0c01d:
        cmp ax, 1 // 00b0c01d
        fldz  // 00b0c021
        push ecx // 00b0c023
        fstp dword ptr [esp] // 00b0c024
        jne l_00b0c030 // 00b0c027
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c029
        jmp l_00b0c035 // 00b0c02e
    l_00b0c030:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c030
    l_00b0c035:
        fstp dword ptr [esp + 0x20] // 00b0c035
    l_00b0c039:
        mov edx, dword ptr [ebp + 0x34] // 00b0c039
        movss xmm0, dword ptr [esp + 0x20] // 00b0c03c
        movss dword ptr [edx + 0x10], xmm0 // 00b0c042
        mov eax, dword ptr [ebp + 0x34] // 00b0c047
        mov cl, byte ptr [esi + 0xb0] // 00b0c04a
        mov byte ptr [eax + 0x58], cl // 00b0c050
        mov eax, dword ptr [esi + 0xa4] // 00b0c053
        test eax, eax // 00b0c059
        jne l_00b0c065 // 00b0c05b
        mov edx,dword ptr [esp+0x98] // borrowed access
        call resources_bridge // 00b0c05d
        mov eax, dword ptr [eax + 0xc] // 00b0c062
    l_00b0c065:
        mov edx, dword ptr [ebp + 0x34] // 00b0c065
        mov dword ptr [edx + 0x60], eax // 00b0c068
        mov eax, dword ptr [esi + 0xa8] // 00b0c06b
        test eax, eax // 00b0c071
        jne l_00b0c07d // 00b0c073
        mov edx,dword ptr [esp+0x98] // borrowed access
        call resources_bridge // 00b0c075
        mov eax, dword ptr [eax + 0x10] // 00b0c07a
    l_00b0c07d:
        mov ecx, dword ptr [ebp + 0x34] // 00b0c07d
        mov dword ptr [ecx + 0x5c], eax // 00b0c080
        mov ecx, dword ptr [esi + 0xd4] // 00b0c083
        movzx eax, word ptr [ecx + 0xa] // 00b0c089
        test ax, ax // 00b0c08d
        jne l_00b0c09f // 00b0c090
        movss xmm0, dword ptr [ecx + 4] // 00b0c092
        movss dword ptr [esp + 0x24], xmm0 // 00b0c097
        jmp l_00b0c0bb // 00b0c09d
    l_00b0c09f:
        cmp ax, 1 // 00b0c09f
        fldz  // 00b0c0a3
        push ecx // 00b0c0a5
        fstp dword ptr [esp] // 00b0c0a6
        jne l_00b0c0b2 // 00b0c0a9
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c0ab
        jmp l_00b0c0b7 // 00b0c0b0
    l_00b0c0b2:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c0b2
    l_00b0c0b7:
        fstp dword ptr [esp + 0x24] // 00b0c0b7
    l_00b0c0bb:
        mov ecx, dword ptr [esi + 0xd0] // 00b0c0bb
        movzx eax, word ptr [ecx + 0xa] // 00b0c0c1
        test ax, ax // 00b0c0c5
        jne l_00b0c0d7 // 00b0c0c8
        movss xmm0, dword ptr [ecx + 4] // 00b0c0ca
        movss dword ptr [esp + 0x1c], xmm0 // 00b0c0cf
        jmp l_00b0c0f3 // 00b0c0d5
    l_00b0c0d7:
        cmp ax, 1 // 00b0c0d7
        fldz  // 00b0c0db
        push ecx // 00b0c0dd
        fstp dword ptr [esp] // 00b0c0de
        jne l_00b0c0ea // 00b0c0e1
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c0e3
        jmp l_00b0c0ef // 00b0c0e8
    l_00b0c0ea:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c0ea
    l_00b0c0ef:
        fstp dword ptr [esp + 0x1c] // 00b0c0ef
    l_00b0c0f3:
        mov ecx, dword ptr [esi + 0xcc] // 00b0c0f3
        movzx eax, word ptr [ecx + 0xa] // 00b0c0f9
        test ax, ax // 00b0c0fd
        jne l_00b0c10f // 00b0c100
        movss xmm0, dword ptr [ecx + 4] // 00b0c102
        movss dword ptr [esp + 0x20], xmm0 // 00b0c107
        jmp l_00b0c12b // 00b0c10d
    l_00b0c10f:
        cmp ax, 1 // 00b0c10f
        fldz  // 00b0c113
        push ecx // 00b0c115
        fstp dword ptr [esp] // 00b0c116
        jne l_00b0c122 // 00b0c119
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c11b
        jmp l_00b0c127 // 00b0c120
    l_00b0c122:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c122
    l_00b0c127:
        fstp dword ptr [esp + 0x20] // 00b0c127
    l_00b0c12b:
        mov eax, dword ptr [esi + 0xd4] // 00b0c12b
        movss xmm0, dword ptr [eax] // 00b0c131
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0c135
        pop edx
        lahf  // 00b0c13c
        test ah, 0x44 // 00b0c13d
        movss dword ptr [esp + 0x2c], xmm0 // 00b0c140
        jp l_00b0c158 // 00b0c146
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0c148
        pop edx
        movss dword ptr [esp + 0x30], xmm0 // 00b0c150
        jmp l_00b0c17b // 00b0c156
    l_00b0c158:
        xor ecx, ecx // 00b0c158
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0c15a
        mov dword ptr [esp + 0x30], eax // 00b0c15f
        fild dword ptr [esp + 0x30] // 00b0c163
        fmul dword ptr [esp + 0x2c] // 00b0c167
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0c16b
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0c171
        pop edx
        fstp dword ptr [esp + 0x30] // 00b0c177
    l_00b0c17b:
        mov eax, dword ptr [esi + 0xd0] // 00b0c17b
        movss xmm0, dword ptr [eax] // 00b0c181
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0c185
        pop edx
        lahf  // 00b0c18c
        test ah, 0x44 // 00b0c18d
        movss dword ptr [esp + 0x28], xmm0 // 00b0c190
        jp l_00b0c1a8 // 00b0c196
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0c198
        pop edx
        movss dword ptr [esp + 0x28], xmm0 // 00b0c1a0
        jmp l_00b0c1cb // 00b0c1a6
    l_00b0c1a8:
        xor ecx, ecx // 00b0c1a8
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0c1aa
        mov dword ptr [esp + 0x2c], eax // 00b0c1af
        fild dword ptr [esp + 0x2c] // 00b0c1b3
        fmul dword ptr [esp + 0x28] // 00b0c1b7
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0c1bb
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0c1c1
        pop edx
        fstp dword ptr [esp + 0x28] // 00b0c1c7
    l_00b0c1cb:
        mov eax, dword ptr [esi + 0xcc] // 00b0c1cb
        movss xmm0, dword ptr [eax] // 00b0c1d1
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0c1d5
        pop edx
        lahf  // 00b0c1dc
        test ah, 0x44 // 00b0c1dd
        movss dword ptr [esp + 0x18], xmm0 // 00b0c1e0
        jp l_00b0c1f8 // 00b0c1e6
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0c1e8
        pop edx
        movss dword ptr [esp + 0x2c], xmm0 // 00b0c1f0
        jmp l_00b0c21b // 00b0c1f6
    l_00b0c1f8:
        xor ecx, ecx // 00b0c1f8
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0c1fa
        mov dword ptr [esp + 0x2c], eax // 00b0c1ff
        fild dword ptr [esp + 0x2c] // 00b0c203
        fmul dword ptr [esp + 0x18] // 00b0c207
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0c20b
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0c211
        pop edx
        fstp dword ptr [esp + 0x2c] // 00b0c217
    l_00b0c21b:
        fld dword ptr [esp + 0x2c] // 00b0c21b
        mov byte ptr [esp + 0x1b], 0xff // 00b0c21f
        fmul dword ptr [esp + 0x20] // 00b0c224
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+104] // current native global 00ce4b48
        fld qword ptr [edx] // 00b0c228
        pop edx
        fnstcw word ptr [esp + 0x12] // 00b0c22e
        movzx eax, word ptr [esp + 0x12] // 00b0c232
        fmul st(1), st(0) // 00b0c237
        or eax, 0xc00 // 00b0c239
        fxch st(1) // 00b0c23e
        mov dword ptr [esp + 0x2c], eax // 00b0c240
        fldcw word ptr [esp + 0x2c] // 00b0c244
        fistp dword ptr [esp + 0x2c] // 00b0c248
        mov dl, byte ptr [esp + 0x2c] // 00b0c24c
        mov byte ptr [esp + 0x1a], dl // 00b0c250
        mov edx, dword ptr [ebp + 0x34] // 00b0c254
        fldcw word ptr [esp + 0x12] // 00b0c257
        fld dword ptr [esp + 0x28] // 00b0c25b
        fmul dword ptr [esp + 0x1c] // 00b0c25f
        fnstcw word ptr [esp + 0x12] // 00b0c263
        fmul st(0), st(1) // 00b0c267
        movzx eax, word ptr [esp + 0x12] // 00b0c269
        or eax, 0xc00 // 00b0c26e
        mov dword ptr [esp + 0x2c], eax // 00b0c273
        fldcw word ptr [esp + 0x2c] // 00b0c277
        fistp dword ptr [esp + 0x2c] // 00b0c27b
        mov al, byte ptr [esp + 0x2c] // 00b0c27f
        mov byte ptr [esp + 0x19], al // 00b0c283
        fldcw word ptr [esp + 0x12] // 00b0c287
        fld dword ptr [esp + 0x30] // 00b0c28b
        fmul dword ptr [esp + 0x24] // 00b0c28f
        fnstcw word ptr [esp + 0x12] // 00b0c293
        fmulp st(1), st(0) // 00b0c297
        movzx eax, word ptr [esp + 0x12] // 00b0c299
        or eax, 0xc00 // 00b0c29e
        mov dword ptr [esp + 0x30], eax // 00b0c2a3
        fldcw word ptr [esp + 0x30] // 00b0c2a7
        fistp dword ptr [esp + 0x30] // 00b0c2ab
        mov cl, byte ptr [esp + 0x30] // 00b0c2af
        mov byte ptr [esp + 0x18], cl // 00b0c2b3
        mov eax, dword ptr [esp + 0x18] // 00b0c2b7
        mov dword ptr [edx + 0x64], eax // 00b0c2bb
        mov ecx, dword ptr [esi + 0xc8] // 00b0c2be
        fldcw word ptr [esp + 0x12] // 00b0c2c4
        movzx eax, word ptr [ecx + 0xa] // 00b0c2c8
        test ax, ax // 00b0c2cc
        jne l_00b0c2de // 00b0c2cf
        movss xmm0, dword ptr [ecx + 4] // 00b0c2d1
        movss dword ptr [esp + 0x1c], xmm0 // 00b0c2d6
        jmp l_00b0c2fa // 00b0c2dc
    l_00b0c2de:
        cmp ax, 1 // 00b0c2de
        fldz  // 00b0c2e2
        push ecx // 00b0c2e4
        fstp dword ptr [esp] // 00b0c2e5
        jne l_00b0c2f1 // 00b0c2e8
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c2ea
        jmp l_00b0c2f6 // 00b0c2ef
    l_00b0c2f1:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c2f1
    l_00b0c2f6:
        fstp dword ptr [esp + 0x1c] // 00b0c2f6
    l_00b0c2fa:
        mov ecx, dword ptr [esi + 0xc4] // 00b0c2fa
        movzx eax, word ptr [ecx + 0xa] // 00b0c300
        test ax, ax // 00b0c304
        jne l_00b0c316 // 00b0c307
        movss xmm0, dword ptr [ecx + 4] // 00b0c309
        movss dword ptr [esp + 0x20], xmm0 // 00b0c30e
        jmp l_00b0c332 // 00b0c314
    l_00b0c316:
        cmp ax, 1 // 00b0c316
        fldz  // 00b0c31a
        push ecx // 00b0c31c
        fstp dword ptr [esp] // 00b0c31d
        jne l_00b0c329 // 00b0c320
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c322
        jmp l_00b0c32e // 00b0c327
    l_00b0c329:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c329
    l_00b0c32e:
        fstp dword ptr [esp + 0x20] // 00b0c32e
    l_00b0c332:
        mov ecx, dword ptr [esi + 0xc0] // 00b0c332
        movzx eax, word ptr [ecx + 0xa] // 00b0c338
        test ax, ax // 00b0c33c
        jne l_00b0c34e // 00b0c33f
        movss xmm0, dword ptr [ecx + 4] // 00b0c341
        movss dword ptr [esp + 0x24], xmm0 // 00b0c346
        jmp l_00b0c36a // 00b0c34c
    l_00b0c34e:
        cmp ax, 1 // 00b0c34e
        fldz  // 00b0c352
        push ecx // 00b0c354
        fstp dword ptr [esp] // 00b0c355
        jne l_00b0c361 // 00b0c358
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c35a
        jmp l_00b0c366 // 00b0c35f
    l_00b0c361:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c361
    l_00b0c366:
        fstp dword ptr [esp + 0x24] // 00b0c366
    l_00b0c36a:
        mov eax, dword ptr [esi + 0xc8] // 00b0c36a
        movss xmm0, dword ptr [eax] // 00b0c370
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0c374
        pop edx
        lahf  // 00b0c37b
        test ah, 0x44 // 00b0c37c
        movss dword ptr [esp + 0x2c], xmm0 // 00b0c37f
        jp l_00b0c397 // 00b0c385
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0c387
        pop edx
        movss dword ptr [esp + 0x28], xmm0 // 00b0c38f
        jmp l_00b0c3ba // 00b0c395
    l_00b0c397:
        xor ecx, ecx // 00b0c397
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0c399
        mov dword ptr [esp + 0x30], eax // 00b0c39e
        fild dword ptr [esp + 0x30] // 00b0c3a2
        fmul dword ptr [esp + 0x2c] // 00b0c3a6
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0c3aa
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0c3b0
        pop edx
        fstp dword ptr [esp + 0x28] // 00b0c3b6
    l_00b0c3ba:
        mov eax, dword ptr [esi + 0xc4] // 00b0c3ba
        movss xmm0, dword ptr [eax] // 00b0c3c0
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0c3c4
        pop edx
        lahf  // 00b0c3cb
        test ah, 0x44 // 00b0c3cc
        movss dword ptr [esp + 0x2c], xmm0 // 00b0c3cf
        jp l_00b0c3e7 // 00b0c3d5
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0c3d7
        pop edx
        movss dword ptr [esp + 0x2c], xmm0 // 00b0c3df
        jmp l_00b0c40a // 00b0c3e5
    l_00b0c3e7:
        xor ecx, ecx // 00b0c3e7
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0c3e9
        mov dword ptr [esp + 0x30], eax // 00b0c3ee
        fild dword ptr [esp + 0x30] // 00b0c3f2
        fmul dword ptr [esp + 0x2c] // 00b0c3f6
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0c3fa
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0c400
        pop edx
        fstp dword ptr [esp + 0x2c] // 00b0c406
    l_00b0c40a:
        mov eax, dword ptr [esi + 0xc0] // 00b0c40a
        movss xmm0, dword ptr [eax] // 00b0c410
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0c414
        pop edx
        lahf  // 00b0c41b
        test ah, 0x44 // 00b0c41c
        movss dword ptr [esp + 0x18], xmm0 // 00b0c41f
        jp l_00b0c437 // 00b0c425
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0c427
        pop edx
        movss dword ptr [esp + 0x30], xmm0 // 00b0c42f
        jmp l_00b0c45a // 00b0c435
    l_00b0c437:
        xor ecx, ecx // 00b0c437
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0c439
        mov dword ptr [esp + 0x30], eax // 00b0c43e
        fild dword ptr [esp + 0x30] // 00b0c442
        fmul dword ptr [esp + 0x18] // 00b0c446
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0c44a
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0c450
        pop edx
        fstp dword ptr [esp + 0x30] // 00b0c456
    l_00b0c45a:
        fld dword ptr [esp + 0x24] // 00b0c45a
        mov byte ptr [esp + 0x1b], 0xff // 00b0c45e
        fmul dword ptr [esp + 0x30] // 00b0c463
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+104] // current native global 00ce4b48
        fld qword ptr [edx] // 00b0c467
        pop edx
        fnstcw word ptr [esp + 0x12] // 00b0c46d
        movzx eax, word ptr [esp + 0x12] // 00b0c471
        fmul st(1), st(0) // 00b0c476
        or eax, 0xc00 // 00b0c478
        fxch st(1) // 00b0c47d
        mov dword ptr [esp + 0x30], eax // 00b0c47f
        fldcw word ptr [esp + 0x30] // 00b0c483
        fistp dword ptr [esp + 0x30] // 00b0c487
        mov cl, byte ptr [esp + 0x30] // 00b0c48b
        mov byte ptr [esp + 0x1a], cl // 00b0c48f
        mov ecx, dword ptr [ebp + 0x34] // 00b0c493
        fldcw word ptr [esp + 0x12] // 00b0c496
        fld dword ptr [esp + 0x20] // 00b0c49a
        fmul dword ptr [esp + 0x2c] // 00b0c49e
        fnstcw word ptr [esp + 0x12] // 00b0c4a2
        fmul st(0), st(1) // 00b0c4a6
        movzx eax, word ptr [esp + 0x12] // 00b0c4a8
        or eax, 0xc00 // 00b0c4ad
        mov dword ptr [esp + 0x30], eax // 00b0c4b2
        fldcw word ptr [esp + 0x30] // 00b0c4b6
        fistp dword ptr [esp + 0x30] // 00b0c4ba
        mov dl, byte ptr [esp + 0x30] // 00b0c4be
        mov byte ptr [esp + 0x19], dl // 00b0c4c2
        fldcw word ptr [esp + 0x12] // 00b0c4c6
        fld dword ptr [esp + 0x1c] // 00b0c4ca
        fmul dword ptr [esp + 0x28] // 00b0c4ce
        fnstcw word ptr [esp + 0x12] // 00b0c4d2
        fmulp st(1), st(0) // 00b0c4d6
        movzx eax, word ptr [esp + 0x12] // 00b0c4d8
        or eax, 0xc00 // 00b0c4dd
        mov dword ptr [esp + 0x30], eax // 00b0c4e2
        fldcw word ptr [esp + 0x30] // 00b0c4e6
        fistp dword ptr [esp + 0x30] // 00b0c4ea
        mov al, byte ptr [esp + 0x30] // 00b0c4ee
        mov byte ptr [esp + 0x18], al // 00b0c4f2
        mov edx, dword ptr [esp + 0x18] // 00b0c4f6
        mov dword ptr [ecx + 0x68], edx // 00b0c4fa
        fldcw word ptr [esp + 0x12] // 00b0c4fd
        mov eax, dword ptr [ebp + 0x34] // 00b0c501
        fld dword ptr [esi + 0xac] // 00b0c504
        fstp dword ptr [eax + 0x6c] // 00b0c50a
        mov ecx, dword ptr [esi + 0x8c] // 00b0c50d
        movzx eax, word ptr [ecx + 0xa] // 00b0c513
        test ax, ax // 00b0c517
        jne l_00b0c529 // 00b0c51a
        movss xmm0, dword ptr [ecx + 4] // 00b0c51c
        movss dword ptr [esp + 0x24], xmm0 // 00b0c521
        jmp l_00b0c545 // 00b0c527
    l_00b0c529:
        cmp ax, 1 // 00b0c529
        fldz  // 00b0c52d
        push ecx // 00b0c52f
        fstp dword ptr [esp] // 00b0c530
        jne l_00b0c53c // 00b0c533
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c535
        jmp l_00b0c541 // 00b0c53a
    l_00b0c53c:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c53c
    l_00b0c541:
        fstp dword ptr [esp + 0x24] // 00b0c541
    l_00b0c545:
        mov eax, dword ptr [esi + 0x8c] // 00b0c545
        movss xmm0, dword ptr [eax] // 00b0c54b
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0c54f
        pop edx
        lahf  // 00b0c556
        test ah, 0x44 // 00b0c557
        movss dword ptr [esp + 0x2c], xmm0 // 00b0c55a
        jp l_00b0c572 // 00b0c560
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0c562
        pop edx
        movss dword ptr [esp + 0x30], xmm0 // 00b0c56a
        jmp l_00b0c595 // 00b0c570
    l_00b0c572:
        xor ecx, ecx // 00b0c572
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0c574
        mov dword ptr [esp + 0x30], eax // 00b0c579
        fild dword ptr [esp + 0x30] // 00b0c57d
        fmul dword ptr [esp + 0x2c] // 00b0c581
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0c585
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0c58b
        pop edx
        fstp dword ptr [esp + 0x30] // 00b0c591
    l_00b0c595:
        fld dword ptr [esp + 0x24] // 00b0c595
        mov ecx, dword ptr [ebp + 0x34] // 00b0c599
        fmul dword ptr [esp + 0x30] // 00b0c59c
        fstp dword ptr [esp + 0x30] // 00b0c5a0
        fld dword ptr [esp + 0x30] // 00b0c5a4
        fstp dword ptr [ecx + 0x70] // 00b0c5a8
        mov edx, dword ptr [ebp + 0x34] // 00b0c5ab
        fld dword ptr [esi + 0x90] // 00b0c5ae
        fstp dword ptr [edx + 0x74] // 00b0c5b4
        mov ecx, dword ptr [esi + 0xe4] // 00b0c5b7
        movzx eax, word ptr [ecx + 0xa] // 00b0c5bd
        test ax, ax // 00b0c5c1
        jne l_00b0c5d3 // 00b0c5c4
        movss xmm0, dword ptr [ecx + 4] // 00b0c5c6
        movss dword ptr [esp + 0x24], xmm0 // 00b0c5cb
        jmp l_00b0c5ef // 00b0c5d1
    l_00b0c5d3:
        cmp ax, 1 // 00b0c5d3
        fldz  // 00b0c5d7
        push ecx // 00b0c5d9
        fstp dword ptr [esp] // 00b0c5da
        jne l_00b0c5e6 // 00b0c5dd
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c5df
        jmp l_00b0c5eb // 00b0c5e4
    l_00b0c5e6:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c5e6
    l_00b0c5eb:
        fstp dword ptr [esp + 0x24] // 00b0c5eb
    l_00b0c5ef:
        mov eax, dword ptr [ebp + 0x34] // 00b0c5ef
        movss xmm0, dword ptr [esp + 0x24] // 00b0c5f2
        movss dword ptr [eax + 0x2c], xmm0 // 00b0c5f8
        mov ecx, dword ptr [esi + 0x94] // 00b0c5fd
        movzx eax, word ptr [ecx + 0xa] // 00b0c603
        test ax, ax // 00b0c607
        jne l_00b0c619 // 00b0c60a
        movss xmm0, dword ptr [ecx + 4] // 00b0c60c
        movss dword ptr [esp + 0x24], xmm0 // 00b0c611
        jmp l_00b0c635 // 00b0c617
    l_00b0c619:
        cmp ax, 1 // 00b0c619
        fldz  // 00b0c61d
        push ecx // 00b0c61f
        fstp dword ptr [esp] // 00b0c620
        jne l_00b0c62c // 00b0c623
        call evaluate_native_particle_linear_curve_00affa70 // 00b0c625
        jmp l_00b0c631 // 00b0c62a
    l_00b0c62c:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b0c62c
    l_00b0c631:
        fstp dword ptr [esp + 0x24] // 00b0c631
    l_00b0c635:
        mov eax, dword ptr [esi + 0x94] // 00b0c635
        movss xmm0, dword ptr [eax] // 00b0c63b
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+4] // current native global 00d7a218
        ucomiss xmm0, dword ptr [edx] // 00b0c63f
        pop edx
        lahf  // 00b0c646
        test ah, 0x44 // 00b0c647
        movss dword ptr [esp + 0x2c], xmm0 // 00b0c64a
        jp l_00b0c662 // 00b0c650
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm0, dword ptr [edx] // 00b0c652
        pop edx
        movss dword ptr [esp + 0x30], xmm0 // 00b0c65a
        jmp l_00b0c685 // 00b0c660
    l_00b0c662:
        xor ecx, ecx // 00b0c662
        mov edx,dword ptr [esp+0x98] // borrowed access
        call random_bridge // 00b0c664
        mov dword ptr [esp + 0x30], eax // 00b0c669
        fild dword ptr [esp + 0x30] // 00b0c66d
        fmul dword ptr [esp + 0x2c] // 00b0c671
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+12] // current native global 00d5da30
        fmul qword ptr [edx] // 00b0c675
        pop edx
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+16] // current native global 00d7a210
        fadd qword ptr [edx] // 00b0c67b
        pop edx
        fstp dword ptr [esp + 0x30] // 00b0c681
    l_00b0c685:
        fld dword ptr [esp + 0x24] // 00b0c685
        mov ecx, dword ptr [ebp + 0x34] // 00b0c689
        fmul dword ptr [esp + 0x30] // 00b0c68c
        fstp dword ptr [esp + 0x30] // 00b0c690
        fld dword ptr [esp + 0x30] // 00b0c694
        fstp dword ptr [ecx + 0x78] // 00b0c698
        mov eax, dword ptr [ebp + 0x34] // 00b0c69b
        fld dword ptr [eax + 0x74] // 00b0c69e
        mov ecx, 0x1b8 // 00b0c6a1
        fsub dword ptr [eax + 0x78] // 00b0c6a6
        fstp dword ptr [eax + 0x7c] // 00b0c6a9
        mov edx,dword ptr [esp+0x98] // borrowed access
        call node_allocate_bridge // 00b0c6ac
        mov edi, eax // 00b0c6b1
        mov dword ptr [esp + 0x30], edi // 00b0c6b3
        test edi, edi // 00b0c6b7
        mov dword ptr [esp + 0x94], 5 // 00b0c6b9
        je l_00b0c6f5 // 00b0c6c4
        push offset traceline_name // 00b0c6c6
        lea ecx, [esp + 0x38] // 00b0c6cb
        mov edx,dword ptr [esp+0x9c] // borrowed access
        call string_bridge // 00b0c6cf
        or dword ptr [esp + 0x14], 4 // 00b0c6d4
        lea edx, [esp + 0x34] // 00b0c6d9
        push edx // 00b0c6dd
        mov ecx, edi // 00b0c6de
        mov byte ptr [esp + 0x98], 6 // 00b0c6e0
        mov edx,dword ptr [esp+0x9c] // borrowed access
        call node_construct_bridge // 00b0c6e8
        mov dword ptr [edi], 0xd0c928 // 00b0c6ed
        jmp l_00b0c6f7 // 00b0c6f3
    l_00b0c6f5:
        xor edi, edi // 00b0c6f5
    l_00b0c6f7:
        test byte ptr [esp + 0x14], 4 // 00b0c6f7
        mov dword ptr [ebp + 0x30], edi // 00b0c6fc
        mov dword ptr [esp + 0x94], 0xffffffff // 00b0c6ff
        je l_00b0c72b // 00b0c70a
        mov ecx, dword ptr [esp + 0x38] // 00b0c70c
        test ecx, ecx // 00b0c710
        je l_00b0c72b // 00b0c712
        mov eax, dword ptr [esp + 0x34] // 00b0c714
        push 1 // 00b0c718
        add eax, 1 // 00b0c71a
        push eax // 00b0c71d
        push ecx // 00b0c71e
        mov edx,dword ptr [esp+0xa4] // borrowed access
        call storage_bridge // 00b0c71f
        mov ecx, eax // 00b0c724
        call string_release_bridge // 00b0c726
    l_00b0c72b:
        mov edi, dword ptr [esp + 0xa4] // 00b0c72b
        mov edx, dword ptr [edi + 0xa4] // 00b0c732
        mov edx, dword ptr [edx + 0xa4] // 00b0c738
        mov ecx, dword ptr [ebp + 0x30] // 00b0c73e
        mov eax, dword ptr [ecx] // 00b0c741
        push edx
        push ecx
        mov ecx,eax
        mov edx,dword ptr [esp+0xa0]
        push 0x5c
        call capture_bridge
        pop ecx
        pop edx
        push edx // 00b0c746
        mov edx, dword ptr [ebp + 0x34] // 00b0c747
        push edx // 00b0c74a
        mov edx,dword ptr [esp+0xa0] // borrowed access
        push eax // native captured target
        call node_attach_bridge // 00b0c74b
        mov eax, dword ptr [esi + 0x14] // 00b0c74d
        cmp byte ptr [eax + 0x64], 0 // 00b0c750
        jne l_00b0c765 // 00b0c754
        cmp byte ptr [eax + 0x65], 0 // 00b0c756
        jne l_00b0c765 // 00b0c75a
        mov ecx, edi // 00b0c75c
        call native_particle_record_matrix_00afda80 // 00b0c75e
        jmp l_00b0c7dd // 00b0c763
    l_00b0c765:
        xorps xmm0, xmm0 // 00b0c765
        push edx
        mov edx,dword ptr [esp+0x9c]
        mov edx,dword ptr [edx+8] // current native global 00d7a24c
        movss xmm1, dword ptr [edx] // 00b0c768
        pop edx
        movss dword ptr [esp + 0x4c], xmm1 // 00b0c770
        movss dword ptr [esp + 0x50], xmm0 // 00b0c776
        movss dword ptr [esp + 0x54], xmm0 // 00b0c77c
        movss dword ptr [esp + 0x58], xmm0 // 00b0c782
        movss dword ptr [esp + 0x5c], xmm0 // 00b0c788
        movss dword ptr [esp + 0x60], xmm1 // 00b0c78e
        movss dword ptr [esp + 0x64], xmm0 // 00b0c794
        movss dword ptr [esp + 0x68], xmm0 // 00b0c79a
        movss dword ptr [esp + 0x6c], xmm0 // 00b0c7a0
        movss dword ptr [esp + 0x70], xmm0 // 00b0c7a6
        movss dword ptr [esp + 0x74], xmm1 // 00b0c7ac
        movss dword ptr [esp + 0x78], xmm0 // 00b0c7b2
        movss dword ptr [esp + 0x7c], xmm0 // 00b0c7b8
        movss dword ptr [esp + 0x80], xmm0 // 00b0c7be
        movss dword ptr [esp + 0x84], xmm0 // 00b0c7c7
        movss dword ptr [esp + 0x88], xmm1 // 00b0c7d0
        lea eax, [esp + 0x4c] // 00b0c7d9
    l_00b0c7dd:
        fldz  // 00b0c7dd
        mov edx, dword ptr [esi] // 00b0c7df
        push ecx // 00b0c7e1
        fst dword ptr [esp] // 00b0c7e2
        push eax // 00b0c7e5
        push edx
        push ecx
        mov ecx,edx
        mov edx,dword ptr [esp+0xa8]
        push 0x28
        call capture_bridge
        pop ecx
        pop edx
        push 0 // 00b0c7e9
        push ecx // 00b0c7eb
        fstp dword ptr [esp] // 00b0c7ec
        push ebp // 00b0c7ef
        mov ecx, esi // 00b0c7f0
        mov edx,dword ptr [esp+0xac] // borrowed access
        push eax // native captured target
        call initial_update_bridge // 00b0c7f2
        pop edi // 00b0c7fb
        pop esi // 00b0c7fc
        pop ebp // 00b0c7fd
        pop ebx // 00b0c7fe
        add esp, 0x88 // 00b0c806
        add esp,4 // added borrowed access slot
        ret 8 // 00b0c80c
    }
}
} // namespace bsp
