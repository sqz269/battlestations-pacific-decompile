#pragma once
#include "bsp/native_model_owner.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_mesh_section.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/registered_type4_effect_behavior.hpp"

namespace bsp {
class NativeTracerProfileBindings;

// SAME actual30h records at tracer+194. Reserve copies the ten float words
// through x87 then the two link DWORDs; resize initializes ONLY the two links.
// Link identities are copied unchanged during reserve, never rebased.
struct NativeTracerPointStorage {
    float words_00[10];
    NativeTracerPointStorage* link_28;
    NativeTracerPointStorage* link_2c;
};
struct NativeTracerPointArrayStorage {
    void* data_00;
    std::int32_t count_04;
    std::int32_t capacity_08;
};
static_assert(sizeof(NativeTracerPointStorage) == 0x30);
static_assert(sizeof(NativeTracerPointArrayStorage) == 0x0c);
static_assert(offsetof(NativeTracerPointStorage, link_28) == 0x28);

// Complete BAC070..BAC12A and BAC310..BAC363. Original ECX actual three-word
// header, signed capacity/count stack, RET4. Same existing ordinary CRT/new
// boundary, not a separate pool. Supported native-valid headers/backing;
// byte-size arithmetic wraps32. No zero-fill, pointer relink, or rollback.
void reserve_native_tracer_points_00bac070(NativeTracerPointArrayStorage&, std::int32_t capacity);
void resize_native_tracer_points_00bac310(NativeTracerPointArrayStorage&, std::int32_t count);

struct NativeTracerConstructionConstants {
    const volatile std::uint32_t& scalar_00ce74f8;
    const volatile std::uint32_t& scalar_00ce3d34;
    const volatile std::uint32_t& scalar_00d6404c;
    const volatile std::uint32_t& scalar_00ce380c;
    const volatile std::uint32_t& scalar_00d0d918;
    const volatile std::uint32_t& scalar_00ce81a4;
    const volatile std::uint32_t& one_00d7a24c;
    const volatile std::uint32_t& scalar_00ce38b8;
    const volatile std::uint32_t& unchanged_00d7a260;
};
class NativeTracerConstructionCallees {
public:
    virtual ~NativeTracerConstructionCallees() = default;
    // Pure nonthrowing companion bookkeeping: bind these REAL constructed
    // owners in the SAME NativeRenderActualOwners, using existing concrete
    // NativeMesh/Material/MeshSectionReference, without an additional retain.
    virtual void bind_mesh(NativeMeshStorage&) noexcept = 0;
    virtual void bind_material(NativeMaterialStorage&) noexcept = 0;
    virtual void bind_section(NativeMeshSectionStorage&) noexcept = 0;
    // BADB7B: capture renderer and TABLE before descriptor virtual24, then
    // read table+5C AFTER descriptor returns. Dispatch that captured entry on
    // that captured renderer with exact (template30,1,descriptor) stack words.
    // Must return the real owned registered stream. No successful default.
    virtual void* renderer_virtual5c(void* actual_renderer, std::uint32_t captured_entry,
        std::uint32_t first, std::uint32_t second, void* actual_descriptor) = 0;
};
struct NativeTracerConstructionAccess {
    NativeMeshEnvironment& meshes;
    NativeMeshConstants mesh_constants;
    NativeMeshSectionEnvironment& sections;
    NativeMeshSectionLayoutServices& layouts;
    NativeMaterialDestructionAccess& materials;
    NativeMaterialParameterAccess& parameters;
    NativeTracerConstructionCallees& callees;
    void* const volatile& renderer_00f8d394;
    NativeTracerConstructionConstants constants;
};

// Complete BAD6F0..BADC6D construction THROUGH REQUIRED REAL renderer and
// canonical owner bindings, including native member/raw-slot/string unwind.
// Original ECX actual7ACh, stack source/unused/textures0/1/template, RET14h;
// EAX same owner. Prepare bindings.model on the SAME actual7B0h tracer slot.
// Its environment.actual_names must equal parameters.parameter_names: base,
// temporary, material-parameter and unwind names use one actual string service.
// No second node, transform, count, name, pool, ring, or base188h scalar lifetime.
// The actual template, retained in190, supplies +08 scalar,+0C mesh,+10 ring count,
// +14 effect,+30 renderer/range word,+34 range word,+38 four-float bounds.
// source+14 is its actual root. The second argument is marshalled but unread.
// Constructor-unwritten bytes, including184/188 and matrices/parameter source
// arrays, retain the preimage. Full native-invalid-pointer/SEH behavior and
// game rendering are not claimed. Native retained template/material/stream and
// raw constructed mesh/section are NOT automatically released on failure.
// The caller returns the failed raw slot through BAC2B0 after member unwind;
// on success establish NativeTracerReference before owning-reference traffic.
RegisteredType4TracerView construct_native_tracer_00bad6f0(
    NativeTracerProfileBindings&, void* actual_source, void* unused_second,
    void* actual_texture0, void* actual_texture1, void* actual_template,
    NativeTracerConstructionAccess&);
} // namespace bsp
