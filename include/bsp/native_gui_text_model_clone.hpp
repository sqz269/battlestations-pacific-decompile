#pragma once

#include "bsp/native_model_owner.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_mesh_clone.hpp"

namespace bsp {
class GuiWidgetOwnerRuntime;
class GuiNativeGeometryOwners;
struct NativeMaterialDestructionAccess;

enum class NativeGuiTextModelBaseCopyResult {
    copied
};

// B6F150 for flags26h/3Eh,parent=null. Other flags/parents remain outside this
// fragment. Source and
// destination are distinct LIVE canonical Models in the SAME environment; the
// destination has already been pool-allocated and constructed from the current
// source name. Positive lists append each actual light and raw node backlink
// through the same NativeNodeDestructionRuntime.point_lights domain, then current50,
// retained130 twice, null parenting, root registration, current38, and final
// stores. Flags20 skips source child clones; no child-recursion API is invented.
// Every nonnull retained130 needs the existing actual node retained binding.
// Exceptions after callbacks preserve partial state; this is not a resumable
// transaction and must not be restarted to repeat those effects.
NativeGuiTextModelBaseCopyResult copy_native_gui_text_model_base_00b6f150_fragment(
    NativeModelOwner& source, NativeModelOwner& destination);

// B75331..B75364 ONLY: entry requires an actual completed current-geometry10
// clone with ONE acquired creator reference. Resolve its SAME canonical mesh
// companion, read source17C then178 AFTER the clone callback, assign geometry,
// then consume that acquired reference. No mesh copy or fallback is supplied.
// Clear the caller's owned slot immediately before release, so a throwing
// terminal callback cannot cause a second release. Other failures leave it
// owned by the caller. Both Models stay borrowed and live through callbacks.
void associate_native_gui_text_model_clone_geometry_00b752b0_fragment(
    NativeModelOwner& source, NativeModelOwner& destination,
    NativeMeshStorage*& acquired_geometry);

// B75365..B753F1 ONLY: entry is after the entire geometry branch (or after
// testing CURRENT source180 null). Copy retained174 with publish/retain/release
// ordering, then ten individual x87 FLD/FSTP source+08..2C pairs. No cached
// source snapshot; release callbacks may change the source before those reads.
void finish_native_gui_text_model_clone_00b752b0_fragment(
    NativeModelOwner& source, NativeModelOwner& destination);

// Acquired references belonging to the caller, not a second native owner.
// All fields must initially be null. After construction, model retains the
// native creator reference through all later effects. Mesh.mesh is nonnull
// after mesh construction/registration until consuming its creator reference,
// including partially completed mesh copying after a later exception.
// Exceptions preserve these publications; neither references nor native
// effects are silently rolled back. Release through their canonical owners.
struct NativeGuiTextModelCloneAcquired {
    NativeModelReference* model{};
    NativeMeshCloneAcquired mesh;
};

// Compose B752B0 for the glyph route's flags26h,parent0. Allocates and
// constructs a Model from the live source name, performs base copy, clones
// current mesh geometry into the same actual owner domain, associates it,
// releases the geometry creator, then executes the retained174/pose tail.
// Positive lists require live actual light descriptor bindings; array growth
// requires matching backing provenance. Exceptions preserve acquired.model
// and prior native effects; they are diagnostic and cannot be resumed/restarted.
// Only successful return with copied means the complete supported clone path.
NativeGuiTextModelBaseCopyResult clone_native_gui_text_model_00b752b0(
    NativeModelOwner& source, GuiWidgetOwnerRuntime&, GuiNativeGeometryOwners&,
    NativeMaterialDestructionAccess&,
    const volatile std::uint32_t* material_current_vtable_00d5e520,
    const volatile std::uint32_t* mesh_current_vtable_00d62d60,
    NativeGuiTextModelCloneAcquired& acquired);

// Exact AA9520 Text/type3 flags3E,parent0 route. Same Model/base/association
// tail as26, with actual separately owned geometry streams. This does not adopt
// a copied Text runtime or retain a raw Text allocation. Reached stream factory,
// registration/map interruptions remain on acquired.mesh.stream.
NativeGuiTextModelBaseCopyResult clone_native_gui_text_model_00b752b0_flags3e(
    NativeModelOwner& source, GuiWidgetOwnerRuntime&, GuiNativeGeometryOwners&,
    NativeMaterialDestructionAccess&,
    const volatile std::uint32_t* material_current_vtable_00d5e520,
    const volatile std::uint32_t* mesh_current_vtable_00d62d60,
    NativeGuiTextModelCloneAcquired&, NativeStreamCloneServices&);

// Other flags/parent combinations remain outside these composed Text routes.
// Raw slots cannot be cast to C++ owners.
// New MSVC Win32 interfaces, not native binary entry replacements.
} // namespace bsp
