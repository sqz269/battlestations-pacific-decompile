#pragma once
#include "bsp/native_material_pass_copy.hpp"
#include "bsp/native_shader_descriptor_owner.hpp"
#include <memory>

namespace bsp {
// Child implementations must attach their acquired native state here BEFORE a
// later throwing call. A failure is not replayable; the enclosing load retains
// this exact frame and all descriptor/name arguments. No successful fallback.
class NativeMaterialProgramChildFrame {
public:
    virtual ~NativeMaterialProgramChildFrame() = default;
};
using NativeMaterialProgramChild = std::unique_ptr<NativeMaterialProgramChildFrame>;

struct NativeMaterialProgramRequest {
    NativeMaterialEffectStorage& effect_ecx;
    NativeShaderDescriptorStorage& descriptor_edx;
    NativeShaderDescriptorStorage& mode_descriptor;
    std::uint8_t mode_flag;
    std::uint32_t render_target_count;
    std::uint8_t descriptor_flag;
    const NativeString& program_name;
    std::uint8_t policy;
    std::uint32_t generation;
};
// These are the still-unimplemented actual native children, not permission to
// translate ShaderLuaCode or CompiledMaterialPass into successful raw storage.
// Returning from read means the ENTIRE B43B00 descriptor body is populated.
// A nonnull compiled pass is fully constructed/registered in the SAME actual
// owner domain and transfers the native returned reference, without a retain.
// nullptr is the native compiler failure, distinct from an unavailable child.
// State-cache calls transfer the incoming held reference to their actual result
// using B26500/B265C0/B26680 retain/release ordering. No identity-only stand-in.
// On normal return, a child frame's destructor is metadata-only: all completed
// owners have transferred to the caller/actual registration. On failure it must
// retain its real acquisitions; the parent neither resets nor retries it.
// Save argument VALUES and the supplied actual descriptor/name addresses, never
// an address of the transient request struct or other caller stack operands.
class NativeMaterialEffectProgramChildren {
public:
    virtual ~NativeMaterialEffectProgramChildren() = default;
    virtual void read_descriptor_00b43b00(NativeShaderDescriptorStorage&,
        const void* actual_name_header, std::uint32_t generation,
        NativeMaterialProgramChild&) = 0;
    virtual bool resolve_name_00bdf4c0(void* actual_vfs,
        NativeString& actual_name, NativeMaterialProgramChild&) = 0;
    virtual NativeMaterialPassStorage* build_program_00b3c3a0(
        const NativeMaterialProgramRequest&, NativeMaterialProgramChild&) = 0;
    virtual NativeMaterialStateOwnerStorage* cache_render_00b26500(void* renderer,
        NativeMaterialStateOwnerStorage*, NativeMaterialProgramChild&) = 0;
    virtual NativeMaterialStateOwnerStorage* cache_third_00b265c0(void* renderer,
        NativeMaterialStateOwnerStorage*, NativeMaterialProgramChild&) = 0;
    virtual NativeMaterialStateOwnerStorage* cache_sampler_00b26680(void* renderer,
        NativeMaterialStateOwnerStorage*, NativeMaterialProgramChild&) = 0;
};

struct NativeMaterialEffectProgramsContext {
    NativeStringStorage& strings;
    NativeMaterialEffectDestructionAccess& lifetime;
    NativeMaterialPassConstructionAccess& pass_construction;
    NativeMaterialPassCopyAccess& pass_copy;
    NativeMaterialSecondaryPassRegistration pass_registration;
    NativeMaterialEffectProgramChildren& children;
    void* const volatile& current_renderer_00f8d394;
    void* const volatile& current_manager_00f8bbf0;
    void* const volatile& current_vfs_0109ceec;
    const volatile std::uint8_t& load_variants_0108d6f0;
    // Borrow the actual mutable D5F0A8 profile, not a host callable table.
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8{};
};

// Full B5F160, ECX actual pass, RET: six conditional render-state groups,
// CURRENT callable renderer+104 result byte3D, then thirteen unconditional
// removals. Requires the actual renderer binding, not a copied caps snapshot.
void prune_native_material_pass_states_00b5f160(NativeMaterialPassBaseStorage&,
    void* const volatile& current_renderer_00f8d394);
// Actual numeric-profile route used by B5F6A0/B17DD0. After the same six
// groups, capture current renderer, its profile and current+104 target once;
// dispatch the substantive B1FF50 provider and read its returned byte+3D.
// Unsupported targets fail at that call, preserving all prior removals.
void prune_native_material_pass_states_00b5f160(NativeMaterialPassBaseStorage&,
    void* const volatile& current_renderer_00f8d394,
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8);
// B5F6A0, ECX pass, RET: prune then publish actual cache results18/1C/20,
// reloading renderer and next input around each call. Required child state is
// retained by the caller on error; successful earlier substitutions remain.
void finalize_native_material_pass_00b5f6a0(NativeMaterialPassBaseStorage&,
    NativeMaterialEffectProgramsContext&, NativeMaterialProgramChild&);
// B17DD0, ECX effect, stack pass, RET4: current0C, borrowed14, append9C,
// increment live countA8, then actual pass+04. No old-owner release.
void retain_native_material_effect_pass_00b17dd0(NativeMaterialEffectStorage&,
    NativeMaterialPassStorage&, NativeMaterialEffectProgramsContext&,
    NativeMaterialProgramChild&);
// Full 711370 actual output8h, ECX output, stack unsignedDWORD, EAX output,
// RET4. Decimal CRT formatting is lowered to to_chars; two native pooled
// copies and temporary release remain. Does not release prior output storage.
NativeString* construct_native_material_program_number_00711370(NativeString&,
    std::uint32_t, NativeStringStorage&);

enum class NativeMaterialEffectProgramPhase {
    fresh, first_load, release_derived, release_base, second_load, complete, failed
};
class NativeMaterialEffectProgramOperation final {
public:
    NativeMaterialEffectProgramOperation();
    ~NativeMaterialEffectProgramOperation();
    NativeMaterialEffectProgramOperation(const NativeMaterialEffectProgramOperation&) = delete;
    NativeMaterialEffectProgramOperation& operator=(const NativeMaterialEffectProgramOperation&) = delete;
    bool complete() const noexcept;
    bool pass_slots_initialized() const noexcept;
    bool has_live_bindings() const noexcept;
    NativeMaterialEffectProgramPhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    std::uint32_t primary_slots_written() const noexcept;
    NativeMaterialProgramChildFrame* active_child() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend std::uint8_t load_native_material_effect_variants_00b46950(
        NativeMaterialEffectStorage&, const void*, NativeMaterialEffectProgramsContext&,
        NativeMaterialEffectProgramOperation&);
};
// Original B46950 ECX effect, stack name8h, AL result, RET4. The effect must
// have completed actual B407A0; C8..137 may still be uninitialized. The retained
// operation is one-shot and must outlive effect descriptor deletion, including
// after successful loading. Allocate it before the native creator if host
// allocation failure would otherwise discard that creator. No raw Text/effect
// overlay, blanket slot initialization, destructor rollback or count duplicate.
// Flag0 returns B45EE0(name,0,0). Flag!=0 performs B45EE0(name,0,3), derived
// release, base release, B45EE0(name,1,3), returns1 ignoring both native ALs.
// Unsafe cleanup after an early compiler-null result with uninitialized slots
// is an explicit host error; its unwritten preimages remain unchanged.
std::uint8_t load_native_material_effect_variants_00b46950(
    NativeMaterialEffectStorage&, const void* actual_name_header,
    NativeMaterialEffectProgramsContext&, NativeMaterialEffectProgramOperation&);
} // namespace bsp
