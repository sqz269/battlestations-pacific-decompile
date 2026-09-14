#include "bsp/native_mesh_scalar_fields.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_vector.hpp"
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mesh scalar fields require MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, U offset = 0) noexcept { return ptr(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p, offset)) = value; }

// Preserve the original BE99D0 -> FSTP32 without a C++ float return/store
// between the source reader's x87 return and the actual local record word.
__declspec(naked) void __cdecl read_float_word(void*, void*, NativeResourceStreamReadContext&) {
    __asm {
        push ebp
        mov ebp, esp
        push dword ptr [ebp+16]
        push dword ptr [ebp+12]
        call read_native_resource_node_float_00be99d0
        add esp, 8
        mov eax, dword ptr [ebp+8]
        fstp dword ptr [eax]
        pop ebp
        ret
    }
}
// Bounds are deliberately discarded from ST0, not rounded through a float
// temporary. Keep a frame so a throwing source reader can unwind its call.
__declspec(naked) void __cdecl discard_float(void*, NativeResourceStreamReadContext&) {
    __asm {
        push ebp
        mov ebp, esp
        push dword ptr [ebp+12]
        push dword ptr [ebp+8]
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp st(0)
        pop ebp
        ret
    }
}
void return_name(void* data, U size, NativeStringRawPoolContext& strings) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, size, strings.actual_small_returns_disabled_01090aa4);
}
} // namespace

U read_native_resource_node_control_dword_00be99f0(void* handle, NativeResourceStreamReadContext& reads) {
    auto* const node = ptr(word(handle));
    auto* const budget = static_cast<U*>(at(node, 0x20));
    auto* const reader = ptr(word(node, 8));
    return read_native_resource_control_dword_00bf02a0(reader, budget, reads);
}
void set_native_mesh_lod_00b72710(void* mesh, U value) noexcept { put(mesh, 0x0c, value); }
void append_native_mesh_lod_phase_00b73270(void* mesh, const void* source) noexcept {
    const auto count = word(mesh, 0x50);
    auto* const destination = at(mesh, 0x10 + (count << 4));
    put(destination, 0, word(source));
    put(destination, 4, word(source, 4));
    put(destination, 8, word(source, 8));
    put(destination, 12, word(source, 12));
    put(mesh, 0x50, word(mesh, 0x50) + 1u);
}
void append_native_mesh_weight_name_00b73d50(void* mesh, const void* source, NativeStringRawPoolContext& strings) {
    ActualNativeStringPoolStorage storage{strings.actual_published_01090aa8,
        strings.actual_small_returns_disabled_01090aa4, strings.actual_manager_publication_01090aa0};
    append_native_string_vector_004cdc20(*static_cast<NativeStringVectorStorage*>(at(mesh, 0xb0)),
        *static_cast<const NativeString*>(source), storage);
}
void consume_native_mesh_sphere_00b93590(void*, void* handle, NativeResourceStreamReadContext& reads) {
    discard_float(handle, reads); discard_float(handle, reads);
    discard_float(handle, reads); discard_float(handle, reads);
}
void consume_native_mesh_bounds_00b935c0(void*, void* handle, NativeResourceStreamReadContext& reads) {
    discard_float(handle, reads); discard_float(handle, reads);
    discard_float(handle, reads); discard_float(handle, reads);
    discard_float(handle, reads); discard_float(handle, reads);
}
void read_native_mesh_lod_phases_00b93710(void* mesh, void* handle, NativeResourceStreamReadContext& reads,
    const volatile U& minimum, const volatile U& maximum) {
    auto remaining = read_native_resource_node_control_dword_00be99f0(handle, reads);
    while (remaining != 0) {
        U phase[4];
        put(phase, 0, minimum); put(phase, 4, maximum);
        read_float_word(phase, handle, reads); read_float_word(phase + 1, handle, reads);
        phase[2] = read_native_resource_node_dword_00be9a00(handle, reads);
        phase[3] = read_native_resource_node_dword_00be9a00(handle, reads);
        append_native_mesh_lod_phase_00b73270(mesh, phase);
        --remaining;
    }
}
void read_native_mesh_weight_names_00b93f90(void* mesh, void* handle, NativeResourceStreamReadContext& reads) {
    U temporary[2]; bool complete = false;
    try {
        while (word(ptr(word(handle)), 0x20) != 0) {
            auto* const returned = read_native_resource_handle_string_00bea010(handle, temporary, reads);
            complete = true;
            append_native_mesh_weight_name_00b73d50(mesh, returned, reads.strings);
            auto* const data = ptr(word(temporary, 4)); complete = false;
            if (data) { const auto size = word(temporary) + 1u; return_name(data, size, reads.strings); }
        }
    } catch (...) {
        if (complete) {
            try { destroy_native_string_header_0041dd20(temporary, reads.strings); }
            catch (...) { std::terminate(); }
        }
        throw;
    }
}
} // namespace bsp
