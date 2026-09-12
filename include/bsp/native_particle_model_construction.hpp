#pragma once
#include "bsp/native_model_owner.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_mesh_section.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/random_threads.hpp"

namespace bsp {

// Actual AFF5F0-produced28h allocation. The definition has the sole retained
// reference; model08 is borrowed. No companion count or automatic destruction.
struct NativeParticleEmitterStorage {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    NativeNodeStorage* model_08;
    void* definition_0c;
    void* container_10;
    std::uint32_t words_14[4];
    std::uint32_t untouched_24;
};

// AFD2E0 produces an18h object, distinct from AFF690's30h container. The
// second array holds108h records, not particle-state6Ch records. Names of the
// arrays remain provisional; their actual element constructors are required.
struct NativeParticleArrayStorage { void* data_00; std::int32_t count_04; };
struct NativeParticleModelArraysStorage {
    NativeNodeStorage* model_00;
    NativeParticleArrayStorage bytes_04;
    NativeParticleArrayStorage records_0c;
    std::uint32_t word_14;
};

// Exact model+184..2DB, following the SAME NativeModelOwner prefix. +184 is
// a derived float here, NOT NativeModelPool's slab ID. This object is issued
// by F8D2D0 at2E0h stride; its only pool ID lives separately at+2DC.
// Unwritten bytes and matrices retain the allocation preimage.
struct NativeParticleModelTailStorage {
    float scalar_184, scalar_188;
    void* variant_18c;
    NativeParticleModelArraysStorage* arrays_190;
    NativeRenderPointerArrayStorage emitters_194;
    float reciprocal_1a0;
    std::uint8_t active_1a4, initialized_1a5;
    std::array<std::byte, 2> untouched_1a6;
    std::uint32_t words_1a8[2];
    std::uint8_t flag_1b0, flag_1b1;
    std::array<std::byte, 2> untouched_1b2;
    NativeMeshStorage* mesh_1b4;
    std::array<std::byte, 0x10> untouched_1b8;
    NativeMeshStorage* mesh_1c8;
    std::array<std::byte, 8> untouched_1cc;
    std::uint8_t flag_1d4;
    std::array<std::byte, 3> untouched_1d5;
    float scalar_1d8, scalar_1dc, scalar_1e0, scalar_1e4, scalar_1e8;
    std::uint32_t words_1ec[4];
    std::uint32_t random_1fc;
    std::array<std::byte, 0x18> untouched_200;
    CameraMatrix custom_world_218, custom_inverse_world_258, point_matrix_298;
    std::uint32_t untouched_2d8;
};
static_assert(sizeof(NativeParticleEmitterStorage) == 0x28);
static_assert(sizeof(NativeParticleModelArraysStorage) == 0x18);
static_assert(sizeof(NativeParticleModelTailStorage) == 0x158);
static_assert(offsetof(NativeParticleModelTailStorage, variant_18c) + 0x184 == 0x18c);
static_assert(offsetof(NativeParticleModelTailStorage, emitters_194) + 0x184 == 0x194);
static_assert(offsetof(NativeParticleModelTailStorage, mesh_1b4) + 0x184 == 0x1b4);
static_assert(offsetof(NativeParticleModelTailStorage, mesh_1c8) + 0x184 == 0x1c8);
static_assert(offsetof(NativeParticleModelTailStorage, custom_world_218) + 0x184 == 0x218);
static_assert(offsetof(NativeParticleModelTailStorage, point_matrix_298) + 0x184 == 0x298);

// Required real dependencies. None has a successful default implementation.
// Raw-address parameters preserve actual identities/current singleton reads.
class NativeParticleModelConstructionCallees {
public:
    virtual ~NativeParticleModelConstructionCallees() = default;
    // Nonthrowing HOST bookkeeping only: replace the SAME base.node callbacks
    // for actual D5DA50 and establish its real derived lifetime dispatch. Do not
    // create NativeModelReference (it deletes through the wrong188h pool).
    // After native base unwind, retire only the derived companion bookkeeping.
    virtual void bind_particle_profile(NativeModelOwner&) noexcept = 0;
    virtual void retire_failed_particle_profile(NativeModelOwner&) noexcept = 0;
    // Bind the already-constructed real owners to the SAME canonical lookup,
    // using NativeMesh/Material/MeshSectionReference, without an extra retain.
    virtual void bind_mesh(NativeMeshStorage&) noexcept = 0;
    virtual void bind_material(NativeMaterialStorage&) noexcept = 0;
    virtual void bind_section(NativeMeshSectionStorage&) noexcept = 0;
    // AF40E0 visits variant+10 pointer rows/count30 and invokes AF9F50 on
    // each. That routine recurses and invokes current member virtual14/0C.
    virtual void call_00af40e0(void* actual_variant) = 0;
    // Read actual resource owner +04/+1C/+18 respectively; no retain.
    virtual void* call_00af10a0(void* actual_resources) = 0;
    virtual void* call_00af10b0(void* actual_resources) = 0;
    virtual NativeMaterialStorage* call_00af1120(void* actual_resources) = 0;
    // Both exact low bytes select source at+08/+0C/+10/+14. RET8.
    virtual NativeMaterialStorage* call_00af10f0(void* actual_resources,
        std::uint8_t first, std::uint8_t second) = 0;
    // B0D140: owner60 -> B4D170(+0C) -> B4CB10(+08).
    // B0D130: owner3C -> B4CB10(+08). Both return borrowed textures.
    virtual void* call_00b0d140(void* actual_shadow_owner) = 0;
    virtual void* call_00b0d130(void* actual_shadow_owner) = 0;
    // CURRENT virtual5C of the CAPTURED actual renderer/table. Return a real
    // owned registered stream; original stack(first=0, bytes=1000h, descriptor).
    virtual void* renderer_virtual5c(void* actual_renderer, std::uint32_t captured_entry,
        std::uint32_t first, std::uint32_t bytes, void* actual_descriptor) = 0;
    // Actual resize bodies (including element construction/unwind), not malloc
    // substitutes. AFD130 produces count bytes with a4h cookie; AFD220 produces
    // count108h records with a4h cookie, calling AFDAC0 and AFD9F0.
    virtual void call_00afd130(NativeParticleArrayStorage&, std::int32_t count) = 0;
    virtual void call_00afd220(NativeParticleArrayStorage&, std::int32_t count) = 0;
    virtual void call_00afd0f0(NativeParticleArrayStorage&) = 0;
    virtual void call_00afd1e0(NativeParticleArrayStorage&) = 0;
    // Must use the supplied canonical primary RandomThreads state, including
    // BD2E60's x87 unsigned scaling and binary32 spill before affine range.
    virtual float call_00bd2f10(RandomThreads&, RandomStream, float minimum, float maximum) = 0;
    // Store actual node48 and recurse through actual34/3C with the SAME mask.
    virtual void call_007099c0(NativeNodeStorage&, std::uint32_t mask) = 0;
    // Append the raw model into actual manager+04/+08/+0C through AF0630.
    // No retain, substitute registry, or successful no-op registration.
    virtual void call_00af0950(void* actual_manager, NativeNodeStorage&) = 0;
};

struct NativeParticleModelConstructionAccess {
    NativeMeshEnvironment& meshes;
    NativeMeshConstants mesh_constants;
    NativeMeshSectionEnvironment& sections;
    NativeMeshSectionLayoutServices& layouts;
    NativeMaterialDestructionAccess& materials;
    NativeMaterialParameterAccess& parameters;
    RandomThreads& random;
    NativeParticleModelConstructionCallees& callees;
    void* const volatile& resources_00f8c280;
    void* const volatile& shadow_00f8d39c;
    void* const volatile& renderer_00f8d394;
    void* const volatile& manager_00f8c274;
    volatile std::uint32_t& live_count_00f8d2c8;
    const volatile std::uint32_t& one_00d7a24c;
    const volatile std::uint32_t& random_max_00ce3d64;
};

// Complete AFF5F0..AFF634: ECX raw28h, stack model/definition, RET8, same EAX.
NativeParticleEmitterStorage* construct_native_particle_emitter_00aff5f0(
    void*, NativeNodeStorage&, void* actual_definition);
// Complete AF6120..AF617E, including verified missing live continuation6171..79.
// Same existing actual pointer-array/ordinary allocation domain, minimum1.
void reserve_native_particle_emitter_pointers_00af6120(
    NativeRenderPointerArrayStorage&, std::int32_t capacity);
// Complete AFD2E0..AFD365 through REQUIRED real array/element dependencies.
// ECX raw18h, stack model/count, RET8, same EAX. Reverse member unwind.
NativeParticleModelArraysStorage* construct_native_particle_model_arrays_00afd2e0(
    void*, NativeNodeStorage&, std::int32_t count, NativeParticleModelConstructionCallees&);

// Complete AF74A0..AF7F31 ordinary body and14-state member/allocation unwind
// THROUGH REQUIRED REAL CALLEES. ECX raw2DCh, stack variant, same EAX, RET4.
// Pass a PREPARED canonical NativeModelOwner over a2E0h F8D2D0 slot; construct
// the variant's native8h string at+08 beforehand. Uses its SAME node binding,
// scene association, count and model tail. No successful application binding,
// derived destructor, native SEH/ABI replacement or gameplay proof is supplied.
// Null mesh/material/section faults and corrupt metadata are outside the domain.
// Constructor-unwritten fields retain preimages, including218/258 when1B0!=0.
// After throw the caller alone returns the raw slot through AF62F0; do not call
// the188h base scalar destructor. Native failure does not release every raw
// allocation or retained variant; see the documented cleanup states.
NativeNodeStorage* construct_native_particle_model_00af74a0(
    NativeModelOwner&, void* actual_variant, NativeParticleModelConstructionAccess&);

} // namespace bsp
