#include "bsp/native_runtime_texture_creation.hpp"
#include "bsp/native_logical_texture_named_base.hpp"
#include "bsp/native_physical_buffer_owner.hpp"
#include "bsp/native_texture_surface_cache_storage.hpp"
#include "bsp/native_renderer_device_recreation_actual.hpp"
#include "bsp/native_render_resources_surfaces.hpp"
#include "bsp/native_cube_texture_owner_array_reserve.hpp"
#include <atomic>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4 && sizeof(D3DSURFACE_DESC)==32);
void* at(const void* p,Word offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+offset);
}
Word word(const volatile void* p) noexcept {
    Word result;
    __asm { mov eax,p }
    __asm { mov eax,[eax] }
    __asm { mov result,eax }
    return result;
}
void put(void* p,Word value) noexcept { *static_cast<volatile Word*>(p)=value; }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
template<class T> T method(void* p,Word offset) noexcept {
    return reinterpret_cast<T>(word(at(pointer(word(p)),offset)));
}
using ComWord=ULONG (__stdcall*)(void*);
using ComDesc=HRESULT (__stdcall*)(void*,UINT,D3DSURFACE_DESC*);
using Create=HRESULT (__stdcall*)(void*,UINT,UINT,UINT,DWORD,D3DFORMAT,D3DPOOL,IDirect3DTexture9**,HANDLE*);
void pair(void* captured) {
    (void)method<ComWord>(captured,4)(captured);
    (void)method<ComWord>(captured,8)(captured);
}
int cleanup_exception(unsigned long code) noexcept {
    if(code==0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_constructor(NativeRuntimeTextureConstructionContext& c,
    NativeRuntimeTextureConstructionAcquired& a) noexcept {
    __try {
        auto& strings=c.owners.renderer_notification.actual_string_storage;
        if(a.unwind_state>=3) {
            a.unwind_state=2;
            destroy_native_buffer_diagnostic_record_00b3f4c0(a.diagnostic,strings);
        }
        if(a.unwind_state>=2) {
            a.unwind_state=1;
            destroy_native_string_header_0041dd20(a.first_name,strings);
        }
        if(a.unwind_state>=1) {
            a.unwind_state=0;
            destroy_native_texture_surface_cache_00b3ec40(
                *static_cast<NativeTextureSurfaceCacheStorage*>(at(a.owner,0x40)));
        }
        if(a.unwind_state>=0) {
            a.unwind_state=-1;
            unwind_native_logical_texture_named_base_00b34010(a.owner,strings);
        }
    } __except(cleanup_exception(GetExceptionCode())) { __assume(0); }
}
void unwind_factory(NativeRuntimeTextureCreationContext& c,
    NativeRuntimeTextureCreationAcquired& a) noexcept {
    __try {
        if(a.unwind_state==1) {
            a.unwind_state=0;
            return_d3d9_texture2d_slot_00b3dcd0(a.raw_slot);
            a.raw_slot_returned=true;
        }
        if(a.unwind_state==0) {
            a.unwind_state=-1;
            destroy_native_renderer_optional_guard_00b21110(a.guard,c.synchronization);
        }
    } __except(cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct ConstructorCleanup {
    NativeRuntimeTextureConstructionContext& context;
    NativeRuntimeTextureConstructionAcquired& acquired;
    ~ConstructorCleanup() noexcept {
        if(acquired.phase!=NativeRuntimeTextureConstructionAcquired::Phase::complete) {
            acquired.phase=NativeRuntimeTextureConstructionAcquired::Phase::failed;
            if(acquired.unwind_state>=0)unwind_constructor(context,acquired);
        }
    }
};
struct FactoryCleanup {
    NativeRuntimeTextureCreationContext& context;
    NativeRuntimeTextureCreationAcquired& acquired;
    ~FactoryCleanup() noexcept {
        if(acquired.phase!=NativeRuntimeTextureCreationAcquired::Phase::complete) {
            acquired.phase=NativeRuntimeTextureCreationAcquired::Phase::failed;
            if(acquired.unwind_state>=0)unwind_factory(context,acquired);
        }
    }
};
void account(Word width,Word height,Word format,NativeRuntimeTextureCreationContext& c) noexcept {
    const Word storage_bits=native_format_storage_bits_00b21210(format);
    Word bytes=storage_bits>>3;
    float bytes_float=0.0f;
    const volatile float* const unsigned_fix=&c.unsigned_dword_fix_00ce3978;
    const volatile double* const counter_fix=&c.unsigned_counter_fix_00d57da0;
    volatile Word* const counter=&c.allocation_bytes_0108d4bc;
    if(storage_bits) {
        __asm {
            mov eax,bytes
            test eax,eax
            fild dword ptr bytes
            jns converted_bytes
            mov edx,unsigned_fix
            fadd dword ptr [edx]
        converted_bytes:
            fstp dword ptr bytes_float
        }
    }
    Word area=width*height;
    unsigned short saved,truncating;
    __int64 converted;
    __asm {
        mov eax,area
        test eax,eax
        fild dword ptr area
        jns converted_area
        mov edx,unsigned_fix
        fadd dword ptr [edx]
    converted_area:
        mov ecx,counter
        mov eax,[ecx]
        fmul dword ptr bytes_float
        test eax,eax
        fild dword ptr [ecx]
        jns converted_counter
        mov edx,counter_fix
        fadd qword ptr [edx]
    converted_counter:
        fnstcw saved
        movzx eax,saved
        faddp st(1),st(0)
        or eax,0c00h
        mov truncating,ax
        fldcw truncating
        fistp qword ptr converted
        mov eax,dword ptr converted
        mov [ecx],eax
        fldcw saved
    }
}
void append(void* renderer,void* owner) {
    void* const header=at(renderer,0x1b00);
    Word capacity=word(at(header,8));
    if(word(at(header,4))==capacity) {
        const auto doubled=static_cast<std::int32_t>(capacity+capacity);
        reserve_native_cube_texture_owner_array_00735ff0(header,doubled>1?doubled:1);
    }
    void* const slot=pointer(word(header)+word(at(header,4))*4u);
    if(slot)put(slot,reinterpret_cast<Word>(owner));
    put(at(header,4),word(at(header,4))+1u);
}
} // namespace

void* construct_native_logical_texture_unnamed_00b33fc0(void* owner,
    void* com,Word flags,Word& serial) {
    put(owner,0x00ceb130);put(owner,0x00d5f1f4);
    ::new(at(owner,4)) std::atomic<std::int32_t>(1);
    put(at(owner,8),0);put(at(owner,0xc),0);put(at(owner,0x14),0);
    put(at(owner,0x10),reinterpret_cast<Word>(com));put(at(owner,0x1c),flags);
    put(at(owner,0x20),word(&serial));put(&serial,word(&serial)+1u);
    put(owner,0x00d5f228);return owner;
}

void* construct_native_runtime_texture_2d_00b3f7b0(void* owner,
    IDirect3DTexture9* input,Word saved_width,Word saved_height,volatile Word flags,
    NativeRuntimeTextureConstructionContext& c,NativeRuntimeTextureConstructionAcquired& a) {
    if(a.phase!=NativeRuntimeTextureConstructionAcquired::Phase::fresh)
        throw std::invalid_argument("runtime texture constructor frame must be fresh");
    a.phase=NativeRuntimeTextureConstructionAcquired::Phase::running;a.owner=owner;
    ConstructorCleanup cleanup{c,a};
        const Word captured_flags=flags;
        construct_native_logical_texture_unnamed_00b33fc0(owner,input,captured_flags,c.owners.actual_shared_serial_0108d6e8);
        put(owner,0x00d61948);put(at(owner,0x24),0);put(at(owner,0x3c),0);a.unwind_state=0;
        put(at(owner,0x40),0);put(at(owner,0x44),0);put(at(owner,0x48),0);
        void* com=pointer(word(at(owner,0x10)));put(at(owner,0x4c),0);
        const auto retain=method<ComWord>(com,4);a.unwind_state=1;(void)retain(com);
        com=pointer(word(at(owner,0x10)));
        const Word levels=method<ComWord>(com,0x34)(com);
        put(at(owner,0x14),levels);put(at(owner,0x1c),captured_flags);
        (void)method<ComDesc>(input,0x44)(input,0,reinterpret_cast<D3DSURFACE_DESC*>(a.descriptor));
        const Word height=word(a.descriptor+0x1c),width=word(a.descriptor+0x18),format=word(a.descriptor);
        put(at(owner,0x2c),height);put(at(owner,0x28),width);
        put(at(owner,0x38),saved_height);put(at(owner,0x18),format);put(at(owner,0x34),saved_width);put(at(owner,0x30),0);
        auto& strings=c.owners.renderer_notification.actual_string_storage;
        put(a.first_name,0);put(a.first_name+4,0);
        resize_native_string_header_0041dd40(a.first_name,strings,0x10,true);
        void* const first_data=pointer(word(a.first_name+4));
        if(first_data)std::memmove(first_data,c.actual_handmade_texture_literal_00d619e4,word(a.first_name)+1u);
        put(a.diagnostic,reinterpret_cast<Word>(input));
        const Word first_length=word(a.first_name);a.unwind_state=2;
        put(a.diagnostic+4,0);put(a.diagnostic+8,0);
        resize_native_string_header_0041dd40(a.diagnostic+4,strings,first_length,true);
        void* const second_data=pointer(word(a.diagnostic+8));
        if(first_length) {
            const Word count=word(a.diagnostic+4);
            if(count)std::memmove(second_data,first_data,count);
        }
        a.unwind_state=3;
        resource_support_singleton_00b3e730(c.owners.surfaces.actual_resource_support_0108fedc,
            c.owners.surfaces.actual_lifetime_01090aa0);
        a.unwind_state=2;
        if(second_data)strings.release(static_cast<char*>(second_data),word(a.diagnostic+4)+1u);
        a.unwind_state=1;
        if(first_data)strings.release(static_cast<char*>(first_data),word(a.first_name)+1u);
        if((flags&0x10u)!=0)c.owners.actual_tracking_counter_0108daf8=c.owners.actual_tracking_counter_0108daf8+1u;
        a.unwind_state=-1;a.phase=NativeRuntimeTextureConstructionAcquired::Phase::complete;return owner;
}

void* create_native_runtime_texture_2d_00b2a070(void* renderer,
    const volatile NativeRuntimeTextureCreationArguments& arguments,
    NativeRuntimeTextureCreationContext& c,NativeRuntimeTextureCreationAcquired& a) {
    if(a.phase!=NativeRuntimeTextureCreationAcquired::Phase::fresh)
        throw std::invalid_argument("runtime texture factory frame must be fresh");
    a.phase=NativeRuntimeTextureCreationAcquired::Phase::running;
    FactoryCleanup cleanup{c,a};
        if(c.synchronization.mode_00!=0) {
            a.guard.renderer_04=renderer;
            a.guard.entered_00=enter_native_renderer_optional_guard_00b33ad0(renderer,c.synchronization);
        }
        const Word flags=arguments.flags;a.unwind_state=0;
        const Word pool=flags&0xfu;if(pool<=3)put(&a.native_pool_slot,pool);
        Word usage=(flags&0x10u)!=0?1u:0u;
        switch(flags&0xf00u) {
        case 0x100:usage|=2;break;case 0x200:usage|=0x4000;break;
        case 0x300:usage|=0x40;break;case 0x400:usage|=0x100;break;
        case 0x500:usage|=0x80;break;default:break;
        }
        if((flags&0xf000u)==0x1000u)usage|=0x200;
        if((flags&0xff000000u)==0x01000000u)usage|=0x400;
        void* device=pointer(word(at(renderer,0x1a10)));
        const Word levels=arguments.levels,height=arguments.height,width=arguments.width;
        a.com_output=nullptr;
        const auto create=method<Create>(device,0x5c);
        a.create_result=create(device,width,height,levels,usage,static_cast<D3DFORMAT>(arguments.format),
            static_cast<D3DPOOL>(word(&a.native_pool_slot)),const_cast<IDirect3DTexture9**>(&a.com_output),nullptr);
        if(!a.com_output && a.create_result!=0 && static_cast<Word>(a.create_result)!=0x8876017cu
            && static_cast<Word>(a.create_result)!=0x8007000eu) {
            if(!c.recreation)throw std::invalid_argument("runtime texture reached unbound actual B29670 recreation domain");
            recreate_native_renderer_device_00b29670(renderer,*c.recreation);
            device=pointer(word(at(renderer,0x1a10)));
            a.create_result=method<Create>(device,0x5c)(device,width,height,levels,usage,
                static_cast<D3DFORMAT>(arguments.format),static_cast<D3DPOOL>(word(&a.native_pool_slot)),
                const_cast<IDirect3DTexture9**>(&a.com_output),nullptr);
        }
        void* const captured=a.com_output;if(captured)pair(captured);
        a.raw_slot=allocate_d3d9_texture2d_slot_00b3f2b0();a.unwind_state=1;
        a.phase=NativeRuntimeTextureCreationAcquired::Phase::constructing;
        a.owner=a.raw_slot?construct_native_runtime_texture_2d_00b3f7b0(a.raw_slot,a.com_output,
            width,height,arguments.flags,c.construction,a.construction):nullptr;
        a.unwind_state=0;
        if((flags&0x10u)!=0)account(width,height,arguments.format,c);
        append(renderer,a.owner);
        void* const before_pair=a.com_output;if(before_pair)pair(before_pair);
        void* const current=a.com_output;(void)method<ComWord>(current,8)(current);
        const auto exit_mode=c.synchronization.mode_00;a.unwind_state=-1;
        if(exit_mode!=0)leave_native_renderer_optional_guard_00b33b00(a.guard.renderer_04,word(&a.guard),c.synchronization);
        a.phase=NativeRuntimeTextureCreationAcquired::Phase::complete;return a.owner;
}
} // namespace bsp
