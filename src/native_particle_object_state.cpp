#include "bsp/native_particle_object_state.hpp"
#include "bsp/native_particle_axial_loading.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_camera_matrix_math.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/native_point_light_owner.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstring>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle Object state requires MSVC Win32.
#endif
namespace bsp {
namespace {
template<class T> T load(const void* p,std::uint32_t o=0) {T v;std::memcpy(&v,static_cast<const std::byte*>(p)+o,sizeof v);return v;}
template<class T> void store(void* p,std::uint32_t o,T v) {std::memcpy(static_cast<std::byte*>(p)+o,&v,sizeof v);}
void* at(void* p,std::uint32_t o) {return static_cast<std::byte*>(p)+o;}
const void* at(const void* p,std::uint32_t o) {return static_cast<const std::byte*>(p)+o;}
static_assert(offsetof(NativeParticleObjectStateAccess,type)==0);
static_assert(offsetof(NativeParticleObjectStateAccess,unit_random)==4);
static_assert(offsetof(NativeParticleObjectStateAccess,full_angle_00ce3d9c)==8);
static_assert(offsetof(NativeParticleObjectStateAccess,half_00ce3800)==12);
static_assert(offsetof(NativeParticleObjectStateAccess,negative_one_00d7a260)==16);
static_assert(offsetof(NativeParticleObjectStateAccess,negative_zero_00d7a208)==20);
static_assert(offsetof(NativeParticleObjectStateAccess,operator_new_00bf681b)==44);
static_assert(offsetof(NativeParticleObjectStateAccess,free_00bf65ac)==48);
static_assert(offsetof(NativeParticleObjectStateAccess,truncate_st0_00bf7420)==52);
RandomState* __fastcall random_state_bridge(RandomStream stream,const NativeParticleUnitRandomAccess* a) {
    return &a->random->state_00bd2ed0(stream);
}
std::uint32_t __fastcall random_next_bridge(RandomState* state) {return random_next_u32_00ba2c20(*state);}
std::uint32_t __fastcall random_thread_bridge(RandomStream stream,const NativeParticleTypeStateAccess* a) {
    return a->random->next_00bd2fc0(stream);
}
struct PopulationGuard {
    TrackedCriticalSection* section;
    explicit PopulationGuard(const NativeParticleObjectStateAccess& a):section(
        get_native_particle_population_lock_0072b740(a.actual_manager_01090aa0,a.actual_lock_0108ff50)->section_04) {
        if(section){EnterCriticalSection(&section->native);auto& n=*reinterpret_cast<volatile std::uint32_t*>(&section->depth);n=n+1u;}
    }
    ~PopulationGuard(){if(section){auto& n=*reinterpret_cast<volatile std::uint32_t*>(&section->depth);n=n-1u;LeaveCriticalSection(&section->native);}}
};
void __fastcall create_light_bridge(void* definition,const NativeParticleObjectStateAccess* a,void* state) {
    if(!load<std::uint8_t>(definition,0x64))return;
    PopulationGuard guard(*a);
    auto& pool=a->lights.pool_0109011c;
    void* raw=pool.allocate_raw_slot_00b7b810(); // B7BD30 forwards to this canonical pool.
    if(!raw){store<void*>(state,0x60,nullptr);return;}
    bool owns_raw=true,prefix_ready=false,name_ready=false;
    NativeString prefix,name;
    try {
        construct_native_string_cstring_0041e870(&prefix,"dynamic_light_",a->strings);
        prefix_ready=true;
        const void* parent=load<const void*>(definition,0x14);
        concatenate_native_string_headers_004261a0(&prefix,&name,at(parent,8),a->strings);
        name_ready=true;
        auto constructed=construct_native_point_light_00b7c710(raw,NativePointLightPool::slot_bytes,name,a->lights.nodes.strings);
        owns_raw=false; // Adoption now owns native construction, even on host failure.
        adopt_constructed_native_point_light(a->lights,constructed);
        store<void*>(state,0x60,raw);
        destroy_native_string_header_0041dd20(&name,a->strings);name_ready=false;
        destroy_native_string_header_0041dd20(&prefix,a->strings);prefix_ready=false;
    } catch(...) {
        if(name_ready)destroy_native_string_header_0041dd20(&name,a->strings);
        if(prefix_ready)destroy_native_string_header_0041dd20(&prefix,a->strings);
        if(owns_raw)pool.return_raw_slot_00b7b1d0(raw);
        throw;
    }
}
void* __fastcall resource_bridge(void* resource,std::uint32_t target,std::uint32_t zero,float one,
    const NativeParticleObjectStateAccess* a) {
    return a->resource_virtual08(a->context,resource,target,zero,one);
}
void __fastcall roots_bridge(void* state,const NativeParticleObjectStateAccess* a,const void* record) {
    PopulationGuard guard(*a);
    void* roots=load<void*>(load<void*>(record,0xa4),0xa4);
    void* node=load<void*>(load<void*>(state,0x34),0xc);
    a->propagate_roots_00b6d890(a->context,node,roots);
}
void __fastcall virtual28_bridge(void* definition,std::uint32_t target,void* state,float dt,
    std::uint32_t word,const void* matrix,float time,const NativeParticleObjectStateAccess* a) {
    a->definition_virtual28(a->context,definition,target,state,dt,word,matrix,time);
}
void* __fastcall rotation_z_bridge(void* matrix,const float* angle,
    const volatile float* negative_zero,const volatile float* one) {
    build_gui_rotation_z_00b64780(*static_cast<CameraMatrix*>(matrix),*angle,negative_zero,one);return matrix;
}
} // namespace

bool __fastcall clear_native_particle_object_state_00af8b00(void*,const NativeParticleObjectStateAccess* a,
    void* state,std::uint32_t) {
    PopulationGuard guard(*a);
    void* instance=load<void*>(state,0x34);
    if(instance){
        if(InterlockedDecrement(static_cast<volatile LONG*>(at(instance,4)))==0){
            const auto target=load<std::uint32_t>(load<const void*>(instance));
            a->instance_virtual00(a->context,instance,target);
        }
        store<void*>(state,0x34,nullptr);
    }
    void* matrix=load<void*>(state,0x30);
    if(matrix){a->free_00bf65ac(matrix);store<void*>(state,0x30,nullptr);} // AF8B7B..84 returning-free continuation.
    return true;
}

__declspec(naked) float __fastcall native_particle_random_state_range_00bd2e60(RandomState*,const NativeParticleUnitRandomAccess*,float,float) {
    __asm {
        push ebx
        mov ebx,edx
        push ecx // 00bd2e60
        mov al,byte ptr [ecx + 0x9c5] // 00bd2e61
        test al,al // 00bd2e67
        jz l_00bd2e91 // 00bd2e69
        cmp byte ptr [ecx + 0x9c4],0x0 // 00bd2e6b
        jnz l_00bd2e7c // 00bd2e72
        xor edx,edx // 00bd2e74
        mov dword ptr [edx],0x3 // 00bd2e76
    l_00bd2e7c:
        test al,al // 00bd2e7c
        jz l_00bd2e91 // 00bd2e7e
        cmp byte ptr [ecx + 0x9c4],0x0 // 00bd2e80
        jnz l_00bd2e91 // 00bd2e87
        xor eax,eax // 00bd2e89
        mov dword ptr [eax],0x3 // 00bd2e8b
    l_00bd2e91:
        call random_next_bridge // 00bd2e91
        test eax,eax // 00bd2e96
        mov dword ptr [esp],eax // 00bd2e98
        fild dword ptr [esp] // 00bd2e9b
        jge l_00bd2ea6 // 00bd2e9e
        mov edx,dword ptr [ebx+4]
        fadd dword ptr [edx] // 00bd2ea0
    l_00bd2ea6:
        mov edx,dword ptr [ebx+8]
        fmul qword ptr [edx] // 00bd2ea6
        fstp dword ptr [esp] // 00bd2eac
        fld dword ptr [esp + 0x10] // 00bd2eaf
        fld dword ptr [esp + 0xc] // 00bd2eb3
        fld st(0) // 00bd2eb7
        fsubp st(2),st(0) // 00bd2eb9
        fxch // 00bd2ebb
        fmul dword ptr [esp] // 00bd2ebd
        faddp st(1),st(0) // 00bd2ec0
        fstp dword ptr [esp + 0x10] // 00bd2ec2
        fld dword ptr [esp + 0x10] // 00bd2ec6
        pop ecx // 00bd2eca
        pop ebx
        ret 0x8 // 00bd2ecb
    }
}

__declspec(naked) float __fastcall native_particle_random_range_00bd2f10(RandomStream,const NativeParticleUnitRandomAccess*,float,float) {
    __asm {
        push ebx
        mov ebx,edx
        fld dword ptr [esp + 0xc] // 00bd2f10
        sub esp,0x8 // 00bd2f14
        fstp dword ptr [esp + 0x4] // 00bd2f17
        fld dword ptr [esp + 0x10] // 00bd2f1b
        fstp dword ptr [esp] // 00bd2f1f
        call random_state_bridge // 00bd2f22
        mov ecx,eax // 00bd2f27
        mov edx,ebx
        call native_particle_random_state_range_00bd2e60 // 00bd2f29
        pop ebx
        ret 0x8 // 00bd2f2e
    }
}

__declspec(naked) void* __fastcall construct_native_particle_object_matrix_00af8440(void*,const NativeParticleObjectStateAccess*) {
    __asm {
        push ebp
        mov ebp,edx
        sub esp,0x14c // 00af8440
        mov eax,dword ptr [ebp+8]
        fld dword ptr [eax] // 00af8446
        push esi // 00af844c
        sub esp,0x8 // 00af844d
        fstp dword ptr [esp + 0x4] // 00af8450
        mov esi,ecx // 00af8454
        fldz // 00af8456
        xor ecx,ecx // 00af8458
        fstp dword ptr [esp] // 00af845a
        mov edx,dword ptr [ebp+4]
        call native_particle_random_range_00bd2f10 // 00af845d
        fstp dword ptr [esp + 0x8] // 00af8462
        mov eax,dword ptr [ebp+8]
        fld dword ptr [eax] // 00af8466
        sub esp,0x8 // 00af846c
        fstp dword ptr [esp + 0x4] // 00af846f
        xor ecx,ecx // 00af8473
        fldz // 00af8475
        fstp dword ptr [esp] // 00af8477
        mov edx,dword ptr [ebp+4]
        call native_particle_random_range_00bd2f10 // 00af847a
        fstp dword ptr [esp + 0x4] // 00af847f
        mov eax,dword ptr [ebp+8]
        fld dword ptr [eax] // 00af8483
        sub esp,0x8 // 00af8489
        fstp dword ptr [esp + 0x4] // 00af848c
        xor ecx,ecx // 00af8490
        fldz // 00af8492
        fstp dword ptr [esp] // 00af8494
        mov edx,dword ptr [ebp+4]
        call native_particle_random_range_00bd2f10 // 00af8497
        fstp dword ptr [esp + 0xc] // 00af849c
        lea edx,[esp + 0x4] // 00af84a0
        lea ecx,[esp + 0x50] // 00af84a4
        mov eax,dword ptr [ebp]
        push dword ptr [eax+8]
        push dword ptr [ebp+20]
        call build_native_particle_rotation_y_00b646e0 // 00af84a8
        push eax // 00af84ad
        lea eax,[esp + 0x14] // 00af84ae
        push eax // 00af84b2
        lea edx,[esp + 0x10] // 00af84b3
        lea ecx,[esp + 0xd8] // 00af84b7
        mov eax,dword ptr [ebp]
        push dword ptr [eax+8]
        push dword ptr [ebp+20]
        call build_native_particle_rotation_x_00b64640 // 00af84be
        push eax // 00af84c3
        lea ecx,[esp + 0x9c] // 00af84c4
        push ecx // 00af84cb
        lea edx,[esp + 0x1c] // 00af84cc
        lea ecx,[esp + 0x120] // 00af84d0
        mov eax,dword ptr [ebp]
        push dword ptr [eax+8]
        push dword ptr [ebp+20]
        call rotation_z_bridge // 00af84d7
        mov ecx,eax // 00af84dc
        call multiply_native_camera_matrices_00413920 // 00af84de
        mov ecx,eax // 00af84e3
        call multiply_native_camera_matrices_00413920 // 00af84e5
        lea edx,[esp + 0x10] // 00af84ea
        push edx // 00af84ee
        mov ecx,esi // 00af84ef
        call copy_native_camera_matrix_004134f0 // 00af84f1
        mov eax,esi // 00af84f6
        pop esi // 00af84f8
        add esp,0x14c // 00af84f9
        pop ebp
        ret // 00af84ff
    }
}

__declspec(naked) void* __fastcall compose_native_particle_record_matrix_00af8270(const void*,void*,void*) {
    __asm {
        mov eax,dword ptr [ecx + 0xa4] // 00af8270
        sub esp,0x40 // 00af8276
        push esi // 00af8279
        push edi // 00af827a
        lea esi,[eax + 0x298] // 00af827b
        mov ecx,0x10 // 00af8281
        lea edi,[esp + 0x8] // 00af8286
        rep movsd // 00af828a
        mov esi,dword ptr [esp + 0x4c] // 00af828c
        lea ecx,[esp + 0x8] // 00af8290
        push ecx // 00af8294
        push esi // 00af8295
        mov ecx,eax // 00af8296
        call get_native_node_local_matrix_00b6db60 // 00af8298
        mov ecx,eax // 00af829d
        call multiply_native_camera_matrices_00413920 // 00af829f
        pop edi // 00af82a4
        mov eax,esi // 00af82a5
        pop esi // 00af82a7
        add esp,0x40 // 00af82a8
        ret 0x4 // 00af82ab
    }
}

__declspec(naked) void __fastcall initialize_native_particle_object_state_00af90a0(void*,const NativeParticleObjectStateAccess*,void*,const void*) {
    __asm {
        sub esp,0xb0
        push ebx
        push ebp
        push esi
        push edi
        mov ebp,ecx
        mov edi,dword ptr [esp+0xc4]
        mov dword ptr [esp+0xb4],edx // borrowed access occupies removed FH3 saved-FS slot
        push edi
        call create_light_bridge
        or esi,0xffffffff
        mov eax,dword ptr [ebp + 0x1c] // 00af91fb
        movss xmm0,dword ptr [eax] // 00af91fe
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+4]
        ucomiss xmm0,dword ptr [edx] // 00af9202
        pop edx
        lahf // 00af9209
        test ah,0x44 // 00af920a
        movss dword ptr [esp + 0x14],xmm0 // 00af920d
        jp l_00af9225 // 00af9213
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+8]
        movss xmm0,dword ptr [edx] // 00af9215
        pop edx
        movss dword ptr [esp + 0x10],xmm0 // 00af921d
        jmp l_00af9248 // 00af9223
    l_00af9225:
        xor ecx,ecx // 00af9225
        mov edx,dword ptr [esp+0xb4]
        mov edx,dword ptr [edx]
        call random_thread_bridge // 00af9227
        mov dword ptr [esp + 0x10],eax // 00af922c
        fild dword ptr [esp + 0x10] // 00af9230
        fmul dword ptr [esp + 0x14] // 00af9234
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+12]
        fmul qword ptr [edx] // 00af9238
        pop edx
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+16]
        fadd qword ptr [edx] // 00af923e
        pop edx
        fstp dword ptr [esp + 0x10] // 00af9244
    l_00af9248:
        mov ecx,dword ptr [ebp + 0x1c] // 00af9248
        fldz // 00af924b
        cmp word ptr [ecx + 0xa],0x0 // 00af924d
        jnz l_00af9263 // 00af9252
        movss xmm0,dword ptr [ecx + 0x4] // 00af9254
        fstp st(0) // 00af9259
        movss dword ptr [esp + 0x14],xmm0 // 00af925b
        jmp l_00af927e // 00af9261
    l_00af9263:
        cmp word ptr [ecx + 0xa],0x1 // 00af9263
        push ecx // 00af9268
        fstp dword ptr [esp] // 00af9269
        jnz l_00af9275 // 00af926c
        call evaluate_native_particle_linear_curve_00affa70 // 00af926e
        jmp l_00af927a // 00af9273
    l_00af9275:
        call evaluate_native_particle_cubic_curve_00affae0 // 00af9275
    l_00af927a:
        fstp dword ptr [esp + 0x14] // 00af927a
    l_00af927e:
        fld dword ptr [esp + 0x14] // 00af927e
        sub esp,0x8 // 00af9282
        fmul dword ptr [esp + 0x18] // 00af9285
        xor ecx,ecx // 00af9289
        fstp dword ptr [edi + 0x44] // 00af928b
        push edx
        mov edx,dword ptr [esp+0xc0]
        mov edx,dword ptr [edx+8]
        fld dword ptr [edx] // 00af928e
        pop edx
        fstp dword ptr [esp + 0x4] // 00af9294
        fldz // 00af9298
        fstp dword ptr [esp] // 00af929a
        mov edx,dword ptr [esp+0xbc]
        mov edx,dword ptr [edx+4]
        call native_particle_random_range_00bd2f10 // 00af929d
        fstp dword ptr [esp + 0x10] // 00af92a2
        fld dword ptr [esp + 0x10] // 00af92a6
        xorps xmm0,xmm0 // 00af92aa
        fstp dword ptr [edi + 0x50] // 00af92ad
        mov eax,dword ptr [ebp + 0x2c] // 00af92b0
        movss xmm1,dword ptr [eax] // 00af92b3
        ucomiss xmm1,xmm0 // 00af92b7
        lahf // 00af92ba
        test ah,0x44 // 00af92bb
        movss dword ptr [esp + 0x14],xmm1 // 00af92be
        jp l_00af92d6 // 00af92c4
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+8]
        movss xmm1,dword ptr [edx] // 00af92c6
        pop edx
        movss dword ptr [esp + 0x10],xmm1 // 00af92ce
        jmp l_00af92fc // 00af92d4
    l_00af92d6:
        xor ecx,ecx // 00af92d6
        mov edx,dword ptr [esp+0xb4]
        mov edx,dword ptr [edx]
        call random_thread_bridge // 00af92d8
        xorps xmm0,xmm0 // 00af92dd
        mov dword ptr [esp + 0x10],eax // 00af92e0
        fild dword ptr [esp + 0x10] // 00af92e4
        fmul dword ptr [esp + 0x14] // 00af92e8
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+12]
        fmul qword ptr [edx] // 00af92ec
        pop edx
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+16]
        fadd qword ptr [edx] // 00af92f2
        pop edx
        fstp dword ptr [esp + 0x10] // 00af92f8
    l_00af92fc:
        fld dword ptr [edi + 0x18] // 00af92fc
        fld dword ptr [esp + 0x10] // 00af92ff
        fld st(0) // 00af9303
        fmulp st(2),st(0) // 00af9305
        fxch // 00af9307
        fstp dword ptr [edi + 0x18] // 00af9309
        fld dword ptr [edi + 0x1c] // 00af930c
        fmul st(0),st(1) // 00af930f
        fstp dword ptr [edi + 0x1c] // 00af9311
        fmul dword ptr [edi + 0x20] // 00af9314
        fstp dword ptr [edi + 0x20] // 00af9317
        mov eax,dword ptr [ebp + 0x30] // 00af931a
        movss xmm1,dword ptr [eax] // 00af931d
        ucomiss xmm1,xmm0 // 00af9321
        lahf // 00af9324
        test ah,0x44 // 00af9325
        movss dword ptr [esp + 0x14],xmm1 // 00af9328
        jp l_00af933a // 00af932e
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+8]
        movss xmm1,dword ptr [edx] // 00af9330
        pop edx
        jmp l_00af9366 // 00af9338
    l_00af933a:
        xor ecx,ecx // 00af933a
        mov edx,dword ptr [esp+0xb4]
        mov edx,dword ptr [edx]
        call random_thread_bridge // 00af933c
        xorps xmm0,xmm0 // 00af9341
        mov dword ptr [esp + 0x10],eax // 00af9344
        fild dword ptr [esp + 0x10] // 00af9348
        fmul dword ptr [esp + 0x14] // 00af934c
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+12]
        fmul qword ptr [edx] // 00af9350
        pop edx
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+16]
        fadd qword ptr [edx] // 00af9356
        pop edx
        fstp dword ptr [esp + 0x10] // 00af935c
        movss xmm1,dword ptr [esp + 0x10] // 00af9360
    l_00af9366:
        movss dword ptr [edi + 0x48],xmm1 // 00af9366
        mov eax,dword ptr [ebp + 0x34] // 00af936b
        movss xmm1,dword ptr [eax] // 00af936e
        ucomiss xmm1,xmm0 // 00af9372
        lahf // 00af9375
        test ah,0x44 // 00af9376
        movss dword ptr [esp + 0x14],xmm1 // 00af9379
        jp l_00af938b // 00af937f
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+8]
        movss xmm1,dword ptr [edx] // 00af9381
        pop edx
        jmp l_00af93b7 // 00af9389
    l_00af938b:
        xor ecx,ecx // 00af938b
        mov edx,dword ptr [esp+0xb4]
        mov edx,dword ptr [edx]
        call random_thread_bridge // 00af938d
        xorps xmm0,xmm0 // 00af9392
        mov dword ptr [esp + 0x10],eax // 00af9395
        fild dword ptr [esp + 0x10] // 00af9399
        fmul dword ptr [esp + 0x14] // 00af939d
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+12]
        fmul qword ptr [edx] // 00af93a1
        pop edx
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+16]
        fadd qword ptr [edx] // 00af93a7
        pop edx
        fstp dword ptr [esp + 0x10] // 00af93ad
        movss xmm1,dword ptr [esp + 0x10] // 00af93b1
    l_00af93b7:
        movss dword ptr [edi + 0x4c],xmm1 // 00af93b7
        mov eax,dword ptr [ebp + 0x80] // 00af93bc
        movss xmm1,dword ptr [eax] // 00af93c2
        ucomiss xmm1,xmm0 // 00af93c6
        lahf // 00af93c9
        test ah,0x44 // 00af93ca
        movss dword ptr [esp + 0x14],xmm1 // 00af93cd
        jp l_00af93e5 // 00af93d3
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+8]
        movss xmm1,dword ptr [edx] // 00af93d5
        pop edx
        movss dword ptr [esp + 0x10],xmm1 // 00af93dd
        jmp l_00af940b // 00af93e3
    l_00af93e5:
        xor ecx,ecx // 00af93e5
        mov edx,dword ptr [esp+0xb4]
        mov edx,dword ptr [edx]
        call random_thread_bridge // 00af93e7
        xorps xmm0,xmm0 // 00af93ec
        mov dword ptr [esp + 0x10],eax // 00af93ef
        fild dword ptr [esp + 0x10] // 00af93f3
        fmul dword ptr [esp + 0x14] // 00af93f7
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+12]
        fmul qword ptr [edx] // 00af93fb
        pop edx
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+16]
        fadd qword ptr [edx] // 00af9401
        pop edx
        fstp dword ptr [esp + 0x10] // 00af9407
    l_00af940b:
        cmp byte ptr [ebp + 0x29],0x0 // 00af940b
        jz l_00af944c // 00af940f
        xor ecx,ecx // 00af9411
        mov edx,dword ptr [esp+0xb4]
        mov edx,dword ptr [edx+4]
        call native_particle_unit_random_00bd2f40 // 00af9413
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx+12]
        fld dword ptr [edx] // 00af9418
        pop edx
        fxch // 00af941e
        fcomip st(0),st(1) // 00af9420
        fstp st(0) // 00af9422
        jc l_00af9439 // 00af9424
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+8]
        movss xmm0,dword ptr [edx] // 00af9426
        pop edx
        movss dword ptr [esp + 0x14],xmm0 // 00af942e
        xorps xmm0,xmm0 // 00af9434
        jmp l_00af945a // 00af9437
    l_00af9439:
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx+16]
        movss xmm0,dword ptr [edx] // 00af9439
        pop edx
        movss dword ptr [esp + 0x14],xmm0 // 00af9441
        xorps xmm0,xmm0 // 00af9447
        jmp l_00af945a // 00af944a
    l_00af944c:
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+8]
        movss xmm1,dword ptr [edx] // 00af944c
        pop edx
        movss dword ptr [esp + 0x14],xmm1 // 00af9454
    l_00af945a:
        fld dword ptr [esp + 0x14] // 00af945a
        fmul dword ptr [esp + 0x10] // 00af945e
        fstp dword ptr [edi + 0x54] // 00af9462
        mov ecx,dword ptr [ebp + 0x84] // 00af9465
        cmp word ptr [ecx + 0xa],0x0 // 00af946b
        jnz l_00af947f // 00af9470
        movss xmm1,dword ptr [ecx + 0x4] // 00af9472
        movss dword ptr [esp + 0x14],xmm1 // 00af9477
        jmp l_00af949f // 00af947d
    l_00af947f:
        cmp word ptr [ecx + 0xa],0x1 // 00af947f
        fldz // 00af9484
        push ecx // 00af9486
        fstp dword ptr [esp] // 00af9487
        jnz l_00af9493 // 00af948a
        call evaluate_native_particle_linear_curve_00affa70 // 00af948c
        jmp l_00af9498 // 00af9491
    l_00af9493:
        call evaluate_native_particle_cubic_curve_00affae0 // 00af9493
    l_00af9498:
        xorps xmm0,xmm0 // 00af9498
        fstp dword ptr [esp + 0x14] // 00af949b
    l_00af949f:
        movss xmm1,dword ptr [esp + 0x14] // 00af949f
        mov ebx,dword ptr [esp + 0xc8] // 00af94a5
        movss dword ptr [edi + 0x58],xmm1 // 00af94ac
        mov eax,dword ptr [ebp + 0x90] // 00af94b1
        test eax,eax // 00af94b7
        jz l_00af95a8 // 00af94b9
        fldz // 00af94bf
        sub esp,0x8 // 00af94c1
        cvtsi2ss xmm0,eax // 00af94c4
        movss dword ptr [esp + 0x4],xmm0 // 00af94c8
        fstp dword ptr [esp] // 00af94ce
        xor ecx,ecx // 00af94d1
        mov edx,dword ptr [esp+0xbc]
        mov edx,dword ptr [edx+4]
        call native_particle_random_range_00bd2f10 // 00af94d3
        mov eax,dword ptr [esp+0xb4]
        call dword ptr [eax+52] // 00af94d8
        fld1 // 00af94dd
        mov edx,dword ptr [ebp + 0x8c] // 00af94df
        mov ecx,dword ptr [edx + eax*0x4] // 00af94e5
        mov eax,dword ptr [ecx] // 00af94e8
        mov edx,dword ptr [eax + 0x8] // 00af94ea
        push dword ptr [esp+0xb4] // borrowed access follows original virtual08 arguments
        push ecx // 00af94ed
        fstp dword ptr [esp] // 00af94ee
        push 0x0 // 00af94f1
        call resource_bridge // 00af94f3
        push 0x40 // 00af94f5
        mov dword ptr [edi + 0x34],eax // 00af94f7
        mov eax,dword ptr [esp+0xb8]
        call dword ptr [eax+44] // 00af94fa
        add esp,0x4 // 00af94ff
        mov dword ptr [esp + 0x18],eax // 00af9502
        test eax,eax // 00af9506
        mov dword ptr [esp + 0xbc],0x6 // 00af9508
        jz l_00af951e // 00af9513
        mov ecx,eax // 00af9515
        mov edx,dword ptr [esp+0xb4]
        call construct_native_particle_object_matrix_00af8440 // 00af9517
        jmp l_00af9520 // 00af951c
    l_00af951e:
        xor eax,eax // 00af951e
    l_00af9520:
        mov dword ptr [esp + 0xbc],esi // 00af9520
        mov dword ptr [edi + 0x30],eax // 00af9527
        mov ecx,edi
        mov edx,dword ptr [esp+0xb4]
        push ebx
        call roots_bridge // 00af952a
        xorps xmm0,xmm0 // 00af9588
    l_00af958b:
        mov eax,dword ptr [ebp + 0x14] // 00af958b
        cmp byte ptr [eax + 0x64],0x0 // 00af958e
        jnz l_00af95b2 // 00af9592
        cmp byte ptr [eax + 0x65],0x0 // 00af9594
        jnz l_00af95b2 // 00af9598
        lea edx,[esp + 0x74] // 00af959a
        push edx // 00af959e
        mov ecx,ebx // 00af959f
        call compose_native_particle_record_matrix_00af8270 // 00af95a1
        jmp l_00af961e // 00af95a6
    l_00af95a8:
        xor eax,eax // 00af95a8
        mov dword ptr [edi + 0x34],eax // 00af95aa
        mov dword ptr [edi + 0x30],eax // 00af95ad
        jmp l_00af958b // 00af95b0
    l_00af95b2:
        push edx
        mov edx,dword ptr [esp+0xb8]
        mov edx,dword ptr [edx]
        mov edx,dword ptr [edx+8]
        movss xmm1,dword ptr [edx] // 00af95b2
        pop edx
        movss dword ptr [esp + 0x34],xmm1 // 00af95ba
        movss dword ptr [esp + 0x38],xmm0 // 00af95c0
        movss dword ptr [esp + 0x3c],xmm0 // 00af95c6
        movss dword ptr [esp + 0x40],xmm0 // 00af95cc
        movss dword ptr [esp + 0x44],xmm0 // 00af95d2
        movss dword ptr [esp + 0x48],xmm1 // 00af95d8
        movss dword ptr [esp + 0x4c],xmm0 // 00af95de
        movss dword ptr [esp + 0x50],xmm0 // 00af95e4
        movss dword ptr [esp + 0x54],xmm0 // 00af95ea
        movss dword ptr [esp + 0x58],xmm0 // 00af95f0
        movss dword ptr [esp + 0x5c],xmm1 // 00af95f6
        movss dword ptr [esp + 0x60],xmm0 // 00af95fc
        movss dword ptr [esp + 0x64],xmm0 // 00af9602
        movss dword ptr [esp + 0x68],xmm0 // 00af9608
        movss dword ptr [esp + 0x6c],xmm0 // 00af960e
        movss dword ptr [esp + 0x70],xmm1 // 00af9614
        lea eax,[esp + 0x34] // 00af961a
    l_00af961e:
        push dword ptr [esp+0xb4] // borrowed access follows original virtual28 arguments
        fldz // 00af961e
        mov edx,dword ptr [ebp] // 00af9620
        push ecx // 00af9623
        fst dword ptr [esp] // 00af9624
        push eax // 00af9627
        mov edx,dword ptr [edx+0x28] // 00af9628
        push 0x0 // 00af962b
        push ecx // 00af962d
        fstp dword ptr [esp] // 00af962e
        push edi // 00af9631
        mov ecx,ebp // 00af9632
        call virtual28_bridge // 00af9634
        pop edi // 00af963d
        pop esi // 00af963e
        pop ebp // 00af963f
        pop ebx // 00af9640
        add esp,0xb0 // 00af9648
        ret 0x8 // 00af964e
    }
}
} // namespace bsp
