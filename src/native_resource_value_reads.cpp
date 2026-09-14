#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_adopted_substream.hpp"
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource value reads require MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
using U=std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U v) noexcept { return reinterpret_cast<void*>(v); }
U word(const void* p,U n=0) noexcept { return *static_cast<const volatile U*>(ptr(bits(p)+n)); }
U __cdecl read_float_bits(void* stream,U* actual,NativeResourceStreamReadContext& context) {
    U value=bits(actual);const auto entry=word(ptr(word(stream)),0x24);
    context.streams.source_read(entry,stream,&value,4,actual);return value;
}
void* __cdecl select_float_stream(void* reader) {
    auto* const stream=ptr(word(reader));const auto entry=word(ptr(word(stream)),0x44);
    if(entry!=0x00be4360) throw std::runtime_error("Reached native resource float target is not reconstructed");
    return stream;
}
// Integer-only setup followed by the original scalar/FSTP sequence. No C++
// float temporary inserts an unobserved store/load between the native stages.
__declspec(naked) void __cdecl read_float_sequence(void*,void*,NativeResourceStreamReadContext&,U) {
    __asm {
        push ebp
        mov ebp, esp
        push ebx
        push esi
        push edi
        mov ebx, dword ptr [ebp+8]
        mov esi, dword ptr [ebp+12]
        mov edi, dword ptr [ebp+20]
    again:
        push dword ptr [ebp+16]
        push ebx
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr [esi]
        add esi, 4
        dec edi
        jnz again
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
    }
}
}
__declspec(naked) float __cdecl read_native_stream_float_00be4360(void*,U*,NativeResourceStreamReadContext&) {
    __asm {
        mov eax, esp
        push dword ptr [eax+12]
        push dword ptr [eax+8]
        push dword ptr [eax+4]
        call read_float_bits
        add esp, 12
        push eax
        fld dword ptr [esp]
        add esp, 4
        ret
    }
}
__declspec(naked) float __cdecl read_native_resource_float_00bf02c0(void*,U*,NativeResourceStreamReadContext&) {
    __asm {
        sub esp, 8
        push dword ptr [esp+12]
        call select_float_stream
        add esp, 4
        mov ecx, eax
        mov edx, dword ptr [esp+20]
        lea eax, [esp]
        push edx
        push eax
        push ecx
        call read_native_stream_float_00be4360
        add esp, 12
        fstp dword ptr [esp+4]
        mov eax, dword ptr [esp+16]
        fld dword ptr [esp+4]
        mov ecx, dword ptr [esp]
        sub dword ptr [eax], ecx
        add esp, 8
        ret
    }
}
__declspec(naked) float __cdecl read_native_resource_node_float_00be99d0(void*,NativeResourceStreamReadContext&) {
    __asm {
        mov eax, dword ptr [esp+4]
        mov eax, dword ptr [eax]
        mov ecx, dword ptr [eax+8]
        add eax, 20h
        push dword ptr [esp+8]
        push eax
        push ecx
        call read_native_resource_float_00bf02c0
        add esp, 12
        ret
    }
}
void read_native_resource_sphere_00b932e0(void* handle,void* output,NativeResourceStreamReadContext& context) {
    read_float_sequence(handle,output,context,4);
}
void read_native_resource_bounds_00b93310(void* handle,void* output,NativeResourceStreamReadContext& context) {
    read_float_sequence(handle,output,context,6);
}
void read_native_resource_matrix_00b936e0(void* handle,void* output,NativeResourceStreamReadContext& context) {
    read_float_sequence(handle,output,context,16);
}
U read_native_resource_node_dword_00be9a00(void* handle,NativeResourceStreamReadContext& context) {
    auto* const node=ptr(word(handle));auto* const budget=static_cast<U*>(ptr(bits(node)+0x20));
    auto* const reader=ptr(word(node,8));return read_native_resource_dword_00bf0280(reader,budget,context);
}
void* read_native_resource_node_string_00be9fe0(void* node,void* output,NativeResourceStreamReadContext& context) {
    auto* const budget=static_cast<U*>(ptr(bits(node)+0x20));auto* const reader=ptr(word(node,8));
    read_native_resource_string_00bf0510(reader,output,budget,context);return output;
}
void* read_native_resource_handle_string_00bea010(void* handle,void* output,NativeResourceStreamReadContext& context) {
    auto* const node=ptr(word(handle));read_native_resource_node_string_00be9fe0(node,output,context);return output;
}
}
