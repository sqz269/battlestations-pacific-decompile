#pragma once
#include "bsp/native_material_effect_owner.hpp"
#include "bsp/gui_native_geometry.hpp"
#include <memory>

namespace bsp {
struct NativeMaterialEffectProgramsContext;
class NativeMaterialEffectProgramOperation;
struct NativeMaterialEffectCacheContext;
struct NativeTextureCacheContext;
struct NativeTextureCacheAcquired;
struct NativeVfsNameResolutionContext;
class NativeVfsNameResolutionAcquired;

// One concrete numeric BDF4C0 invocation. Attach its acquired frame before
// capturing the current manager; never replace or replay an interrupted call.
// Saved header/buffer identities are diagnostics after native string cleanup,
// not readable ownership. This metadata introduces no native object/count.
struct NativeMaterialEffectNameResolutionCall final {
    NativeMaterialEffectNameResolutionCall();
    ~NativeMaterialEffectNameResolutionCall();
    NativeMaterialEffectNameResolutionCall(const NativeMaterialEffectNameResolutionCall&) = delete;
    NativeMaterialEffectNameResolutionCall& operator=(const NativeMaterialEffectNameResolutionCall&) = delete;
    std::unique_ptr<NativeVfsNameResolutionAcquired> acquired;
    void* manager{};
    void* name{};
    bool frame_creation_started{}, started{}, returned{}, failed{}, result{};
    bool invoke(void* actual_mutable_header, void* const volatile& current_manager,
        NativeVfsNameResolutionContext&);
};

enum class NativeMaterialEffectLoadPhase {
    not_started, names, resolution, allocation, constructor, programs,
    name_assignment, registration, fallback, complete, failed
};
// One B2EBB0 invocation. Completed native creators are published BEFORE the
// program loader. No destructor performs native rollback. A failed frame must
// stay alive; it cannot retry or discard uninitialized-pass ownership state.
struct NativeMaterialEffectLoadAcquired final {
    NativeMaterialEffectLoadAcquired();
    ~NativeMaterialEffectLoadAcquired();
    NativeMaterialEffectLoadAcquired(const NativeMaterialEffectLoadAcquired&) = delete;
    NativeMaterialEffectLoadAcquired& operator=(const NativeMaterialEffectLoadAcquired&) = delete;
    void* raw_slot{};
    void* creator{};
    NativeMaterialEffectReference* companion{};
    // Stable same-domain metadata record, published before program ownership
    // moves. A host registration failure leaves this record inspectable.
    void* owner_record{};
    bool constructor_complete{};
    bool registered{};
    bool returned{};
    NativeMaterialEffectLoadPhase phase{NativeMaterialEffectLoadPhase::not_started};
    std::uint32_t native_site{};
    // Actual8h argument at a stable address. Host program-child failures keep
    // its allocation here with their retained frame; success performs the
    // native reverse local cleanup. This is not native FH3 exception parity.
    NativeString resolved_name;
    // Declared after the stable header so completed resolver metadata ends
    // first. Failed outer/inner frames must remain alive; no recovery is added.
    NativeMaterialEffectNameResolutionCall name_resolution;
    bool resolved_name_retained{};
    // On completed registration this SAME frame transfers to the canonical
    // effect record, surviving descriptor current0 callbacks during teardown.
    std::unique_ptr<NativeMaterialEffectProgramOperation> programs;
    // The fallback call can recursively create another real effect. Its cache
    // acquisition must also survive a nested exception; never a stack scratch.
    std::unique_ptr<struct NativeMaterialEffectCacheAcquired> fallback;
    // Stable per-call B18D60 -> B319B0 texture continuation. Completed +98 owns
    // the returned reference; failed provider state is never discarded/reset.
    std::unique_ptr<NativeTextureCacheAcquired> constructor_texture;
};

class NativeMaterialEffectLoadOwners final {
public:
    NativeMaterialEffectLoadOwners(GuiNativeGeometryOwners&,
        NativeMaterialEffectDestructionAccess&,
        const volatile std::uint32_t* profile_00d61a00);
    ~NativeMaterialEffectLoadOwners();
    NativeMaterialEffectLoadOwners(const NativeMaterialEffectLoadOwners&) = delete;
    NativeMaterialEffectLoadOwners& operator=(const NativeMaterialEffectLoadOwners&) = delete;
    void register_completed_creator(NativeMaterialEffectLoadAcquired&);
    NativeRenderActualOwners& actual_owners() noexcept;
    NativeStringStorage& string_storage() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Existing construction uses its actual string/renderer/serial domains. All
// fields below refer to those SAME live engine publications and canonical
// owners. A resolver must execute BDF4C0 against the captured actual manager
// and mutable8h header; a semantic VFS/name copy or success stand-in is invalid.
struct NativeMaterialEffectLoadingContext {
    NativeMaterialEffectConstructionAccess& construction;
    NativeMaterialEffectProgramsContext& programs;
    NativeMaterialEffectLoadOwners& owners;
    void* const volatile& current_vfs_0109ceec;
    const volatile std::uint32_t* renderer_profile_00d5f0a8;
    void* resolution_context;
    bool (*resolve_existing_name_00bdf4c0)(void*, void* actual_manager, void* actual_name);
    const char* empty_string_0108d5a4;
    // Set to the SAME cache context after constructing the mutually borrowing
    // contexts. It must point back to this effects context before any load.
    NativeMaterialEffectCacheContext* cache{};
    // Required at the found-path constructor arm. SAME renderer/string and
    // canonical texture owner domain. No callable-table substitute is supplied.
    NativeTextureCacheContext* texture_cache{};
    // Exactly one provider: this numeric domain OR the explicit callback above.
    // Numeric selection requires resolution_context=null. Configure under
    // external quiescence; contexts/cells outlive every retained invocation.
    NativeVfsNameResolutionContext* numeric_name_resolution{};
};

// Explicit callback interface retained for existing providers. Numeric mode is
// rejected here because this overload has no retained invocation frame.
bool resolve_native_material_effect_name(void* actual_mutable_name,
    NativeMaterialEffectLoadingContext&);
// Same actual stable resolved8h header; numeric mode owns a distinct resolver
// frame in this load before its current-manager capture. Numeric failures
// propagate without callback fallback. No name cleanup/retain is added.
bool resolve_native_material_effect_name(void* actual_mutable_name,
    NativeMaterialEffectLoadingContext&, NativeMaterialEffectLoadAcquired&);
// Full B18F70: ECX actual effect; stack native name; RET4. Identity skips;
// otherwise resize actual+B8 then copy using live source/destination headers.
void set_native_material_effect_name_00b18f70(NativeMaterialEffectBaseStorage&,
    const void* actual_name, NativeStringStorage&);
// B2EBB0: unused registry ECX; name/ignored loader word stack; owned EAX; RET8.
// Rewrites all .mshd occurrences before actual mutable VFS resolution. A found
// path constructs178h, calls B46950 (AL ignored), assigns original name+B8.
// Missing path emits the native diagnostic and recursively calls CURRENT
// renderer48(error.shfx). No recursion sentinel, null-success repair or cache.
void* load_native_material_effect_00b2ebb0(const void* actual_name,
    std::uint32_t ignored_loader_word, NativeMaterialEffectLoadingContext&,
    NativeMaterialEffectLoadAcquired&);
} // namespace bsp
