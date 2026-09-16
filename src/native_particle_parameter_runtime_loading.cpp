#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/native_weak_owner.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle parameter runtime loading requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Bindings=NativeParticleParameterLoadingBindings;
using RawContext=NativeParticleParameterRuntimeRawContext;
using Builder=NativeParticleParameterBuilderStorage;
struct Key { float x,y,incoming[2],outgoing[2]; std::uint32_t kind; std::byte coefficients[16]; };
static_assert(sizeof(Key)==0x2c && sizeof(RawContext)==20 && sizeof(Bindings)==68);
struct RuntimeNumericView {
    const volatile double* const* tangent_scale;
    const volatile float* const* default_slope;
    const volatile double* const* integral_half;
    const volatile double* const* integral_quarter;
};
static_assert(sizeof(RuntimeNumericView)==16);
static_assert(offsetof(Bindings,tangent_scale_00cf1450)==36 &&
    offsetof(Bindings,default_slope_00d5dca0)==40 &&
    offsetof(Bindings,integral_half_00d7a280)==44 &&
    offsetof(Bindings,integral_quarter_00d7a348)==48);
static_assert(offsetof(RawContext,tangent_scale_00cf1450)==4 &&
    offsetof(RawContext,default_slope_00d5dca0)==8 &&
    offsetof(RawContext,integral_half_00d7a280)==12 &&
    offsetof(RawContext,integral_quarter_00d7a348)==16);
template<class Context> RuntimeNumericView numeric_view(Context& a) noexcept {
    return {&a.tangent_scale_00cf1450,&a.default_slope_00d5dca0,
        &a.integral_half_00d7a280,&a.integral_quarter_00d7a348};
}
template<class T> T read(const void* p,std::size_t offset) noexcept {
    T v; std::memcpy(&v,static_cast<const std::byte*>(p)+offset,sizeof v); return v;
}
template<class T> void write(void* p,std::size_t offset,T v) noexcept {
    std::memcpy(static_cast<std::byte*>(p)+offset,&v,sizeof v);
}
Key* key(Builder& p,std::int32_t i) noexcept {
    return reinterpret_cast<Key*>(static_cast<std::byte*>(p.records_00)+static_cast<std::uint32_t>(i)*0x2cu);
}
void copy_words(void* destination,const void* source,std::size_t count) noexcept {
    for(std::size_t i=0;i<count;++i) write(destination,i*4,read<std::uint32_t>(source,i*4));
}
// Native 00AFB3A0..00AFB543; symbolic current-data loads only.
__declspec(naked) void __fastcall coefficients(void*,const RuntimeNumericView*,const void*) {
    __asm {
        sub esp, 034h // 00afb3a0
        cmp dword ptr [ecx + 018h], 2 // 00afb3a3
        movss xmm0, dword ptr [ecx] // 00afb3a7
        movss xmm1, dword ptr [ecx + 4] // 00afb3ab
        movss dword ptr [esp + 014h], xmm0 // 00afb3b0
        fld dword ptr [esp + 014h] // 00afb3b6
        xorps xmm0, xmm0 // 00afb3ba
        movss dword ptr [esp + 024h], xmm1 // 00afb3bd
        fld dword ptr [esp + 024h] // 00afb3c3
        push eax
        mov eax,[edx+0]
        mov eax,[eax]
        fld qword ptr [eax] // 00afb3c7
        pop eax
        je L_00afb3e7 // 00afb3cd
        fld dword ptr [ecx + 010h] // 00afb3cf
        fmul st(0), st(1) // 00afb3d2
        fadd st(0), st(3) // 00afb3d4
        fstp dword ptr [esp + 018h] // 00afb3d6
        fld dword ptr [ecx + 014h] // 00afb3da
        fmul st(0), st(1) // 00afb3dd
        fadd st(0), st(2) // 00afb3df
        fstp dword ptr [esp + 028h] // 00afb3e1
        jmp L_00afb3f3 // 00afb3e5
    L_00afb3e7:
        movss dword ptr [esp + 018h], xmm0 // 00afb3e7
        movss dword ptr [esp + 028h], xmm0 // 00afb3ed
    L_00afb3f3:
        mov eax, dword ptr [esp + 038h] // 00afb3f3
        cmp dword ptr [eax + 018h], 0 // 00afb3f7
        movss xmm2, dword ptr [eax] // 00afb3fb
        movss dword ptr [esp + 020h], xmm2 // 00afb3ff
        movss xmm2, dword ptr [eax + 4] // 00afb405
        fld dword ptr [esp + 020h] // 00afb40a
        movss dword ptr [esp + 030h], xmm2 // 00afb40e
        fld dword ptr [esp + 030h] // 00afb414
        je L_00afb438 // 00afb418
        fld st(1) // 00afb41a
        fld dword ptr [eax + 8] // 00afb41c
        fmul st(0), st(4) // 00afb41f
        fsubp st(1), st(0) // 00afb421
        fstp dword ptr [esp + 01ch] // 00afb423
        fld st(0) // 00afb427
        fld dword ptr [eax + 0ch] // 00afb429
        fmulp st(4), st(0) // 00afb42c
        fsubrp st(3), st(0) // 00afb42e
        fxch st(2) // 00afb430
        fstp dword ptr [esp + 02ch] // 00afb432
        jmp L_00afb446 // 00afb436
    L_00afb438:
        fstp st(2) // 00afb438
        movss dword ptr [esp + 01ch], xmm0 // 00afb43a
        movss dword ptr [esp + 02ch], xmm0 // 00afb440
    L_00afb446:
        fld st(0) // 00afb446
        push eax
        mov eax,[edx+4]
        mov eax,[eax]
        movss xmm0, dword ptr [eax] // 00afb448
        pop eax
        fsub st(0), st(4) // 00afb450
        movss dword ptr [esp + 4], xmm0 // 00afb452
        movss dword ptr [esp], xmm0 // 00afb458
        fstp dword ptr [esp + 8] // 00afb45d
        fld st(1) // 00afb461
        fsub st(0), st(3) // 00afb463
        fstp dword ptr [esp + 0ch] // 00afb465
        fld dword ptr [esp + 018h] // 00afb469
        fsubrp st(4), st(0) // 00afb46d
        fxch st(3) // 00afb46f
        fstp dword ptr [esp + 038h] // 00afb471
        fld dword ptr [esp + 038h] // 00afb475
        fld st(0) // 00afb479
        fldz // 00afb47b
        fld st(0) // 00afb47d
        fxch st(2) // 00afb47f
        fucomip st(0), st(2) // 00afb481
        fstp st(1) // 00afb483
        lahf // 00afb485
        test ah, 044h // 00afb486
        jnp L_00afb49a // 00afb489
        fld dword ptr [esp + 028h] // 00afb48b
        fsubrp st(4), st(0) // 00afb48f
        fxch st(3) // 00afb491
        fdivrp st(1), st(0) // 00afb493
        fstp dword ptr [esp] // 00afb495
        jmp L_00afb49e // 00afb498
    L_00afb49a:
        fstp st(1) // 00afb49a
        fstp st(2) // 00afb49c
    L_00afb49e:
        fld dword ptr [esp + 01ch] // 00afb49e
        fsubp st(3), st(0) // 00afb4a2
        fxch st(2) // 00afb4a4
        fstp dword ptr [esp + 038h] // 00afb4a6
        fld dword ptr [esp + 038h] // 00afb4aa
        fld st(0) // 00afb4ae
        fucomip st(0), st(2) // 00afb4b0
        fstp st(1) // 00afb4b2
        lahf // 00afb4b4
        test ah, 044h // 00afb4b5
        jnp L_00afb4c8 // 00afb4b8
        fld dword ptr [esp + 02ch] // 00afb4ba
        fsubp st(2), st(0) // 00afb4be
        fdivp st(1), st(0) // 00afb4c0
        fstp dword ptr [esp + 4] // 00afb4c2
        jmp L_00afb4cc // 00afb4c6
    L_00afb4c8:
        fstp st(0) // 00afb4c8
        fstp st(0) // 00afb4ca
    L_00afb4cc:
        fld dword ptr [esp + 8] // 00afb4cc
        movss xmm0, dword ptr [esp] // 00afb4d0
        fld st(0) // 00afb4d5
        movss dword ptr [ecx + 024h], xmm0 // 00afb4d7
        fmul st(0), st(0) // 00afb4dc
        movss dword ptr [ecx + 028h], xmm1 // 00afb4de
        fld1 // 00afb4e3
        fdivrp st(1), st(0) // 00afb4e5
        fstp dword ptr [esp + 010h] // 00afb4e7
        fld dword ptr [esp] // 00afb4eb
        fmul st(0), st(1) // 00afb4ee
        fstp dword ptr [esp + 8] // 00afb4f0
        fld dword ptr [esp + 4] // 00afb4f4
        fmul st(0), st(1) // 00afb4f8
        fstp dword ptr [esp + 038h] // 00afb4fa
        fld dword ptr [esp + 038h] // 00afb4fe
        fld st(0) // 00afb502
        fld dword ptr [esp + 8] // 00afb504
        fld st(0) // 00afb508
        faddp st(2), st(0) // 00afb50a
        fld dword ptr [esp + 0ch] // 00afb50c
        fld st(0) // 00afb510
        fsubp st(3), st(0) // 00afb512
        fld st(0) // 00afb514
        fsubp st(3), st(0) // 00afb516
        fld dword ptr [esp + 010h] // 00afb518
        fld st(0) // 00afb51c
        fmulp st(4), st(0) // 00afb51e
        fxch st(3) // 00afb520
        fdivrp st(5), st(0) // 00afb522
        fxch st(4) // 00afb524
        fstp dword ptr [ecx + 01ch] // 00afb526
        fld st(3) // 00afb529
        fadd st(0), st(0) // 00afb52b
        faddp st(4), st(0) // 00afb52d
        fld st(0) // 00afb52f
        fsubp st(4), st(0) // 00afb531
        fsubp st(3), st(0) // 00afb533
        fxch st(2) // 00afb535
        fsubrp st(1), st(0) // 00afb537
        fmulp st(1), st(0) // 00afb539
        fstp dword ptr [ecx + 020h] // 00afb53b
        add esp, 034h // 00afb53e
        ret 4 // 00afb541
    }
}

// Native 00AFFCB0..00AFFD19; symbolic current-data loads only.
__declspec(naked) float __fastcall integral_hermite(const void*,const RuntimeNumericView*,float) {
    __asm {
        fld dword ptr [esp + 4] // 00affcb0
        mov ecx, dword ptr [ecx + 4] // 00affcb4
        fld st(0) // 00affcb7
        fld dword ptr [ecx + 4] // 00affcb9
        fxch st(1) // 00affcbc
        fcomi st(0), st(1) // 00affcbe
        fstp st(1) // 00affcc0
        jbe L_00affcd2 // 00affcc2
    L_00affcc4:
        fld dword ptr [ecx + 020h] // 00affcc4
        add ecx, 01ch // 00affcc7
        fxch st(1) // 00affcca
        fcomi st(0), st(1) // 00affccc
        fstp st(1) // 00affcce
        ja L_00affcc4 // 00affcd0
    L_00affcd2:
        fstp st(0) // 00affcd2
        fsub dword ptr [ecx] // 00affcd4
        fstp dword ptr [esp + 4] // 00affcd6
        fld dword ptr [ecx + 0ch] // 00affcda
        fld dword ptr [esp + 4] // 00affcdd
        fld st(0) // 00affce1
        fmulp st(2), st(0) // 00affce3
        fxch st(1) // 00affce5
        push eax
        mov eax,[edx+12]
        mov eax,[eax]
        fmul qword ptr [eax] // 00affce7
        pop eax
        fld dword ptr [ecx + 010h] // 00affced
        push eax
        mov eax,[edx+0]
        mov eax,[eax]
        fmul qword ptr [eax] // 00affcf0
        pop eax
        faddp st(1), st(0) // 00affcf6
        fmul st(0), st(1) // 00affcf8
        fld dword ptr [ecx + 014h] // 00affcfa
        push eax
        mov eax,[edx+8]
        mov eax,[eax]
        fmul qword ptr [eax] // 00affcfd
        pop eax
        faddp st(1), st(0) // 00affd03
        fmul st(0), st(1) // 00affd05
        fadd dword ptr [ecx + 018h] // 00affd07
        fmulp st(1), st(0) // 00affd0a
        fadd dword ptr [ecx + 8] // 00affd0c
        fstp dword ptr [esp + 4] // 00affd0f
        fld dword ptr [esp + 4] // 00affd13
        ret 4 // 00affd17
    }
}

// Native 00AFFD20..00AFFD6F; symbolic current-data loads only.
__declspec(naked) float __fastcall integral_linear(const void*,const RuntimeNumericView*,float) {
    __asm {
        fld dword ptr [esp + 4] // 00affd20
        mov ecx, dword ptr [ecx + 4] // 00affd24
        fld st(0) // 00affd27
        fld dword ptr [ecx + 4] // 00affd29
        fxch st(1) // 00affd2c
        fcomi st(0), st(1) // 00affd2e
        fstp st(1) // 00affd30
        jbe L_00affd42 // 00affd32
    L_00affd34:
        fld dword ptr [ecx + 018h] // 00affd34
        add ecx, 014h // 00affd37
        fxch st(1) // 00affd3a
        fcomi st(0), st(1) // 00affd3c
        fstp st(1) // 00affd3e
        ja L_00affd34 // 00affd40
    L_00affd42:
        fstp st(0) // 00affd42
        fsub dword ptr [ecx] // 00affd44
        fstp dword ptr [esp + 4] // 00affd46
        fld dword ptr [ecx + 0ch] // 00affd4a
        fld dword ptr [esp + 4] // 00affd4d
        fld st(0) // 00affd51
        fmulp st(2), st(0) // 00affd53
        fxch st(1) // 00affd55
        push eax
        mov eax,[edx+8]
        mov eax,[eax]
        fmul qword ptr [eax] // 00affd57
        pop eax
        fadd dword ptr [ecx + 8] // 00affd5d
        fmulp st(1), st(0) // 00affd60
        fadd dword ptr [ecx + 010h] // 00affd62
        fstp dword ptr [esp + 4] // 00affd65
        fld dword ptr [esp + 4] // 00affd69
        ret 4 // 00affd6d
    }
}

NativeWeakHandlePool& parameter_pool(Bindings& a) noexcept { return a.owners.parameter_pool_00f8d344; }
NativeWeakHandlePool& parameter_pool(RawContext& a) noexcept { return a.parameter_pool_00f8d344; }
void* allocate_segments(Bindings& a,std::uint32_t bytes) { return a.allocate_array_00bf55be(bytes); }
void* allocate_segments(RawContext&,std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});
}
void free_segments(Bindings& a,void* p) noexcept { a.owners.free_array_00bf6989(p); }
void free_segments(RawContext&,void* p) noexcept { singleton_lifetime_free(p); }

// B004A0 ignores incoming ECX/size and tail-calls the actual F8D344 owner.
template<class Context> void* allocate_slot_00b004a0(Context& a) {
    return parameter_pool(a).allocate_raw_slot_009242f0();
}
void* construct_payload_00aff9b0(void* p) noexcept {
    write(p,4,0u); write<std::uint8_t>(p,8,0); write<std::uint8_t>(p,9,0);
    write<std::uint16_t>(p,10,0);
    return p;
}
std::int32_t segment_count(const Builder& p) noexcept {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(p.count_04)-1u);
}
void compute_coefficients_00afbf20(Builder& p,const RuntimeNumericView& numeric) {
    for(std::int32_t i=0;i<segment_count(p);++i)
        coefficients(key(p,i),&numeric,key(p,i+1));
}
template<class Context> void allocate_segments_00affd70(void* p,std::uint8_t count,Context& a) {
    write(p,9,count);
    const auto kind=read<std::uint16_t>(p,10);
    if(kind!=1 && kind!=2) return;
    if(void* old=read<void*>(p,4)) {
        free_segments(a,old);
        write<void*>(p,4,nullptr);
    }
    // Kind/stride is captured before free; the capacity byte is reloaded after.
    const auto bytes=read<std::uint8_t>(p,9)*(kind==1?0x14u:0x1cu);
    write(p,4,allocate_segments(a,bytes));
}

// Append's native FLD32/FSTP32 argument, call, FSTP32 result schedule. Integer
// binding supplies the numeric view without adding any floating-point spill.
void integrated_end(const void* p,const RuntimeNumericView* numeric,
    const void* end,void* result,bool hermite) {
    __asm {
        mov ecx,p
        mov edx,numeric
        mov eax,end
        push ecx
        fld dword ptr[eax]
        fstp dword ptr[esp]
        cmp hermite,0
        je linear
        call integral_hermite
        jmp stored
    linear:
        call integral_linear
    stored:
        mov eax,result
        fstp dword ptr[eax]
    }
}
void append_segment(void* p,const void* segment,bool hermite,const RuntimeNumericView& numeric) {
    const auto stride=hermite?0x1cu:0x14u;
    const auto count=read<std::uint8_t>(p,8);
    std::uint32_t accumulated=0;
    if(count) {
        const auto* previous=static_cast<const std::byte*>(read<void*>(p,4))+
            (static_cast<std::uint32_t>(count)-1u)*stride;
        integrated_end(p,&numeric,previous+4,&accumulated,hermite);
    }
    // The first destination uses captured DL; the final accumulated field and
    // increment use current owner fields after the ordered payload copies.
    auto* destination=static_cast<std::byte*>(read<void*>(p,4))+static_cast<std::uint32_t>(count)*stride;
    copy_words(destination,segment,stride/4);
    destination=static_cast<std::byte*>(read<void*>(p,4))+static_cast<std::uint32_t>(read<std::uint8_t>(p,8))*stride;
    write(destination,hermite?8:16,accumulated);
    write(p,8,static_cast<std::uint8_t>(read<std::uint8_t>(p,8)+1u));
}
void linear_slope(const Key* first,const Key* second,void* destination) noexcept {
    __asm {
        mov eax,second
        mov ecx,first
        fld dword ptr[eax+4]
        fsub dword ptr[ecx+4]
        fld dword ptr[eax]
        fsub dword ptr[ecx]
        fdivp st(1),st(0)
        mov eax,destination
        fstp dword ptr[eax]
    }
}
void constant_value(void* parameter,const void* first) noexcept {
    __asm {
        mov ecx,first
        mov eax,parameter
        fld dword ptr[ecx+4]
        fstp dword ptr[eax+4]
    }
}
template<class Context> void* convert_parameter(void* raw,Context& a) {
    auto& p=*static_cast<Builder*>(raw);
    const auto numeric=numeric_view(a);
    void* allocation=allocate_slot_00b004a0(a);
    // DF30E8 state0/CBB160 owns only placement construction. Later failures
    // and unknown kind retain the allocated slot exactly as in AFBF60.
    struct SlotUnwind {
        void* allocation;
        NativeWeakHandlePool& pool;
        bool armed{true};
        ~SlotUnwind() noexcept {
            if(armed) return_native_particle_parameter_00b00090(allocation,pool);
        }
    } unwind{allocation,parameter_pool(a)};
    void* result=allocation?construct_payload_00aff9b0(allocation):nullptr;
    const auto kind=p.kind_0c;
    unwind.armed=false;
    if(kind==2) {
        write<std::uint16_t>(result,10,2);
        compute_coefficients_00afbf20(p,numeric);
        allocate_segments_00affd70(result,static_cast<std::uint8_t>(segment_count(p)),a);
        for(std::int32_t i=0;i<segment_count(p);++i) {
            const Key* first=key(p,i);
            std::uint32_t segment[7];
            segment[0]=read<std::uint32_t>(first,0); segment[1]=read<std::uint32_t>(first,0x2c); segment[2]=0;
            copy_words(segment+3,first->coefficients,4);
            append_segment(result,segment,true,numeric); // B000A0
        }
        return result;
    }
    if(kind==1) {
        write<std::uint16_t>(result,10,1);
        allocate_segments_00affd70(result,static_cast<std::uint8_t>(segment_count(p)),a);
        for(std::int32_t i=0;i<segment_count(p);++i) {
            const Key* first=key(p,i);
            const Key* second=key(p,i+1);
            std::uint32_t segment[5]{read<std::uint32_t>(first,0),read<std::uint32_t>(second,0),
                read<std::uint32_t>(first,4),0,0};
            linear_slope(first,second,segment+3);
            append_segment(result,segment,false,numeric); // B00120
        }
        return result;
    }
    if(kind==0) {
        write<std::uint16_t>(result,10,0);
        constant_value(result,p.records_00);
        return result;
    }
    return nullptr;
}
} // namespace

void* convert_native_particle_parameter_00afbf60(void* raw,Bindings& a) { return convert_parameter(raw,a); }
void* convert_native_particle_parameter_00afbf60(void* raw,RawContext& a) { return convert_parameter(raw,a); }

// Preserve the public legacy EDX binding and native ST(0) return. Build only
// addresses of live pointer members, with no C++ floating-point return wrapper.
#define BSP_PARAMETER_INTEGRAL_ENTRY(kernel,scale,slope,half,quarter) \
    __asm { push eax } \
    __asm { push edx } \
    __asm { sub esp,16 } \
    __asm { lea eax,[edx+scale] } \
    __asm { mov [esp],eax } \
    __asm { lea eax,[edx+slope] } \
    __asm { mov [esp+4],eax } \
    __asm { lea eax,[edx+half] } \
    __asm { mov [esp+8],eax } \
    __asm { lea eax,[edx+quarter] } \
    __asm { mov [esp+12],eax } \
    __asm { mov edx,esp } \
    __asm { push dword ptr[esp+28] } \
    __asm { call kernel } \
    __asm { lea esp,[esp+16] } \
    __asm { pop edx } \
    __asm { pop eax } \
    __asm { ret 4 }
__declspec(naked) float __fastcall integrate_native_particle_parameter_hermite_00affcb0(
    const void*,const Bindings*,float) { BSP_PARAMETER_INTEGRAL_ENTRY(integral_hermite,36,40,44,48) }
__declspec(naked) float __fastcall integrate_native_particle_parameter_linear_00affd20(
    const void*,const Bindings*,float) { BSP_PARAMETER_INTEGRAL_ENTRY(integral_linear,36,40,44,48) }
__declspec(naked) float __fastcall integrate_native_particle_parameter_hermite_00affcb0(
    const void*,const RawContext*,float) { BSP_PARAMETER_INTEGRAL_ENTRY(integral_hermite,4,8,12,16) }
__declspec(naked) float __fastcall integrate_native_particle_parameter_linear_00affd20(
    const void*,const RawContext*,float) { BSP_PARAMETER_INTEGRAL_ENTRY(integral_linear,4,8,12,16) }
#undef BSP_PARAMETER_INTEGRAL_ENTRY
} // namespace bsp

