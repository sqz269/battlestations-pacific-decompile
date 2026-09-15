#pragma once
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_mesh_clone.hpp"
#include "bsp/native_mesh_section.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_material_factory.hpp"
#include <memory>

namespace bsp {
struct NativeVertexDeclarationLoadingContext;
struct GuiNativeSectionAcquired {
    NativeMeshSectionStorage* creator{};
    RenderCommandReference* companion{};
    void* owner_record{};
    bool started{}, registered{};
};
struct GuiNativeDeclarationAcquired {
    void* reference{}; // Caller reference returned by the actual cache.
    RenderCommandReference* companion{}; // Borrows SAME actual+04.
    bool canonical_registration{};
    bool reused_companion{};
};
// Registration into the SAME canonical NativeRenderActualOwners used by the
// supplied model, mesh and section environments. This is not another resolver.
// bind is transactional: throwing leaves no registration; unbind cannot throw.
// Neither operation retains/releases or reads returned pool storage.
struct GuiNativeGeometryRegistration {
    NativeRenderActualOwners& owners;
    void* context;
    void (*bind)(void*, void* actual, RenderCommandReference&);
    void (*unbind)(void*, void* actual, RenderCommandReference&) noexcept;
    // Nonmutating lookup in this SAME canonical registry. nullptr means absent;
    // lookup failure throws and must never be interpreted as absence. Required
    // when declaration registration is reached; older aggregates stay explicit
    // unsupported at that boundary. No reference operation is performed.
    RenderCommandReference* (*find)(void*, void* actual){};
};

// Holds host companions only. Every mesh/section count is its actual +04.
// Explicit native releases, including model+180 destruction, must finish
// before this companion domain, its canonical registry or its environments die.
// No GeneratedInstanceGeometry, LogicalVertexStream or MaterialCloneState is
// overlaid, copied or treated as a native object by this class.
class GuiNativeGeometryOwners final {
public:
    GuiNativeGeometryOwners(NativeMeshEnvironment&, NativeMeshConstants,
        NativeMeshSectionEnvironment&, GuiNativeGeometryRegistration);
    ~GuiNativeGeometryOwners();
    GuiNativeGeometryOwners(const GuiNativeGeometryOwners&) = delete;
    GuiNativeGeometryOwners& operator=(const GuiNativeGeometryOwners&) = delete;

    // Actual canonical C0h / 64h slots; constructor + canonical registration;
    // return ONE creator reference. Release through the canonical owners.
    NativeMeshStorage* create_mesh();
    NativeMeshSectionStorage* create_section();

    // Actual533FA0 followed by metadata-only canonical admission. A completed
    // native creator/companion remains in acquired on host allocation/bind
    // failure; no destructor rollback or extra reference. One fresh call only.
    NativeMeshSectionStorage* create_native_section_00533fa0(GuiNativeSectionAcquired&);

    // B742A0 -> B73F50, Text flags26h or exactly3E when streams is supplied.
    // Source is a registered actual
    // D62D60 mesh. Explicit actual current table has at least five DWORDs;
    // its +10 must be B742A0. Allocate a distinct mesh; share actual streams
    // for26, clone actual streams for3E;
    // copy sections, clone their materials, copy LOD/optional fields/names.
    // Return ONE registered creator reference in this SAME owner domain.
    // Material access/profile survive every resulting material's final release.
    // Null allocation throws; it cannot produce an empty successful clone.
    // Acquired starts empty; receives mesh BEFORE copying and retains partial
    // native effects plus any pending creators on exception. Do not retry it.
    NativeMeshStorage* clone_mesh_for_text_00b742a0(NativeMeshStorage& source,
        const volatile std::uint32_t* current_vtable_00d62d60,
        NativeMaterialDestructionAccess&,
        const volatile std::uint32_t* current_vtable_00d5e520,
        NativeMeshCloneAcquired& acquired, NativeStreamCloneServices* flags3e_streams = nullptr);

    // Host registration ONLY after a real stream factory completes. Borrow its
    // actual+04 through the corresponding native logical reference; no new count,
    // native object, retain or renderer registration. On metadata/bind failure,
    // the acquired raw creator and any companion remain published for diagnosis.
    // A noncanonical companion may be released directly after maps/caller effects
    // are resolved. A raw creator without companion still needs its real terminal
    // context. Never drop or retry the interrupted clone to perform cleanup.
    void register_stream_clone_creator(NativeStreamCloneAcquired&, NativeStreamCloneServices&);

    // Register/reuse the cache-returned actual declaration without AddRef.
    // Cache creator and caller reference retain their native counts. On bind
    // failure keep the raw reference and any unbound companion in acquired;
    // release only through its actual terminal after caller effects are resolved.
    void register_native_declaration_reference(GuiNativeDeclarationAcquired&,
        NativeVertexDeclarationLoadingContext&);

    // Actual copied creators used by B73F50. Each result is canonically
    // registered with +04=1; caller transfers/releases that creator reference.
    NativeMeshSectionStorage* clone_section_00b85ef0(const NativeMeshSectionStorage&);
    NativeMaterialStorage* clone_material_00b18b60(const NativeMaterialStorage&,
        NativeMaterialDestructionAccess&,
        const volatile std::uint32_t* current_vtable_00d5e520);

    // Existing00535320 actual factory plus canonical material companion.
    // Return one creator reference; SAME registration/retained-owner domain.
    // Access and current profile view outlive this material's final release.
    // Renderer+48 must return an actual registered effect; no shader fallback.
    NativeMaterialStorage* create_material_for_effect_00535320(NativeString&,
        void* const volatile& current_renderer_00f8d394,
        NativeMaterialDestructionAccess&,
        const volatile std::uint32_t* current_vtable_00d5e520);

    // Metadata-only admission of a completed native-cache factory creator.
    // No retain or destructor rollback. Failed bind leaves the raw creator,
    // stable entry and unbound companion in acquired for explicit resolution.
    void register_native_material_creator(NativeMaterialFactoryAcquired&,
        NativeMaterialDestructionAccess&,
        const volatile std::uint32_t* current_vtable_00d5e520);

    // Common +74 fragment AB2563..AB25C3 / ACF913..ACF973. Caller has
    // already tested widget+74==0 and selected its SAME widget+4C model.
    // Allocate BC payload, read actual D7A260 once, B75170(0,raw,s,s),
    // release creator. The existing caller must then perform AA7DC0(zero).
    // Sentinels preserve model+178/+17C through the existing native setter.
    // No raw mesh pointer is returned after a potentially reentrant release.
    void construct_and_associate74_fragment(NativeModelOwner&);

    NativeRenderActualOwners& actual_owners() noexcept;
    // Borrow the SAME stored canonical bind/find/unbind domain for composing
    // other actual resource owners; this never creates a second registry.
    const GuiNativeGeometryRegistration& registration() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// One acquired section reference, either newly allocated or retained from
// actual mesh+54[0]. append_on_publish is host control flow, not a native word.
struct GuiNativeSectionSelection {
    NativeMeshSectionStorage* section;
    bool append_on_publish;
};
GuiNativeSectionSelection acquire_gui_native_section_fragment(
    NativeMeshStorage&, GuiNativeGeometryOwners&);

// AB443D..AB4488 / AD150D..AD155C. Entry is AFTER actual stream unlock and
// AA7220; caller has written section primitive/ranges and configured the SAME
// actual material (parameters, clip/widget owner, texture). Mesh stream0 must
// be a real registered native stream and layout services must dispatch the
// current actual renderer+40. No fake stream, material, layout or queue.
// Set material; consume temporary material ref; rebuild layout; append iff
// new; consume selected section ref. Successful return leaves both pointers
// null. Caller must invoke widget virtual+50 AFTER this fragment.
// On exception, raw publications stand; nonnull pointers still belong to the
// caller. Clearing each pointer BEFORE release prevents retrying a throwing
// terminal callback. This is host bookkeeping, not native exception parity.
void publish_gui_native_section_fragment(NativeMeshStorage&,
    GuiNativeSectionSelection&, void*& owned_actual_material,
    NativeRenderActualOwners&, NativeMeshSectionLayoutServices&);

// Deliberately not a GuiGeometryRuntimeServices adapter: its typed rebuild
// still lacks native material/stream/layout projections. Filling only the
// typed GeneratedInstanceGeometry never populates raw mesh+54 and does not
// make native GUI bounds or rendering usable. See GUI_NATIVE_GEOMETRY.md.
} // namespace bsp
