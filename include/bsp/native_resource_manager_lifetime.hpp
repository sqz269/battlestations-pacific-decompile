#pragma once
#include "bsp/native_resource_parser_singletons.hpp"

namespace bsp {
struct NativeStringRawPoolContext;
struct SingletonLifetimeCallbacks;

struct NativeResourceParserPublications {
    void* volatile& group_params_0109033c;
    void* volatile& mesh_0109047c;
    void* volatile& skined_mesh_01090480;
    void* volatile& matrix_indexed_mesh_01090484;
    void* volatile& camera_010902a0;
    void* volatile& skined_mesh_animation_0109043c;
};
class NativeResourceManagerFactoryCalls {
public:
    virtual ~NativeResourceManagerFactoryCalls() = default;
    virtual void delete_factory(std::uintptr_t captured_target, void* actual_factory,
        std::uint32_t flags) = 0;
};
// Finite binding for the default4h factory's slot0 B7D290. Other factories
// require their own recovered binding; unknown targets are rejected.
class NativeDefaultResourceManagerFactoryCalls final : public NativeResourceManagerFactoryCalls {
public:
    void delete_factory(std::uintptr_t, void*, std::uint32_t) override;
};
struct NativeResourceManagerContext {
    void* volatile& actual_lifetime_manager_01090aa0;
    void* volatile& actual_resource_manager_010901c4;
    NativeResourceParserPublications parsers;
    NativeStringRawPoolContext& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativeResourceParserNameCalls& names;
    NativeResourceManagerFactoryCalls& factories;
};

// Complete B7D290[31]: ECX factory, stack flags, EAX original address, RET4.
// Capture bit0 before stamping CFD7DC, optionally free captured4h allocation.
void* delete_native_default_resource_factory_00b7d290(void*, std::uint32_t) noexcept;
// Complete B7D2C0[17]: ECX manager, RET. Clear actual010901C4 then stampCE3818.
void destroy_native_resource_manager_base_00b7d2c0(void*, NativeResourceManagerContext&) noexcept;

// Complete B81040[260]: ECX raw28h manager, EAX same manager, RET.
// StampD63128, allocate/stamp default4h factory, construct both raw sentinel
// trees, register Mesh, SkinedMesh, SkinedMeshAnimation, MatrixIndexedMesh,
// Camera, GroupParams in that order. Preserve debug words8/14 and work20/24.
// Three native unwind states destroy cache, parser tree, base as completed.
// No cleanup of the separately allocated default factory is present on failure.
void* construct_native_resource_manager_00b81040(void*, NativeResourceManagerContext&);
// Complete4C1400[189]: no native input, EAX manager, RET. Captured fast read;
// slow captured lifetime lock, double check, allocate28h/construct/publish,
// second lifetime getter THEN current resource publication read/registration.
// Construction failure frees captured manager allocation after its cleanup.
// Registration failure retains publication/allocation; both release the guard.
void* get_native_resource_manager_004c1400(NativeResourceManagerContext&);

// Complete B80F10[187]: ECX manager, RET. Delete current default factory through
// its captured current slot0 with flags1, clear/free cache then parser trees,
// clear publication, stampCE3818. Preserve factory4 and work20/24 values.
// No mapped resource/parser release. State2/1/0 unwind remaining members only.
void destroy_native_resource_manager_00b80f10(void*, NativeResourceManagerContext&);
// Complete B81150[30]: ECX manager, stack flags, EAX captured manager, RET4.
// Full destructor first; bit0 then frees the manager allocation.
void* delete_native_resource_manager_00b81150(void*, std::uint32_t, NativeResourceManagerContext&);

// Source composition adapter for the existing raw singleton manager's captured
// profile. D63128 is the primary manager; six D630xx secondary parser profiles
// adjust -4 through their actual thunks. Borrow these same publications through
// the complete drain, including any string-pool recreation during destruction.
void delete_native_resource_registered_owner(std::uintptr_t captured_profile,
    void* popped_owner, std::uint32_t flags, NativeResourceManagerContext&);

// Explicit-service C++ interfaces, not original callable ABI/FH3/SEH. Actual
// mapped literal/profile data and canonical raw string/singleton services are
// required. Parse-slot8 bodies and game executable admission remain separate.
} // namespace bsp
