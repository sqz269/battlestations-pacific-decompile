#pragma once
#include "bsp/native_particle_type_state.hpp"
#include <cstddef>

namespace bsp {
class NativeStringStorage;
struct NativePointLightEnvironment;
struct NativeParticleUnitRandomAccess;
class NativeRenderActualOwners;
class GeneratedModelLifetimeRuntime;
class NativeModelOwner;
struct NativeParticlePopulationLockStorage;
struct NativeHardwareLayoutConstructContext;
struct NativeParticleTracerStateAccess;
struct NativeTracelineGeometryAccess;
struct NativeTracelineUpdateAccess;
struct NativeTracelineLifetimeAccess;
class NativeTracelineReference;
struct RenderNodeRootList;

// Optional concrete dependency composition. Native operations use their actual
// reconstructed bodies. The two callbacks only resolve/register existing host
// companions: they must not allocate or construct another native node/root.
// Bindings and their application companions survive native terminal retirement.
// State BF681B scalar allocations must match lifetime BF65AC; state and geometry
// BF55BE arrays must match lifetime BF6989. Composition requires the identical
// BF6989 callback and identical BF55BE allocation callback across these bindings.
// Wrappers over the same heap must be bound consistently, not independently.
struct NativeParticleTracerReconstruction {
    NativeTracelineGeometryAccess* geometry;
    const NativeTracelineUpdateAccess* update;
    NativeTracelineLifetimeAccess* lifetime;
    void* context;
    // AFTER caller publishes D0C928, before attachment: bind one canonical
    // reference over its already constructed NativeModelOwner, in SAME runtime
    // and retained-owner registry. Preserve all native bytes and actual counts.
    NativeTracelineReference& (*bind_constructed)(void*,void* actual_node);
    // Pure lookup of the existing view of actual root+0C/+1C; never raw-cast.
    RenderNodeRootList& (*resolve_root)(void*,void* actual_root);
};

// Real application boundaries, all over actual storage. No second particle,
// scene, resource, reference-count or allocation domain is created here.
class NativeParticleTracerApplication {
public:
    virtual ~NativeParticleTracerApplication() = default;
    // Pure, nonallocating CURRENT native profile lookup. Native numeric vtable
    // identities are not host function pointers. Preserve FP and object state.
    virtual std::uint32_t table_slot(std::uint32_t native_profile,
        std::uint32_t byte_offset) noexcept = 0;
    // B317E0, renderer+38: actual counted8h format name -> actual declaration.
    // B0AB90 passes pf43uf43ccuf44uf41.mvfm. Registry/decode internals remain
    // required; returning a semantic VertexDeclaration is not this contract.
    virtual void* renderer_virtual38(void* actual_renderer,
        std::uint32_t captured_target, const void* actual_name8h) = 0;
    // Only reached for another CURRENT target; established B2F710 is executed
    // directly using hardware_layouts and its actual shared factory domains.
    virtual void* renderer_virtual40(void* actual_renderer,
        std::uint32_t captured_target, const void* actual_key14h) = 0;
    // AF32F0 owns F8C288, 32 slots/slab, actual1BCh stride and ID at+1B8.
    // Must execute its real pool operation; neither malloc nor BAD6F0's pool.
    virtual void* call_00af32f0(void* actual_pool_00f8c288) = 0;
    // Same raw slot, PREPARED model base companion, actual names and scene
    // domain. No NativeModelReference may replace the derived node lifetime.
    virtual NativeModelOwner& prepared_model(void* actual_traceline) = 0;
    // Current Traceline+5C (D0C928->AF3440): payload80h, actual root from
    // record+A4->A4; RET8. Full mesh/material/resource construction is required.
    virtual void node_virtual5c(void* actual_node, std::uint32_t captured_target,
        void* actual_payload80h, void* actual_root) = 0;
    // Current particle+28 (D5E048->B0A110), RET14h. Five stack words:
    // state, age, word, matrix, delta. The last is accepted but unread natively.
    virtual void definition_virtual28(void* actual_definition,
        std::uint32_t captured_target, void* actual_state, float age,
        std::uint32_t word, const void* actual_matrix, float delta) = 0;
    // Current payload+00 deleting slot (D0D4A4->86ADE0); RET4. This payload
    // is not refcounted. Must destroy its real arrays/retained resources/free.
    virtual void payload_virtual00(void* actual_payload,
        std::uint32_t captured_target, std::uint32_t flags) = 0;
};

struct NativeParticleTracerStateAccess {
    NativeParticleTypeStateAccess common;
    NativeStringStorage* strings;
    NativePointLightEnvironment* point_lights;
    NativeRenderActualOwners* retained_owners;
    GeneratedModelLifetimeRuntime* nodes;
    void* volatile* actual_manager_01090aa0;
    NativeParticlePopulationLockStorage* volatile* actual_lock_0108ff50;
    void* volatile* actual_resources_00f8d38c;
    void* volatile* actual_renderer_00f8d394;
    void* volatile* actual_atlas_00f8c26c;
    void* actual_pool_00f8c288;
    NativeParticleTracerApplication* application;
    NativeHardwareLayoutConstructContext* hardware_layouts;
    void* (__cdecl* allocate_00bf681b)(std::size_t);
    void* (__cdecl* allocate_00bf55be)(std::size_t);
    void (__cdecl* free_00bf6989)(void*);
    const volatile float* phase_max_00ce3d9c;
    const volatile float* width_default_00d7a2f0;
    const volatile float* segment_life_default_00d7a248;
    const volatile float* tile_default_00ce38b8;
    const volatile double* color_scale_00ce4b48;
    const char* null_atlas_pattern_00e17bf0;
    // B0AB90 writes key+00 and +10, leaving key+04/+08/+0C as incoming stack
    // residue. Preserve supplied words, even when count is one.
    std::uint32_t layout_key_residue[3];
    // The SAME common.random domain, used by complete BD2F10/BD2E60.
    const NativeParticleUnitRandomAccess* unit_random;
    const NativeParticleTracerReconstruction* reconstruction = nullptr;
};

// Complete B0B6A0..B0C80E through the required real services above. Original
// ECX Tracer definition, stack(actual6Ch state, actual108h record), RET8.
// EDX adds access. Native instruction order, x87 precision/spills/control-word
// scopes, RNG draws, raw arrays, live globals and current dispatch are retained.
// New source ABI; native FH3/SEH unwind and game validation are separate.
void __fastcall initialize_native_particle_tracer_state_00b0b6a0(void* definition,
    const NativeParticleTracerStateAccess*, void* state, const void* record);
// Complete B0A840..B0A91A. Original ECX unused; stack(state, force), RET8/AL.
// Captures actual72B740 section before node190 check; defers with195/1A8 set
// when segments remain and low force byte is zero. Deletes34 before unlink30.
std::uint8_t cleanup_native_particle_tracer_state_00b0a840(void* state,
    std::uint32_t force, const NativeParticleTracerStateAccess&);
// Complete actual singleton24-byte resource constructor/getter. Borrow the
// same raw manager publication and hardware-layout factory/tree/owner domains.
void* construct_native_tracer_resources_00b0ab90(void* actual18h,
    const NativeParticleTracerStateAccess&);
void* get_native_tracer_resources_00b0b5d0(const NativeParticleTracerStateAccess&);
// Complete thunk; ECX size is overwritten by actual static pool F8C288.
void* allocate_native_traceline_slot_00af3430(const NativeParticleTracerStateAccess&);
// Complete generated-model-derived base constructor; ECX node, stack name,
// RET4/EAX same node. Uses the same prepared base companion and real7099C0.
void* construct_native_traceline_00858260(void*, const void* actual_name8h,
    const NativeParticleTracerStateAccess&);
// Complete actual record matrix selection. ECX108h record, RET/EAX borrowed
// matrix. Uses exact native node local/world helpers, including lazy refresh.
const void* __fastcall native_particle_record_matrix_00afda80(const void* record);
} // namespace bsp
