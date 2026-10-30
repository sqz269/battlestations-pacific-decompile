#pragma once
#include "bsp/native_gui_text_failure_origin.hpp"
#include "bsp/native_instance_geometry.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_node_parenting_storage.hpp"
#include "bsp/native_model_storage_geometry.hpp"
#include <optional>

namespace bsp {
namespace game { class GameNativeStringProcess; }
// These are borrowed EXISTING actual owner/import domains. No Text companion,
// pool, renderer, registry, callable-table substitute or implicit native owner.
struct NativeGuiTextSectionStorageContext {
    NativeInstanceGeometryAccess& actual;
    const NativeNodeRawConstants& node_constants;
    NativeStringRawPoolContext& strings;
    game::GameNativeStringProcess& string_process; // already existing, never created here
    NativeMaterialEffectCacheContext& effects;
    NativeNodeParentingContext& parenting;
    NativeTextureSurfaceReferenceIncrement const volatile& decrement_00ce2220;
    const char* shadow_00d15ec4;
    const char* guidefault_00d5c25c;
};
struct NativeGuiTextSectionStorageFrame {
    // Caller-established initialized, aligned, disjoint preimages. The SAME
    // four words are mask/raw allocation/name length/name data. Original after
    // all pushes F=entryESP-2C: F10/F14/F18/F1C. Header identity survives every
    // provider and failure. No placement NativeString or blanket initialization.
    volatile std::uint32_t (&locals)[4];
    float scalar_178, scalar_17c;
    NativeNodeParentingFrame& parenting;
};
struct NativeGuiTextSectionStorageRelease {
    void* identity{}; // diagnostic only after terminal; never inspect again
    NativeTextureSurfaceReferenceIncrement captured_decrement{};
    long result{};
    bool entered{}, decremented{}, zero_entered{}, zero_returned{}, complete{};
};
struct NativeGuiTextSectionStorageSection {
    GuiNativeSectionAcquired section;
    NativeMaterialFactoryAcquired material;
    NativeMeshSectionStorage* section_creator{};
    NativeMaterialStorage* material_creator{};
    bool appended{}, material_published{};
    NativeGuiTextSectionStorageRelease section_release, material_release;
};
struct NativeGuiTextSectionStorageAcquired {
    // provider_call names an entered SOURCE provider boundary, not proof of
    // an Original/native exception origin. Unobservable nested host failures
    // remain at that boundary; future callers must not infer native origin.
    enum class Failure { none, provider_call, host_metadata };
    bool started{}, complete{}, exception_cleanup_started{};
    bool model_constructed{}, shadow_published{}, model_registered{};
    bool raw_model_return_entered{}, raw_model_returned{}, raw_mesh_return_entered{};
    bool conditional_name_consumed{};
    std::uint32_t active_call_site{}, completed_name_returns{};
    std::int32_t native_eh_state{-1};
    Failure failure{Failure::none};
    NativeModelOwner* model_owner{};
    NativeModelReference* model_creator{}; // same creator, not another reference
    void* captured_shadow_receiver{};
    GuiNativeMeshAcquired mesh;
    NativeMeshStorage* mesh_creator{};
    NativeGuiTextSectionStorageSection shadow_section, main_section;
    NativeTextureSurfaceReferenceIncrement shadow_pair_decrement{};
    NativeTextureSurfaceReferenceIncrement main_pair_decrement{};
    NativeGuiTextSectionStorageRelease mesh_release;
    NativeNodeSetParentAcquired parenting;
};

// Complete normal AB8530[668B/208 instructions], ECX actual Text/no args/RET.
// Both current188 and late current4C branches are represented through actual
// storage and genuine providers. Existing nonnull188/4C must be live canonical
// Models in actual.models; all native allocation results subsequently accessed
// must be nonnull. Current profile/array/hierarchy domains inherit providers.
//
// Preflight performs no native effects. Entered calls are one-shot, including
// failure. Keep raw receiver, frame/backing, contexts, acquired records and all
// completed creators/companions alive after failure. Native construction-only
// cleanup follows states0..5; host metadata failures preserve reached state.
// Frame, locals, parent frame and diagnostics are mutually disjoint and do not
// alias the raw Text or any borrowed provider/context/import/constant storage.
// Provider graphs may share their required canonical identities, but none may
// alias these caller-mutable regions. Existing transitive provider contracts
// cover their reached raw objects, tables, arrays and persistent nested frames.
// The borrowed existing string_process authenticates BOTH raw_context() and
// strings() identities. No accessor creates a process, native pool or manager.
// There is no RAII rollback, automatic retry, extra retain or Text registration.
// No actual native Text C++ lifetime, FH3/SEH/private-stack or binary ABI claim.
void ensure_native_gui_text_sections_storage_00ab8530(void* actual_text,
    NativeGuiTextSectionStorageFrame&, NativeGuiTextSectionStorageContext&,
    NativeGuiTextSectionStorageAcquired&);

// Explicit raw-Model transport. No legacy Model environment, callback cast or
// semantic Text field authority is supplied. All referents are genuine retained
// providers; GuiDefault/effect-cache/Lua readiness is an external precondition.
struct NativeGuiTextRawModelSectionStorageContext {
    NativeModelStorageIdentityContext& model;
    NativeModelStorageGeometryContext& association;
    GuiNativeGeometryOwners& geometry;
    NativeMaterialDestructionAccess& materials;
    NativeStringStorage& material_strings;
    NativeStringRawPoolContext& strings;
    game::GameNativeStringProcess& string_process;
    NativeMaterialEffectCacheContext& effects;
    NativeNodeParentingContext& parenting;
    const volatile std::uint32_t* material_vtable_00d5e520;
    const char* shadow_00d15ec4;
    const char* guidefault_00d5c25c;
};
// The four B75170 operands have real DWORD lifetimes. AB8618/8620 spill
// DIRECTLY to arguments[3]/[2], then PUSH EBX/0 publish [1]/[0]. No float
// snapshot, wrong-type reference or temporary operand frame is introduced.
struct NativeGuiTextRawModelSectionStorageFrame {
    volatile std::uint32_t (&locals)[4];
    volatile std::uint32_t (&arguments)[4];
    NativeModelStorageGeometryFrame association;
    NativeNodeParentingFrame& parenting;
    NativeGuiTextRawModelSectionStorageFrame(volatile std::uint32_t (&l)[4],
        volatile std::uint32_t (&args)[4], NativeNodeParentingFrame& p) noexcept
        : locals(l), arguments(args), association(args[0], args[1], args[2], args[3]), parenting(p) {}
};
struct NativeGuiTextRawModelRecord {
    // Independent of Text/companion storage. Keep through credits, entered
    // failure and any unreturned Model physical slot (also completed flags0).
    void* actual{};
    NativeModelStorageConstructAcquired construction;
    NativeModelStorageLifetimeFrame lifetime_frame{};
    NativeModelStorageLifetimeAcquired lifetime;
    std::optional<NativeModelStorageReference> reference;
    NativeModelStorageGeometryAcquired association;
    NativeNodeTreeRetirementFrame release_frame{};
    NativeNodeTreeRetirementAcquired release;
};
struct NativeGuiTextRawModelSectionStorageAcquired {
    using Failure = NativeGuiTextSectionStorageAcquired::Failure;
    bool started{}, complete{}, exception_cleanup_started{};
    bool model_constructed{}, shadow_published{}, model_registered{};
    bool raw_model_return_entered{}, raw_model_returned{}, raw_mesh_return_entered{};
    bool conditional_name_consumed{};
    std::uint32_t active_call_site{}, completed_name_returns{};
    std::int32_t native_eh_state{-1};
    Failure failure{Failure::none};
    NativeGuiTextRawModelRecord model;
    void* captured_shadow_receiver{};
    GuiNativeMeshAcquired mesh;
    NativeMeshStorage* mesh_creator{};
    NativeGuiTextSectionStorageSection shadow_section, main_section;
    NativeTextureSurfaceReferenceIncrement shadow_pair_decrement{}, main_pair_decrement{};
    NativeGuiTextSectionStorageRelease mesh_release;
    NativeNodeSetParentAcquired parenting;
    const NativeGuiTextFailureOrigin& failure_origin() const noexcept { return failure_origin_; }
private:
    friend struct NativeGuiTextRawModelSectionFailureAccess;
    friend void ensure_native_gui_text_sections_storage_00ab8530(void*,
        NativeGuiTextRawModelSectionStorageFrame&, NativeGuiTextRawModelSectionStorageContext&,
        NativeGuiTextRawModelSectionStorageAcquired&, const NativeGuiTextFailureOriginRuntime&);
    const NativeGuiTextFailureOriginRuntime* failure_runtime_{};
    NativeGuiTextFailureOrigin failure_origin_;
};

// DEFAULT-only admission: no primary4C/parent/children/timed/glyph/cursor/
// cached Text shader publication or mutation. Internal188 is mandatory.
// Prepare the WHOLE frame/acquisition/model/provider graph before ANY pool
// effect. Entered/failed frames cannot be replayed, recycled or destroyed.
// host_metadata is extra admission, not Native ctor failure/cleanup permission.
void ensure_native_gui_text_sections_storage_00ab8530(void* actual_text,
    NativeGuiTextRawModelSectionStorageFrame&, NativeGuiTextRawModelSectionStorageContext&,
    NativeGuiTextRawModelSectionStorageAcquired&);
// In-flight constructor85 route ONLY, distinct from the completed-object POST
// API. Private HOST/UNKNOWN receipts withhold this entry's own nested cleanup.
void ensure_native_gui_text_sections_storage_00ab8530(void* actual_text,
    NativeGuiTextRawModelSectionStorageFrame&, NativeGuiTextRawModelSectionStorageContext&,
    NativeGuiTextRawModelSectionStorageAcquired&, const NativeGuiTextFailureOriginRuntime&);
} // namespace bsp
