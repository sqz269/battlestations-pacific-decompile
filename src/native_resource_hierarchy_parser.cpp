#include "bsp/native_resource_hierarchy_parser.hpp"
#include "bsp/native_resource_hierarchy_pool.hpp"
#include "bsp/native_resource_hierarchy_fields.hpp"
#include "bsp/native_resource_pointer_array.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <exception>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource hierarchy parsing requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U v) noexcept { return reinterpret_cast<void*>(v); }
void* at(const void* p,U n=0) noexcept { return ptr(bits(p)+n); }
U word(const void* p,U n=0) noexcept { return *static_cast<const volatile U*>(at(p,n)); }
void put(void* p,U n,U v) noexcept { *static_cast<volatile U*>(at(p,n))=v; }
std::int32_t signed_bits(U v) noexcept { std::int32_t r;std::memcpy(&r,&v,4);return r; }
bool data_tag(void* handle,const char* tag) {
    auto* const data=static_cast<const char*>(ptr(word(ptr(word(handle)),0x14)));
    return data && _stricmp(data,tag)==0;
}
bool header_tag(void* handle,const char* tag) {
    return equal_native_string_header_00425850(at(ptr(word(handle)),0x10),tag);
}
void copy_name(void* output,const void* source,NativeStringRawPoolContext& strings) {
    if(output==source) return;
    const auto count=word(source);resize_native_string_header_0041dd40(output,strings,count,true);
    if(word(source)!=0) {
        const auto bytes=word(output);auto* const data=ptr(word(source,4));auto* const destination=ptr(word(output,4));
        if(bytes!=0) std::memmove(destination,data,bytes);
    }
}
__declspec(naked) void __fastcall copy_bounds(void*,const void*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        fld dword ptr [edx+4]
        fstp dword ptr [ecx+4]
        fld dword ptr [edx+8]
        fstp dword ptr [ecx+8]
        fld dword ptr [edx+12]
        fstp dword ptr [ecx+12]
        fld dword ptr [edx+16]
        fstp dword ptr [ecx+16]
        fld dword ptr [edx+20]
        fstp dword ptr [ecx+20]
        ret
    }
}
}
__declspec(naked) void* __fastcall build_native_sphere_bounds_00b7d160(void*,const void*) {
    __asm {
        sub esp, 2ch
        fld dword ptr [edx]
        movss xmm0, dword ptr [edx+0ch]
        fstp dword ptr [esp]
        movss dword ptr [esp+8], xmm0
        fld dword ptr [esp]
        movss dword ptr [esp+0ch], xmm0
        fld st(0)
        movss dword ptr [esp+10h], xmm0
        fadd dword ptr [esp+8]
        movss dword ptr [esp+4], xmm0
        mov eax, ecx
        fstp dword ptr [esp+20h]
        fld dword ptr [edx+4]
        movss xmm0, dword ptr [esp+20h]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fld st(0)
        fadd dword ptr [esp+0ch]
        fstp dword ptr [esp+24h]
        fld dword ptr [edx+8]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        fld st(0)
        fadd dword ptr [esp+10h]
        fstp dword ptr [esp+28h]
        fld dword ptr [esp+4]
        fst dword ptr [esp+8]
        fst dword ptr [esp+0ch]
        fstp dword ptr [esp+10h]
        fld dword ptr [esp+8]
        fsubp st(3), st(0)
        fxch st(2)
        fstp dword ptr [esp+14h]
        fsub dword ptr [esp+0ch]
        fstp dword ptr [esp+18h]
        fsub dword ptr [esp+10h]
        fstp dword ptr [esp+1ch]
        fld dword ptr [esp+14h]
        fstp dword ptr [eax]
        fld dword ptr [esp+18h]
        fstp dword ptr [eax+4]
        fld dword ptr [esp+1ch]
        fstp dword ptr [eax+8]
        movss dword ptr [eax+0ch], xmm0
        movss xmm0, dword ptr [esp+24h]
        movss dword ptr [eax+10h], xmm0
        movss xmm0, dword ptr [esp+28h]
        movss dword ptr [eax+14h], xmm0
        add esp, 2ch
        ret
    }
}
__declspec(naked) void* __fastcall set_native_sphere_bounds_00b7d220(void*,void*,const void*) {
    __asm {
        mov edx, dword ptr [esp+4]
        sub esp, 18h
        push esi
        mov esi, ecx
        lea ecx, [esp+4]
        call build_native_sphere_bounds_00b7d160
        fld dword ptr [eax]
        fstp dword ptr [esi]
        fld dword ptr [eax+4]
        fstp dword ptr [esi+4]
        fld dword ptr [eax+8]
        fstp dword ptr [esi+8]
        fld dword ptr [eax+0ch]
        fstp dword ptr [esi+0ch]
        fld dword ptr [eax+10h]
        fstp dword ptr [esi+10h]
        fld dword ptr [eax+14h]
        mov eax, esi
        fstp dword ptr [esi+14h]
        pop esi
        add esp, 18h
        ret 4
    }
}
void append_native_resource_hierarchy_record_00b87ae0(void* resource,void* record) {
    auto capacity=word(resource,0x24);const auto count=word(resource,0x20);auto* const header=at(resource,0x1c);
    if(count==capacity) {
        capacity+=16u;if(signed_bits(capacity)<=16) capacity=16;
        reserve_native_hierarchy_item_pointers_00b87350(header,signed_bits(capacity));
    }
    const auto current_count=word(header,4);const auto data=word(header);auto* const destination=ptr(data+current_count*4u);
    if(destination) put(destination,0,bits(record));put(header,4,word(header,4)+1u);
}
void parse_native_resource_hierarchy_item_00b7eb90(void* manager,void* handle,NativeResourceHierarchyParserContext& context) {
    auto* const record=allocate_static_native_hierarchy_slot_00b87a90();
    if(record) {
        const auto negative=context.negative_bound_00ce4adc;
        put(record,0,0xffffffff);put(record,4,0);put(record,8,0);
        put(record,0x4c,0);put(record,0x50,0);put(record,0x54,0);put(record,0x58,0);
        put(record,0x5c,0);put(record,0x60,0);put(record,0x64,0);
        const auto positive=context.positive_bound_00ce4970;put(record,0x68,positive);
        put(record,0x6c,negative);put(record,0x70,negative);put(record,0x74,negative);
        put(record,0x78,positive);put(record,0x7c,positive);put(record,0x80,positive);
    }
    U child,temporary[2];int state=-1;auto& reads=context.reads;
    try {
        while(native_resource_node_has_remaining_00715bf0(handle)) {
            create_native_resource_child_00bea680(handle,&child,reads);state=0;
            if(data_tag(&child,"Parent")) put(record,0,read_native_resource_node_dword_00be9a00(&child,reads));
            else if(data_tag(&child,"Resource")) {
                const auto value=read_native_resource_node_dword_00be9a00(&child,reads);auto* const header=at(record,0x4c);
                auto capacity=word(header,8);const auto count=word(header,4);
                if(count==capacity) { capacity+=4u;if(signed_bits(capacity)<=8) capacity=8;reserve_native_hierarchy_reference_array_00b7d640(header,signed_bits(capacity)); }
                const auto current_count=word(header,4);const auto data=word(header);auto* const destination=ptr(data+current_count*4u);
                if(destination) put(destination,0,value);put(header,4,word(header,4)+1u);
            } else if(data_tag(&child,"Matrix")) read_native_resource_matrix_00b936e0(&child,at(record,0xc),reads);
            else if(data_tag(&child,"Name")) {
                auto* const result=read_native_resource_handle_string_00bea010(&child,temporary,reads);state=1;
                copy_name(at(record,4),result,reads.strings);
                auto* const data=ptr(word(temporary,4));state=0;
                if(data) {
                    const auto bytes=word(temporary)+1u;
                    auto* const pool=native_string_pool_get_or_create_00419cc0(reads.strings.actual_published_01090aa8,reads.strings.actual_manager_publication_01090aa0);
                    return_native_string_pool_00bd1510(pool,data,bytes,reads.strings.actual_small_returns_disabled_01090aa4);
                }
            } else if(header_tag(&child,"Flags")) put(record,0x58,read_native_resource_node_dword_00be9a00(&child,reads));
            else if(header_tag(&child,"BoundingSphere")) {
                auto* const sphere=at(record,0x5c);read_native_resource_sphere_00b932e0(&child,sphere,reads);
                U bounds[6];auto* const result=set_native_sphere_bounds_00b7d220(bounds,nullptr,sphere);copy_bounds(at(record,0x6c),result);
            } else if(header_tag(&child,"BoundingBox")) read_native_resource_bounds_00b93310(&child,at(record,0x6c),reads);
            else skip_native_resource_node_00be9c40(&child,reads);
            state=-1;release_native_structured_node_handle_00be9ed0(&child,reads.streams);
        }
        auto* const resource=ptr(word(manager,0x24));append_native_resource_hierarchy_record_00b87ae0(resource,record);
    } catch(...) {
        try {
            if(state==1) destroy_native_string_header_0041dd20(temporary,reads.strings);
            if(state>=0) release_native_structured_node_handle_00be9ed0(&child,reads.streams);
        } catch(...) { std::terminate(); }
        throw;
    }
}
void parse_native_resource_hierarchy_00b7f100(void* manager,void* handle,NativeResourceHierarchyParserContext& context) {
    U child;bool active=false;auto& reads=context.reads;
    try {
        while(native_resource_node_has_remaining_00715bf0(handle)) {
            create_native_resource_child_00bea680(handle,&child,reads);active=true;
            if(data_tag(&child,"Item")) parse_native_resource_hierarchy_item_00b7eb90(manager,&child,context);
            else skip_native_resource_node_00be9c40(&child,reads);
            active=false;release_native_structured_node_handle_00be9ed0(&child,reads.streams);
        }
    } catch(...) {
        if(active) { try { release_native_structured_node_handle_00be9ed0(&child,reads.streams); } catch(...) { std::terminate(); } }
        throw;
    }
}
}
