#include "bsp/native_particle_sprite_state.hpp"
#include "bsp/native_particle_model_update.hpp"
#include "bsp/native_particle_emission_state.hpp"
#include "bsp/native_point_light_owner.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include <cstddef>
#include <cstring>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle Sprite state requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(offsetof(NativeParticleSpriteStateAccess, type) == 0);
static_assert(offsetof(NativeParticleTypeStateAccess, random) == 0);
static_assert(offsetof(NativeParticleTypeStateAccess, zero_00d7a218) == 4);
static_assert(offsetof(NativeParticleTypeStateAccess, one_00d7a24c) == 8);
static_assert(offsetof(NativeParticleTypeStateAccess, random_scale_00d5da30) == 12);
static_assert(offsetof(NativeParticleTypeStateAccess, base_00d7a210) == 16);
static_assert(offsetof(NativeParticleTypeStateAccess, angle_scale_00d5daf8) == 20);
static_assert(offsetof(NativeParticleSpriteStateAccess, unit) == 28);
static_assert(offsetof(NativeParticleSpriteStateAccess, sign_threshold_00ce3800) == 40);
static_assert(offsetof(NativeParticleSpriteStateAccess, negative_one_00d7a260) == 44);
static_assert(offsetof(NativeParticleSpriteStateAccess, sprite_counter_00f8d388) == 48);
std::uint32_t __fastcall random_bridge(RandomStream stream, const NativeParticleSpriteStateAccess* access) {
    return access->type.random->next_00bd2fc0(stream);
}
template<class T> T load(const void* p, std::uint32_t offset) noexcept {
    T value; std::memcpy(&value, static_cast<const std::byte*>(p)+offset, sizeof value);
    return value;
}
void publish_light(void* state, void* light) noexcept {
    std::memcpy(static_cast<std::byte*>(state)+0x60, &light, sizeof light);
}
struct PopulationGuard {std::uint32_t profile; TrackedCriticalSection* section;};
static_assert(sizeof(PopulationGuard)==8);
void enter(TrackedCriticalSection* section) {
    if (section) {
        EnterCriticalSection(&section->native);
        auto& depth=*reinterpret_cast<volatile std::uint32_t*>(&section->depth);
        depth=depth+1u;
    }
}
void leave(TrackedCriticalSection* section) noexcept {
    if (section) {
        auto& depth=*reinterpret_cast<volatile std::uint32_t*>(&section->depth);
        depth=depth-1u;
        LeaveCriticalSection(&section->native);
    }
}
// B08F91..B09095. Native states1/2/3 arm slot, prefix, concatenation in that
// order; state4 transfers the constructed slot before temporary releases.
// Existing adoption registers physical identities only. Its C++ host-binding
// failure consumes/reclaims the fresh construction, outside native FH3 proof.
void __fastcall initialize_light(void* definition, const NativeParticleSpriteStateAccess* access,
    void* state) {
    const auto& light_access=*access->lights;
    auto& environment=light_access.environment;
    auto& strings=light_access.strings;
    auto* lock=get_native_particle_population_lock_0072b740(
        light_access.actual_manager_01090aa0, light_access.actual_lock_0108ff50);
    auto* const section=lock->section_04;
    PopulationGuard guard{0x00ce37fcu,section};
    enter(section);
    try {
        void* raw=environment.pool_0109011c.allocate_raw_slot_00b7b810();
        if (!raw) {
            publish_light(state,nullptr);
        } else {
            NativeString prefix,joined;
            bool slot_owned=true, prefix_live=false, joined_live=false;
            try {
                construct_native_string_cstring_0041e870(&prefix,"dynamic_light_",strings);
                prefix_live=true;
                const auto* parent=load<const std::byte*>(definition,0x14);
                concatenate_native_string_headers_004261a0(&prefix,&joined,parent+8,strings);
                joined_live=true;
                auto storage=construct_native_point_light_00b7c710(raw,
                    NativePointLightPool::slot_bytes,joined,environment.nodes.strings);
                slot_owned=false;
                auto* reference=adopt_constructed_native_point_light(environment,storage);
                publish_light(state,&reference->light_owner().node.storage);
            } catch (...) {
                if (joined_live) destroy_native_string_header_0041dd20(&joined,strings);
                if (prefix_live) destroy_native_string_header_0041dd20(&prefix,strings);
                if (slot_owned) environment.pool_0109011c.return_raw_slot_00b7b1d0(raw);
                throw;
            }
            destroy_native_string_header_0041dd20(&joined,strings);
            destroy_native_string_header_0041dd20(&prefix,strings);
        }
        leave(section);
    } catch (...) {
        destroy_native_singleton_guard_00411ee0(&guard);
        throw;
    }
}

// Native B0909B..B094DC with its original scratch/argument offsets and FP
// instructions. EBP borrows access except around the two native frame draws.
// Light/FH3 lifetime work is composed by initialize_light before this body.
__declspec(naked) void __fastcall initialize_math(void*, const NativeParticleSpriteStateAccess*, void*, const void*) {
    __asm {
        sub esp,0x2c
        push ebx
        push ebp
        push esi
        push edi
        mov ebp,edx
        mov esi,ecx
        mov edi,dword ptr [esp+0x40]
        mov edx,dword ptr [esi + 0x1c] // 00b0909b
        movss xmm0,dword ptr [edx] // 00b0909e
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b090a2
        pop edx
        lahf // 00b090a9
        test ah,0x44 // 00b090aa
        movss dword ptr [esp + 0x10],xmm0 // 00b090ad
        jp l_00b090c5 // 00b090b3
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b090b5
        pop edx
        movss dword ptr [esp + 0x40],xmm0 // 00b090bd
        jmp l_00b090e8 // 00b090c3
    l_00b090c5:
        xor ecx,ecx // 00b090c5
        mov edx,ebp
        call random_bridge // 00b090c7
        mov dword ptr [esp + 0x40],eax // 00b090cc
        fild dword ptr [esp + 0x40] // 00b090d0
        fmul dword ptr [esp + 0x10] // 00b090d4
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b090d8
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b090de
        pop edx
        fstp dword ptr [esp + 0x40] // 00b090e4
    l_00b090e8:
        mov ecx,dword ptr [esi + 0x1c] // 00b090e8
        fldz // 00b090eb
        movzx eax,word ptr [ecx + 0xa] // 00b090ed
        test ax,ax // 00b090f1
        jnz l_00b09105 // 00b090f4
        movss xmm0,dword ptr [ecx + 0x4] // 00b090f6
        fstp st(0) // 00b090fb
        movss dword ptr [esp + 0x10],xmm0 // 00b090fd
        jmp l_00b0911f // 00b09103
    l_00b09105:
        cmp ax,0x1 // 00b09105
        push ecx // 00b09109
        fstp dword ptr [esp] // 00b0910a
        jnz l_00b09116 // 00b0910d
        call evaluate_native_particle_linear_curve_00affa70 // 00b0910f
        jmp l_00b0911b // 00b09114
    l_00b09116:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b09116
    l_00b0911b:
        fstp dword ptr [esp + 0x10] // 00b0911b
    l_00b0911f:
        fld dword ptr [esp + 0x10] // 00b0911f
        fmul dword ptr [esp + 0x40] // 00b09123
        fstp dword ptr [edi + 0x44] // 00b09127
        mov eax,dword ptr [esi + 0x80] // 00b0912a
        movss xmm0,dword ptr [eax] // 00b09130
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b09134
        pop edx
        lahf // 00b0913b
        test ah,0x44 // 00b0913c
        movss dword ptr [esp + 0x10],xmm0 // 00b0913f
        jp l_00b09157 // 00b09145
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b09147
        pop edx
        movss dword ptr [esp + 0x40],xmm0 // 00b0914f
        jmp l_00b0917a // 00b09155
    l_00b09157:
        xor ecx,ecx // 00b09157
        mov edx,ebp
        call random_bridge // 00b09159
        mov dword ptr [esp + 0x40],eax // 00b0915e
        fild dword ptr [esp + 0x40] // 00b09162
        fmul dword ptr [esp + 0x10] // 00b09166
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b0916a
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b09170
        pop edx
        fstp dword ptr [esp + 0x40] // 00b09176
    l_00b0917a:
        mov ecx,dword ptr [esi + 0x80] // 00b0917a
        movzx eax,word ptr [ecx + 0xa] // 00b09180
        test ax,ax // 00b09184
        jnz l_00b09196 // 00b09187
        movss xmm0,dword ptr [ecx + 0x4] // 00b09189
        movss dword ptr [esp + 0x10],xmm0 // 00b0918e
        jmp l_00b091b2 // 00b09194
    l_00b09196:
        cmp ax,0x1 // 00b09196
        fldz // 00b0919a
        push ecx // 00b0919c
        fstp dword ptr [esp] // 00b0919d
        jnz l_00b091a9 // 00b091a0
        call evaluate_native_particle_linear_curve_00affa70 // 00b091a2
        jmp l_00b091ae // 00b091a7
    l_00b091a9:
        call evaluate_native_particle_cubic_curve_00affae0 // 00b091a9
    l_00b091ae:
        fstp dword ptr [esp + 0x10] // 00b091ae
    l_00b091b2:
        fld dword ptr [esp + 0x10] // 00b091b2
        push edx
        mov edx,dword ptr [ebp+20] // borrowed current global
        fmul qword ptr [edx] // 00b091b6
        pop edx
        fmul dword ptr [esp + 0x40] // 00b091bc
        fstp dword ptr [edi + 0x50] // 00b091c0
        mov ecx,dword ptr [esi + 0x2c] // 00b091c3
        movss xmm0,dword ptr [ecx] // 00b091c6
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b091ca
        pop edx
        lahf // 00b091d1
        test ah,0x44 // 00b091d2
        movss dword ptr [esp + 0x10],xmm0 // 00b091d5
        jp l_00b091ed // 00b091db
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b091dd
        pop edx
        movss dword ptr [esp + 0x40],xmm0 // 00b091e5
        jmp l_00b09210 // 00b091eb
    l_00b091ed:
        xor ecx,ecx // 00b091ed
        mov edx,ebp
        call random_bridge // 00b091ef
        mov dword ptr [esp + 0x40],eax // 00b091f4
        fild dword ptr [esp + 0x40] // 00b091f8
        fmul dword ptr [esp + 0x10] // 00b091fc
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b09200
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b09206
        pop edx
        fstp dword ptr [esp + 0x40] // 00b0920c
    l_00b09210:
        fld dword ptr [edi + 0x18] // 00b09210
        fld dword ptr [esp + 0x40] // 00b09213
        fld st(0) // 00b09217
        fmulp st(2),st(0) // 00b09219
        fxch // 00b0921b
        fstp dword ptr [edi + 0x18] // 00b0921d
        fld dword ptr [edi + 0x1c] // 00b09220
        fmul st(0),st(1) // 00b09223
        fstp dword ptr [edi + 0x1c] // 00b09225
        fmul dword ptr [edi + 0x20] // 00b09228
        fstp dword ptr [edi + 0x20] // 00b0922b
        mov edx,dword ptr [esi + 0x30] // 00b0922e
        movss xmm0,dword ptr [edx] // 00b09231
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b09235
        pop edx
        lahf // 00b0923c
        test ah,0x44 // 00b0923d
        movss dword ptr [esp + 0x10],xmm0 // 00b09240
        jp l_00b09252 // 00b09246
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b09248
        pop edx
        jmp l_00b0927b // 00b09250
    l_00b09252:
        xor ecx,ecx // 00b09252
        mov edx,ebp
        call random_bridge // 00b09254
        mov dword ptr [esp + 0x40],eax // 00b09259
        fild dword ptr [esp + 0x40] // 00b0925d
        fmul dword ptr [esp + 0x10] // 00b09261
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b09265
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b0926b
        pop edx
        fstp dword ptr [esp + 0x40] // 00b09271
        movss xmm0,dword ptr [esp + 0x40] // 00b09275
    l_00b0927b:
        movss dword ptr [edi + 0x48],xmm0 // 00b0927b
        mov eax,dword ptr [esi + 0x34] // 00b09280
        movss xmm0,dword ptr [eax] // 00b09283
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b09287
        pop edx
        lahf // 00b0928e
        test ah,0x44 // 00b0928f
        movss dword ptr [esp + 0x10],xmm0 // 00b09292
        jp l_00b092a4 // 00b09298
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b0929a
        pop edx
        jmp l_00b092cd // 00b092a2
    l_00b092a4:
        xor ecx,ecx // 00b092a4
        mov edx,ebp
        call random_bridge // 00b092a6
        mov dword ptr [esp + 0x40],eax // 00b092ab
        fild dword ptr [esp + 0x40] // 00b092af
        fmul dword ptr [esp + 0x10] // 00b092b3
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b092b7
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b092bd
        pop edx
        fstp dword ptr [esp + 0x40] // 00b092c3
        movss xmm0,dword ptr [esp + 0x40] // 00b092c7
    l_00b092cd:
        movss dword ptr [edi + 0x4c],xmm0 // 00b092cd
        mov ecx,dword ptr [esi + 0x84] // 00b092d2
        movss xmm0,dword ptr [ecx] // 00b092d8
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b092dc
        pop edx
        lahf // 00b092e3
        test ah,0x44 // 00b092e4
        movss dword ptr [esp + 0x10],xmm0 // 00b092e7
        jp l_00b092ff // 00b092ed
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b092ef
        pop edx
        movss dword ptr [esp + 0x10],xmm0 // 00b092f7
        jmp l_00b09322 // 00b092fd
    l_00b092ff:
        xor ecx,ecx // 00b092ff
        mov edx,ebp
        call random_bridge // 00b09301
        mov dword ptr [esp + 0x40],eax // 00b09306
        fild dword ptr [esp + 0x40] // 00b0930a
        fmul dword ptr [esp + 0x10] // 00b0930e
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b09312
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b09318
        pop edx
        fstp dword ptr [esp + 0x10] // 00b0931e
    l_00b09322:
        cmp byte ptr [esi + 0x29],0x0 // 00b09322
        jz l_00b09347 // 00b09326
        xor ecx,ecx // 00b09328
        lea edx,[ebp+28]
        call native_particle_unit_random_00bd2f40 // 00b0932a
        push edx
        mov edx,dword ptr [ebp+40] // borrowed current global
        fld dword ptr [edx] // 00b0932f
        pop edx
        fxch // 00b09335
        fcomip st(0),st(1) // 00b09337
        fstp st(0) // 00b09339
        jnc l_00b09347 // 00b0933b
        push edx
        mov edx,dword ptr [ebp+44] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b0933d
        pop edx
        jmp l_00b0934f // 00b09345
    l_00b09347:
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b09347
        pop edx
    l_00b0934f:
        movss dword ptr [esp + 0x40],xmm0 // 00b0934f
        fld dword ptr [esp + 0x40] // 00b09355
        fmul dword ptr [esp + 0x10] // 00b09359
        fstp dword ptr [edi + 0x54] // 00b0935d
        mov edx,dword ptr [esi + 0x88] // 00b09360
        movss xmm0,dword ptr [edx] // 00b09366
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b0936a
        pop edx
        lahf // 00b09371
        test ah,0x44 // 00b09372
        movss dword ptr [esp + 0x10],xmm0 // 00b09375
        jp l_00b09387 // 00b0937b
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b0937d
        pop edx
        jmp l_00b093b0 // 00b09385
    l_00b09387:
        xor ecx,ecx // 00b09387
        mov edx,ebp
        call random_bridge // 00b09389
        mov dword ptr [esp + 0x40],eax // 00b0938e
        fild dword ptr [esp + 0x40] // 00b09392
        fmul dword ptr [esp + 0x10] // 00b09396
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b0939a
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b093a0
        pop edx
        fstp dword ptr [esp + 0x40] // 00b093a6
        movss xmm0,dword ptr [esp + 0x40] // 00b093aa
    l_00b093b0:
        movss dword ptr [edi + 0x58],xmm0 // 00b093b0
        mov eax,dword ptr [esi + 0x38] // 00b093b5
        movss xmm0,dword ptr [eax] // 00b093b8
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b093bc
        pop edx
        lahf // 00b093c3
        test ah,0x44 // 00b093c4
        movss dword ptr [esp + 0x10],xmm0 // 00b093c7
        jp l_00b093d9 // 00b093cd
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b093cf
        pop edx
        jmp l_00b09402 // 00b093d7
    l_00b093d9:
        xor ecx,ecx // 00b093d9
        mov edx,ebp
        call random_bridge // 00b093db
        mov dword ptr [esp + 0x40],eax // 00b093e0
        fild dword ptr [esp + 0x40] // 00b093e4
        fmul dword ptr [esp + 0x10] // 00b093e8
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b093ec
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b093f2
        pop edx
        fstp dword ptr [esp + 0x40] // 00b093f8
        movss xmm0,dword ptr [esp + 0x40] // 00b093fc
    l_00b09402:
        movss dword ptr [edi + 0x5c],xmm0 // 00b09402
        mov ecx,dword ptr [esi + 0x5c] // 00b09407
        movss xmm0,dword ptr [ecx] // 00b0940a
        push edx
        mov edx,dword ptr [ebp+4] // borrowed current global
        ucomiss xmm0,dword ptr [edx] // 00b0940e
        pop edx
        lahf // 00b09415
        test ah,0x44 // 00b09416
        movss dword ptr [esp + 0x10],xmm0 // 00b09419
        jp l_00b0942b // 00b0941f
        push edx
        mov edx,dword ptr [ebp+8] // borrowed current global
        movss xmm0,dword ptr [edx] // 00b09421
        pop edx
        jmp l_00b09454 // 00b09429
    l_00b0942b:
        xor ecx,ecx // 00b0942b
        mov edx,ebp
        call random_bridge // 00b0942d
        mov dword ptr [esp + 0x40],eax // 00b09432
        fild dword ptr [esp + 0x40] // 00b09436
        fmul dword ptr [esp + 0x10] // 00b0943a
        push edx
        mov edx,dword ptr [ebp+12] // borrowed current global
        fmul qword ptr [edx] // 00b0943e
        pop edx
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b09444
        pop edx
        fstp dword ptr [esp + 0x40] // 00b0944a
        movss xmm0,dword ptr [esp + 0x40] // 00b0944e
    l_00b09454:
        movss dword ptr [edi + 0x3c],xmm0 // 00b09454
        cmp byte ptr [esi + 0x60],0x0 // 00b09459
        jz l_00b0947d // 00b0945d
        push ebp // save borrowed access during native EBP range
        mov ebp,dword ptr [esi + 0x54] // 00b0945f
        mov ebx,dword ptr [esi + 0x50] // 00b09462
        xor ecx,ecx // 00b09465
        mov edx,dword ptr [esp]
        call random_bridge // 00b09467
        sub ebp,ebx // 00b0946c
        add ebp,0x1 // 00b0946e
        xor edx,edx // 00b09471
        div ebp // 00b09473
        add edx,ebx // 00b09475
        cvtsi2ss xmm0,edx // 00b09477
        pop ebp
        jmp l_00b09482 // 00b0947b
    l_00b0947d:
        cvtsi2ss xmm0,dword ptr [esi + 0x50] // 00b0947d
    l_00b09482:
        movss dword ptr [edi + 0x34],xmm0 // 00b09482
        movss dword ptr [edi + 0x30],xmm0 // 00b09487
        cmp byte ptr [esi + 0x61],0x0 // 00b0948c
        jz l_00b094b5 // 00b09490
        push ebp // save borrowed access during native EBP range
        mov ebp,dword ptr [esi + 0x54] // 00b09492
        mov esi,dword ptr [esi + 0x50] // 00b09495
        xor ecx,ecx // 00b09498
        mov edx,dword ptr [esp]
        call random_bridge // 00b0949a
        sub ebp,esi // 00b0949f
        add ebp,0x1 // 00b094a1
        xor edx,edx // 00b094a4
        div ebp // 00b094a6
        add edx,esi // 00b094a8
        cvtsi2ss xmm0,edx // 00b094aa
        pop ebp
        movss dword ptr [edi + 0x38],xmm0 // 00b094ae
        jmp l_00b094c1 // 00b094b3
    l_00b094b5:
        fld dword ptr [edi + 0x30] // 00b094b5
        push edx
        mov edx,dword ptr [ebp+16] // borrowed current global
        fadd qword ptr [edx] // 00b094b8
        pop edx
        fstp dword ptr [edi + 0x38] // 00b094be
    l_00b094c1:
        push edx
        mov edx,dword ptr [ebp+48] // borrowed current global
        add dword ptr [edx],0x1 // 00b094c5
        pop edx
        pop edi // 00b094cc
        pop esi // 00b094cd
        pop ebp // 00b094ce
        pop ebx // 00b094cf
        add esp,0x2c // 00b094d7
        ret 0x8 // 00b094da
    }
}
} // namespace

__declspec(naked) void __fastcall initialize_native_particle_sprite_state_00b08f60(
    void*, const NativeParticleSpriteStateAccess*, void*, const void*) {
    __asm {
        cmp byte ptr [ecx+0x64],0 // B08F83: disabled path never reads services
        jz math
        push ecx
        push edx
        push dword ptr [esp+12]
        call initialize_light
        pop edx
        pop ecx
    math:
        jmp initialize_math
    }
}
__declspec(naked) std::uint32_t __fastcall release_native_particle_sprite_state_00b007f0(
    void*, const NativeParticleSpriteStateAccess*, void*, std::uint32_t) {
    __asm {
        mov eax,1 // B007F0
        push edx
        mov edx,dword ptr [edx+48]
        sub dword ptr [edx],eax // B007F5
        pop edx
        ret 8 // B007FB
    }
}
} // namespace bsp
