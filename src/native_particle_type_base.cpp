#include "bsp/native_particle_type_base.hpp"
#include "bsp/native_string.hpp"
#include <cstring>
#include <initializer_list>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle type base reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
const void* at(const void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
template<class T> T read(const void* p, std::uint32_t n=0) noexcept {
    T v; std::memcpy(&v,at(p,n),sizeof v); return v;
}
template<class T> void put(void* p, std::uint32_t n, T v) noexcept {
    std::memcpy(at(p,n),&v,sizeof v);
}
void profile(void* p, const volatile std::uint32_t* table) noexcept {
    put(p,0,reinterpret_cast<std::uintptr_t>(table));
}
void copy_record(void* destination, const void* source) noexcept {
    // Native REP MOVSD is a forward copy, including overlapping buffers.
    __asm {
        mov edi,destination
        mov esi,source
        mov ecx,7
        rep movsd
    }
}
void destroy_name_reference(void* p, NativeParticleTypeBaseBindings& a) noexcept {
    destroy_native_string_header_0041dd20(at(p,8),a.owners.strings);
    profile(p,a.owners.reference_profile_00ceb130);
}
void release_descriptor(void* descriptor, NativeParticleTypeBaseBindings& a) {
    if (read<std::int32_t>(descriptor,8)<0)
        reserve_native_particle_type_records_00b00c20(descriptor,0,a);
    while (read<std::int32_t>(descriptor,4)>0)
        put(descriptor,4,read<std::uint32_t>(descriptor,4)-1u);
    put<std::uint32_t>(descriptor,4,0);
    a.owners.free_array_00bf6989(read<void*>(descriptor));
}
void unwind_members(void* p, NativeParticleTypeBaseBindings& a) {
    try { release_descriptor(at(p,0x68),a); }
    catch (...) { destroy_name_reference(p,a); throw; }
    destroy_name_reference(p,a);
}
} // namespace

NativeD3dx9Float32To16Import::NativeD3dx9Float32To16Import(HMODULE module) {
    if (!module) throw std::invalid_argument("actual d3dx9_40 module required");
    const auto address=GetProcAddress(module,"D3DXFloat32To16Array");
    if (!address) throw std::runtime_error("actual D3DXFloat32To16Array export required");
    static_assert(sizeof address==sizeof function_);
    std::memcpy(&function_,&address,sizeof function_);
}
std::uint16_t* NativeD3dx9Float32To16Import::convert(std::uint16_t* output,
    const float* input, UINT count) const { return function_(output,input,count); }

void reserve_native_particle_type_records_00b00c20(void* p,
    std::int32_t requested, NativeParticleTypeBaseBindings& a) {
    if (requested<1) requested=1;
    if (read<std::int32_t>(p,8)>=requested) return;
    void* fresh=a.allocate_array_00bf55be(static_cast<std::uint32_t>(requested)*0x1cu);
    std::uint32_t index=0, displacement=0;
    void* destination=fresh;
    while (static_cast<std::int32_t>(index)<read<std::int32_t>(p,4)) {
        if (destination) copy_record(destination,at(read<void*>(p),displacement));
        ++index; displacement+=0x1cu; destination=at(destination,0x1c);
    }
    a.owners.free_array_00bf6989(read<void*>(p));
    put(p,0,fresh); put(p,8,requested);
}

void* construct_native_particle_type_base_00b01150(void* p, const void* name,
    std::uint32_t word14, void* parent, NativeParticleTypeBaseBindings& a) {
    profile(p,a.owners.reference_profile_00ceb130);
    put<std::uint32_t>(p,4,1); profile(p,a.base_profile_00d5ddc0);
    put<std::uint32_t>(p,8,0); put<std::uint32_t>(p,12,0);
    void* descriptor=at(p,0x68);
    put<std::uint32_t>(descriptor,0,0); put<std::uint32_t>(descriptor,4,0);
    put<std::uint32_t>(descriptor,8,0);
    try {
        void* destination=at(p,8);
        if (destination!=name) {
            resize_native_string_header_0041dd40(destination,a.owners.strings,
                read<std::uint32_t>(name),true);
            if (read<std::uint32_t>(name)!=0) {
                const auto bytes=read<std::uint32_t>(destination);
                const void* source=read<const void*>(name,4);
                void* target=read<void*>(destination,4);
                if (bytes) std::memmove(target,source,bytes);
            }
        }
        const auto one=*a.one_00d7a24c;
        const auto scalar=*a.scalar_00ce3804;
        put(p,0x24,one); put(p,0x14,word14); put<std::uint32_t>(p,0x58,0);
        const auto record_scalar=*a.record_scalar_00ce6650;
        // Four native float words, four half words, one explicit stack word.
        // memcpy retains even signaling-NaN bit patterns without an FP load.
        float inputs[4];
        const std::uint32_t words[4]={0,0,record_scalar,record_scalar};
        std::memcpy(inputs,words,sizeof inputs);
        std::uint16_t halves[4];
        put(p,0x18,parent); put<std::uint8_t>(p,0x78,1);
        put<std::uint32_t>(p,0x1c,0);
        put<std::uint8_t>(p,0x29,0); put<std::uint8_t>(p,0x28,0);
        for (const auto n : {0x2cu,0x30u,0x48u,0x34u,0x38u,0x3cu,0x40u,0x44u,0x74u})
            put<std::uint32_t>(p,n,0);
        put<std::uint8_t>(p,0x4c,0);
        for (const auto n : {0x50u,0x54u,0x5cu}) put<std::uint32_t>(p,n,0);
        put<std::uint8_t>(p,0x60,0); put<std::uint8_t>(p,0x61,0);
        put<std::uint8_t>(p,0x62,1); put<std::uint8_t>(p,0x63,0);
        put<std::uint8_t>(p,0x65,0); put(p,0x20,scalar);
        a.half_import.convert(halves,inputs,4);
        std::uint32_t record[7];
        std::memcpy(record,inputs,sizeof inputs);
        std::memcpy(record+4,halves,sizeof halves);
        record[6]=a.initial_record_stack_word18;
        const auto capacity=read<std::uint32_t>(descriptor,8);
        if (read<std::uint32_t>(descriptor,4)==capacity) {
            std::uint32_t growth=capacity+capacity;
            if (static_cast<std::int32_t>(growth)<=1) growth=1;
            reserve_native_particle_type_records_00b00c20(descriptor,
                static_cast<std::int32_t>(growth),a);
        }
        void* target=at(read<void*>(descriptor),read<std::uint32_t>(descriptor,4)*0x1cu);
        if (target) copy_record(target,record);
        put(descriptor,4,read<std::uint32_t>(descriptor,4)+1u);
    } catch (...) { unwind_members(p,a); throw; }
    return p;
}

void destroy_native_particle_type_base_00b00fb0(void* p, NativeParticleTypeBaseBindings& a) {
    profile(p,a.base_profile_00d5ddc0);
    try {
        for (const auto n : {0x1cu,0x2cu,0x30u,0x48u,0x34u,0x38u,0x3cu,0x40u,0x44u,0x5cu}) {
            void* parameter=read<void*>(p,n);
            if (parameter) {
                destroy_native_particle_parameter_00affdf0(parameter,a.owners);
                return_native_particle_parameter_00b00090(parameter,a.owners);
                if (n==0x48) put<std::uint32_t>(p,n,0);
            }
        }
    } catch (...) { unwind_members(p,a); throw; }
    try { release_descriptor(at(p,0x68),a); }
    catch (...) { destroy_name_reference(p,a); throw; }
    destroy_name_reference(p,a);
}
void* delete_native_particle_type_base_00b01130(void* p, std::uint32_t flags,
    NativeParticleTypeBaseBindings& a) {
    destroy_native_particle_type_base_00b00fb0(p,a);
    if (flags&1) a.owners.free_00bf65ac(p);
    return p;
}
std::int32_t clamp_native_particle_type_record_range_00b00920(void* p) noexcept {
    auto last=static_cast<std::int32_t>(read<std::uint32_t>(p,0x6c)-1u);
    const auto upper=read<std::int32_t>(p,0x54);
    if (upper<last) last=upper;
    put(p,0x54,last);
    const auto lower=read<std::int32_t>(p,0x50);
    put(p,0x50,last<lower ? last : lower);
    return last;
}
void native_particle_type_noop_00b00740(void*,std::uint32_t) noexcept {}
} // namespace bsp
