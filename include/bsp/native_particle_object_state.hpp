#pragma once
#include "bsp/native_particle_type_state.hpp"
#include "bsp/native_particle_unit_random.hpp"
#include "bsp/native_particle_emission_state.hpp"

namespace bsp {
class NativeStringStorage;
struct NativePointLightEnvironment;
// Complete BD2E60/BD2F10. Original ECX state/stream, stack(min,max), RET8,
// ST0 float32 result. EDX borrows the existing canonical random/scalar domain.
float __fastcall native_particle_random_state_range_00bd2e60(RandomState*,
    const NativeParticleUnitRandomAccess*, float minimum, float maximum);
float __fastcall native_particle_random_range_00bd2f10(RandomStream,
    const NativeParticleUnitRandomAccess*, float minimum, float maximum);

struct NativeParticleObjectStateAccess {
    const NativeParticleTypeStateAccess* type;
    const NativeParticleUnitRandomAccess* unit_random;
    const volatile float* full_angle_00ce3d9c;
    const volatile float* half_00ce3800;
    const volatile float* negative_one_00d7a260;
    const volatile float* negative_zero_00d7a208;
    void* volatile& actual_manager_01090aa0;
    NativeParticlePopulationLockStorage* volatile& actual_lock_0108ff50;
    NativeStringStorage& strings;
    NativePointLightEnvironment& lights;
    void* context;
    void* (__cdecl* operator_new_00bf681b)(std::uint32_t);
    void (__cdecl* free_00bf65ac)(void*);
    // ST0 input, pop ST0, EAX integer; required actual CRT dispatch including
    // CURRENT0109EEA4 and BF7456 low-word conversion. Assembly-call contract.
    std::int32_t (__cdecl* truncate_st0_00bf7420)();
    // Captured native targets: ECX actual identity, stack(zero,one), RET8;
    // instance virtual00 has no stack arguments. Resource factory can override
    // the domain, so no profile or successful dispatch fallback is invented.
    void* (*resource_virtual08)(void*, void* actual_resource, std::uint32_t target,
        std::uint32_t zero, float one);
    void (*instance_virtual00)(void*, void* actual_instance, std::uint32_t target);
    // Complete existing B6D890 through the canonical node/root domain.
    void (*propagate_roots_00b6d890)(void*, void* actual_node, void* actual_roots);
    // ECX definition; stack(state,dt,word,matrix,time), RET14. Captured actual
    // virtual28 must execute its real implementation, including overrides.
    void (*definition_virtual28)(void*, void* actual_definition, std::uint32_t target,
        void* actual_state, float dt, std::uint32_t word, const void* actual_matrix, float time);
};

// Complete AF8440/AF8270, actual40h matrices and actual108h record; no copies of
// resource identities. Constructor uses three primary draws and native (Z*X)*Y.
void* __fastcall construct_native_particle_object_matrix_00af8440(void* actual40h,
    const NativeParticleObjectStateAccess*);
void* __fastcall compose_native_particle_record_matrix_00af8270(const void* actual108h,
    void* unused_edx, void* actual40h_output);
// Complete AF90A0/AF8B00 normal native paths. Original ECX definition,
// initializer stack(state6Ch,record108h), cleanup stack(state6Ch,argument DWORD),
// RET8; cleanup ignores definition and argument and returns AL=true. EDX adds
// access. Resource+4 is the physical count;
// state+34/+30 are actual instance/matrix pointers, never semantic wrappers.
// Native FH3 exception ABI and incidental initializer EAX are not reproduced.
void __fastcall initialize_native_particle_object_state_00af90a0(void* actual_definition,
    const NativeParticleObjectStateAccess*, void* actual_state, const void* actual_record);
bool __fastcall clear_native_particle_object_state_00af8b00(void* actual_definition,
    const NativeParticleObjectStateAccess*, void* actual_state, std::uint32_t argument);
} // namespace bsp
