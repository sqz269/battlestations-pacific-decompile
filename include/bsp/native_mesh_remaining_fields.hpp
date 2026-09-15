#pragma once
#include "bsp/native_resource_stream_reads.hpp"
#include <cstdint>

namespace bsp {
// Actual B73260 unchecked DWORD pointer load, with wrapping address arithmetic.
// ECX mesh, stacked index, EAX stream, RET4. Unlike the older six-slot GUI
// convenience interface, this does not add a range or mesh-count check.
void* native_mesh_vertex_stream_unchecked_00b73260(const void* actual_mesh,
    std::uint32_t index) noexcept;
std::uint32_t native_vertex_declaration_element_count_00b47900(
    const void* actual_declaration) noexcept; // ECX owner, EAX owner+10, RET.
void set_native_vertex_compressed_format_00b61d90(void* actual_stream,
    void* actual_allocation) noexcept; // ECX owner, stacked pointer, RET4; store+50 only.

struct NativeMeshMetadataReadContext {
    NativeResourceStreamReadContext& reads;
    // Current original D61D6C profile through+24. Numeric code tokens, never
    // callable host pointers. The canonical getter is the existing B48CE0.
    const volatile std::uint32_t* logical_profile_00d61d6c;
};
struct NativeMeshMetadataReadAcquired {
    enum class Phase { empty, declaration, allocation, read, publication, complete };
    Phase phase{Phase::empty};
    std::uint32_t native_site{}, element_count{}, allocation_bytes{}, read_bytes{};
    void* allocation{}; // Outstanding raw allocation until the +50 store.
    void* published{}; // Borrowed identity after transfer, not another owner.
    void* source_stream{}; // Borrowed initial stream; can differ from destination.
    void* destination_stream{};
};
// Complete B93800[116]. Incoming ECX unused; stacked mesh/handle; RET8.
// Allocation count saturates to FFFFFFFF on unsigned count*20h overflow;
// raw-read count wraps. After the read, select the CURRENT last stream again
// and store+50 without freeing the old value. No decompression or inline count.
// No native EH cleanup: failure retains the raw allocation and all prior effects.
// Starts empty; never replay an entered operation. Allocation uses the existing
// host CRT/new-handler boundary, not a new successful fallback or allocator.
void read_native_mesh_compressed_format_00b93800(void* actual_mesh,
    void* actual_handle, NativeMeshMetadataReadContext&, NativeMeshMetadataReadAcquired&);

struct NativeMeshLightingConstants {
    const volatile std::uint32_t& first_four_00d7a24c;
    const volatile std::uint32_t& last_00ce38b8;
};
// Complete B17840[232]: ECX record, EAX same, RET. Raw bit stores, not float
// conversion. Capture first constant before stores and final constant before
// stores+38/+3C. Stack-local spill aliases remain a native ABI boundary.
void* initialize_native_mesh_lighting_record_00b17840(void* actual_record,
    NativeMeshLightingConstants) noexcept;
// Complete B179D0[28]: ECX material, stacked unused slot/record, RET8.
// Write material byte+10C=1 then17 forward DWORD loads/stores to material+38.
// Preserve source/destination overlap behavior; not memcpy/memmove.
void set_native_material_lighting_record_00b179d0(void* actual_material,
    std::uint32_t ignored_slot, const void* actual_record) noexcept;
// Complete B93390[176]: ECX handle, EDX output68bytes, RET. Seventeen
// BE99D0 calls with direct x87 FSTP32 to successive words; partial stores remain.
void read_native_mesh_lighting_record_00b93390(void* actual_handle,
    void* actual_output, NativeResourceStreamReadContext&);
// Complete B937A0[89]: unused ECX; stacked material/handle; RET8. Unsigned
// count loop; each ignored slot plus17float record overwrites the SAME material.
// Zero count changes nothing; a later failed record leaves earlier publication.
void read_native_material_lighting_field_00b937a0(void* actual_material,
    void* actual_handle, NativeResourceStreamReadContext&, NativeMeshLightingConstants);

// These cover metadata and lighting dependencies only. B93D30 texture and
// B941D0 subset/aggregate material admission remain separate unfinished work.
// New MSVC Win32 source interfaces, not binary ABI/SEH or gameplay replacements.
} // namespace bsp
