#pragma once
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_logical_vertex_owner.hpp"

namespace bsp {

// Complete four-byte native leaves: ECX actual generator, EAX raw pointer, RET.
// The B55B20 producer stores declaration/layout at +10/+14, respectively.
void* native_instance_generator_declaration_00b556b0(const void*) noexcept;
void* native_instance_generator_layout_00b556c0(const void*) noexcept;

struct NativeInstanceGeometryAccess {
    NativeModelEnvironment& models;
    GuiNativeGeometryOwners& geometry;
    NativeLogicalVertexOwnerContext& streams;
    NativeMaterialDestructionAccess& materials;
    NativeStringStorage& strings;
    const volatile std::uint32_t& unchanged_00d7a260;
    const volatile std::uint32_t* material_vtable_00d5e520;
    void* companion_context;
    // Transactional HOST bookkeeping over the SAME freshly allocated188h
    // slot and supplied environment. Preserve every native byte; no retain.
    NativeModelOwner& (*prepare_model)(void*, void*, NativeModelEnvironment&);
    // Only a failed native constructor/preparation is retired here. The owner
    // is prepared or dead; erase its host companion without native destruction
    // or pool return. This runs BEFORE returning its raw slot to the same pool.
    void (*retire_failed_model)(void*, NativeModelOwner&) noexcept;
    // Transactional canonical registration of ONE companion borrowing actual
    // +04. No retain/native stores. On throw leave the live owner available in
    // acquired, with no registration; native construction is not rolled back.
    NativeModelReference& (*bind_completed_model)(void*, NativeModelOwner&);
    NativeLogicalVertexReference& (*bind_completed_stream)(void*, void*,
        NativeLogicalVertexOwnerContext&);
};

// Diagnostic publication of the actual acquired creators, not native storage
// or an ownership map. Initially empty. Later failures preserve all prior
// native effects; nonnull creators need explicit final cleanup by the caller.
// Clear immediately before release so a throwing terminal cannot be retried.
struct NativeInstanceGeometryAcquired {
    NativeModelOwner* model_owner{};
    NativeModelReference* model_reference{};
    NativeMeshStorage* mesh{};
    void* stream{};
    NativeMaterialStorage* material{};
    NativeMeshSectionStorage* section{};
};

// Complete B4C8D0 normal body: ECX native8h name header, EDX generator;
// stack actual mesh, selected section; EAX actual new model; RET8. Uses actual
// canonical pools, model/mesh/section/material owners and renderer B287C0.
// The renderer must carry CURRENT D5F0A8, whose actual5C is B287C0. All owner,
// string, physical-buffer and model sentinel bindings share the same domains.
// One model creator remains on success; temporary creator fields become null.
// Native constructor allocation unwind is retained by the existing factories;
// there is no rollback of a completed model/mesh on a later native exception.
// Null allocation's subsequent native access violation and host-registration
// failure are diagnostic C++ boundaries, not native FH3/SEH equivalence.
void* create_native_instance_geometry_00b4c8d0(const NativeString& name,
    const void* actual_generator, const NativeMeshStorage& source_mesh,
    const NativeMeshSectionStorage& selected_section, NativeInstanceGeometryAccess&,
    NativeInstanceGeometryAcquired&);

// New MSVC Win32 source interfaces; no semantic geometry overlays, private
// refcounts, fake factory callbacks, binary detour ABI or gameplay claim.
} // namespace bsp
