#pragma once
#include "bsp/native_render_context.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_mesh_pool.hpp"
#include "bsp/native_string.hpp"
#include <array>

namespace bsp {

struct NativeMeshLodPhaseStorage {
    std::uint32_t minimum_00;
    std::uint32_t maximum_04;
    std::uint32_t untouched_08;
    std::uint32_t untouched_0c;
};
struct NativeMeshWeightNameStorage {
    std::uint32_t length_00;
    char* data_04;
};
struct NativeMeshWeightNamesStorage {
    NativeMeshWeightNameStorage* data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
// Actual BCh payload within the C0h slot. No implicit initialization or cleanup.
// +BC is the separate pool-chunk word and is never part of this object.
struct NativeMeshStorage {
    volatile std::uint32_t native_vtable_00;
    std::atomic<std::int32_t> references_04;
    std::uint32_t word_08;
    std::uint32_t lod_bits_0c;
    NativeMeshLodPhaseStorage lod_phases_10[4];
    std::int32_t lod_count_50;
    NativeRenderPointerArrayStorage draw_sections_54;
    void* index_stream_60;
    void* vertex_streams_64[6];
    std::int32_t vertex_stream_count_7c;
    std::uint8_t flag_80;
    std::array<std::byte, 3> untouched_81;
    std::uint32_t zero_words_84[4];
    std::uint8_t flag_94;
    std::array<std::byte, 0x1b> untouched_95;
    NativeMeshWeightNamesStorage weight_names_b0;
};

struct NativeMeshConstants {
    const volatile std::uint32_t& minimum_00ce4adc;
    const volatile std::uint32_t& maximum_00ce4970;
    const volatile std::uint32_t& lod_00d7a24c;
};
struct NativeMeshEnvironment {
    // SAME concrete canonical pool0108FFF8 which issued this C0h slot.
    NativeMeshPool& pool_0108fff8;
    NativeRenderActualOwners& retained_owners;
    NativeStringStorage& strings;
    // Actual CURRENT D62D60 profile, at least two DWORDs. Never synthesize it.
    const volatile std::uint32_t* vtable_00d62d60;
};

// Original ECX=actual BCh payload, EAX=same, RET. Placement-only constructor:
// preserves untouched bytes/unused stream slots and the pool word; reads the
// two phase constants first and LOD constant at its later native store point.
NativeMeshStorage* construct_native_mesh_00b73d70(void*, NativeMeshConstants) noexcept;

// B72E10/B73840: ECX actual three-word header, signed capacity/count, RET4.
// Reserve grows only with minimum4; resize zeroes newly exposed pointers and
// shrinks count without releasing owners. Ordinary array allocation/free uses
// the existing BF55BE/BF6989 boundary, separate from the mesh object pool.
void reserve_native_mesh_section_pointers_00b72e10(
    NativeRenderPointerArrayStorage&, std::int32_t capacity);
void resize_native_mesh_section_pointers_00b73840(
    NativeRenderPointerArrayStorage&, std::int32_t count);
void destroy_native_mesh_section_pointers_00b73cb0(NativeRenderPointerArrayStorage&);

// B73B70/B73BB0, ECX mesh, index owner / (signed stream index, owner), RET4/8.
// Equal pointer is a no-op. Publish incoming, retain its actual+04, then
// release captured old through its current canonical zero-reference dispatch.
// Stream growth clears every newly exposed slot before retained assignment.
void set_native_mesh_index_stream_00b73b70(
    NativeMeshStorage&, NativeRenderActualOwners&, void* actual_index);
void set_native_mesh_vertex_stream_00b73bb0(
    NativeMeshStorage&, NativeRenderActualOwners&, std::int32_t index, void* actual_stream);
// ECX mesh, nonnull actual section on stack, RET4 via InterlockedIncrement.
// Grow capacity max(2*old,4), store section, increment count, retain actual+04.
// Section storage/material/index-buffer construction remains a separate owner.
void append_native_mesh_draw_section_00b73c60(NativeMeshStorage&, void* actual_section);
// ECX mesh, RET. Reload data/count after each release; entries assumed nonnull.
// On success resize0 without freeing backing or clearing dangling cells.
void clear_native_mesh_draw_sections_00b73c10(NativeMeshStorage&, NativeRenderActualOwners&);

// 427110's no-grow count0 fragment, and full427880 in the valid header domain.
// Reverse order; decrement actual count BEFORE returning each string's captured
// data/length+1. Preserve element headers. Full destructor then frees backing.
void clear_native_mesh_weight_names_00427110_fragment(
    NativeMeshWeightNamesStorage&, NativeStringStorage&) noexcept;
void destroy_native_mesh_weight_names_00427880(
    NativeMeshWeightNamesStorage&, NativeStringStorage&) noexcept;

// Full ECX mesh, RET: release/clear index; release streams using captured end;
// release sections using live count; strings, section backing, base table.
// State2 unwind destroys strings, state1 section backing, state0 base, WITHOUT
// retrying owner releases. No reset of stream count/slots or freed pointers.
void destroy_native_mesh_00b73e60(NativeMeshStorage&, NativeMeshEnvironment&);
// ECX mesh, flags on stack, EAX original address even after return, RET4.
// Return physical slot through canonical pool iff flags&1 after successful body.
NativeMeshStorage* delete_native_mesh_00b74280(
    NativeMeshStorage*, NativeMeshEnvironment&, std::uint32_t flags);

class NativeMeshReference;
struct NativeMeshCompanionDisposal {
    void* context;
    void (*retire)(void*, NativeMeshReference&) noexcept;
};
// One canonical companion, borrowing the SAME actual+04, with no extra retain.
// Register it in NativeRenderActualOwners before publishing this raw mesh in
// model+180. Its terminal path requires current D62D60 slots BD30E0/B74280.
// Native storage, environment and companion survive until terminal retirement;
// direct destruction while bound is invalid. Missing profiles never fall back.
class NativeMeshReference final : public RenderCommandReference {
public:
    NativeMeshReference(NativeMeshStorage&, NativeMeshEnvironment&, NativeMeshCompanionDisposal);
    ~NativeMeshReference() override;
    NativeMeshReference(const NativeMeshReference&) = delete;
    NativeMeshReference& operator=(const NativeMeshReference&) = delete;
    NativeMeshStorage& storage() noexcept { return storage_; }
    void release_zero_references() noexcept override;
private:
    enum class Phase { bound, destroying, retired };
    NativeMeshStorage& storage_;
    NativeMeshEnvironment& environment_;
    NativeMeshCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const noexcept;
};

// Valid native domain: aligned live owners/atomics; 0<=stream count<=6 and
// 0<=index<6; nonnegative valid vector headers, count<=capacity; allocation
// sizes and signed capacity doubling fit. No corrupt-header recovery, renderer
// services, fake section objects, complete string-vector growth, or game ABI.
} // namespace bsp
