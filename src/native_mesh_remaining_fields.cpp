#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_mesh_buffer_fields.hpp"
#include "bsp/native_mesh_scalar_fields.hpp"
#include "bsp/native_renderer_binding_getters.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mesh metadata and lighting fields require MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U v) noexcept { return reinterpret_cast<void*>(v); }
void* at(const void* p, U offset = 0) noexcept { return ptr(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p, offset)) = value; }

// Keep BE99D0's ST0 return alive through the one original FSTP32, without an
// intervening C++ float return/store. The frame supports source C++ unwinding;
// it is not the original register/private-stack or native SEH frame.
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
} // namespace

void* native_mesh_vertex_stream_unchecked_00b73260(const void* mesh, U index) noexcept {
    return ptr(word(mesh, 0x64u + index * 4u));
}
U native_vertex_declaration_element_count_00b47900(const void* declaration) noexcept {
    return word(declaration, 0x10);
}
void set_native_vertex_compressed_format_00b61d90(void* stream, void* allocation) noexcept {
    put(stream, 0x50, bits(allocation));
}
void read_native_mesh_compressed_format_00b93800(void* mesh, void* handle,
    NativeMeshMetadataReadContext& context, NativeMeshMetadataReadAcquired& acquired) {
    using Phase = NativeMeshMetadataReadAcquired::Phase;
    if (acquired.phase != Phase::empty || acquired.allocation || acquired.published)
        throw std::logic_error("Native mesh metadata operation cannot be replayed");
    acquired.phase = Phase::declaration;
    acquired.native_site = 0x00b93820;
    void* const source = native_mesh_vertex_stream_unchecked_00b73260(mesh,
        native_mesh_vertex_stream_count_00b72b20(mesh) - 1u);
    acquired.source_stream = source;
    const auto token = word(source);
    if (token != 0x00d61d6cu || !context.logical_profile_00d61d6c ||
        context.logical_profile_00d61d6c[0x24 / 4] != 0x00b48ce0u)
        throw std::logic_error("Native compressed metadata requires current logical vertex slot24");
    void* const declaration = native_logical_vertex_stream_get_declaration_00b48ce0(source);
    const U count = native_vertex_declaration_element_count_00b47900(declaration);
    acquired.element_count = count;
    const std::uint64_t product = static_cast<std::uint64_t>(count) * 0x20u;
    const U allocation_bytes = product > 0xffffffffu ? 0xffffffffu : static_cast<U>(product);
    acquired.allocation_bytes = allocation_bytes;
    acquired.phase = Phase::allocation;
    acquired.native_site = 0x00b9383c;
    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, allocation_bytes, allocation_bytes});
    acquired.allocation = allocation;
    const U read_bytes = count << 5;
    acquired.read_bytes = read_bytes;
    acquired.phase = Phase::read;
    acquired.native_site = 0x00b9384f;
    read_native_resource_node_raw_00be9a20(handle, allocation, read_bytes, context.reads);
    acquired.phase = Phase::publication;
    acquired.native_site = 0x00b93869;
    void* const destination = native_mesh_vertex_stream_unchecked_00b73260(mesh,
        native_mesh_vertex_stream_count_00b72b20(mesh) - 1u);
    acquired.destination_stream = destination;
    set_native_vertex_compressed_format_00b61d90(destination, allocation);
    acquired.allocation = nullptr;
    acquired.published = allocation;
    acquired.phase = Phase::complete;
}
void* initialize_native_mesh_lighting_record_00b17840(void* record,
    NativeMeshLightingConstants constants) noexcept {
    const U first = constants.first_four_00d7a24c;
    for (U offset = 0; offset != 0x10; offset += 4) put(record, offset, first);
    for (U offset = 0x10; offset != 0x38; offset += 4) put(record, offset, 0);
    const U last = constants.last_00ce38b8;
    put(record, 0x38, 0);
    put(record, 0x3c, 0);
    put(record, 0x40, last);
    return record;
}
void set_native_material_lighting_record_00b179d0(void* material, U,
    const void* record) noexcept {
    *static_cast<volatile std::uint8_t*>(at(material, 0x10c)) = 1;
    for (U offset = 0; offset != 0x44; offset += 4)
        put(material, 0x38u + offset, word(record, offset));
}
void read_native_mesh_lighting_record_00b93390(void* handle, void* output,
    NativeResourceStreamReadContext& reads) {
    for (U offset = 0; offset != 0x44; offset += 4)
        read_float_word(at(output, offset), handle, reads);
}
void read_native_material_lighting_field_00b937a0(void* material, void* handle,
    NativeResourceStreamReadContext& reads, NativeMeshLightingConstants constants) {
    U remaining = read_native_resource_node_control_dword_00be99f0(handle, reads);
    while (remaining != 0) {
        const U ignored_slot = read_native_resource_node_control_dword_00be99f0(handle, reads);
        U record[17];
        initialize_native_mesh_lighting_record_00b17840(record, constants);
        read_native_mesh_lighting_record_00b93390(handle, record, reads);
        set_native_material_lighting_record_00b179d0(material, ignored_slot, record);
        --remaining;
    }
}
} // namespace bsp
