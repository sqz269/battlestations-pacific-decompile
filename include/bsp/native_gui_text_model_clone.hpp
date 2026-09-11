#pragma once

#include "bsp/native_model_owner.hpp"
#include "bsp/native_mesh_owner.hpp"

namespace bsp {

enum class NativeGuiTextModelBaseCopyResult {
    copied,
    point_light_owners_required
};

// Partial B6F150 for the Text caller's flags26h,parent=null branch. Source and
// destination are distinct LIVE canonical Models in the SAME environment; the
// destination has already been pool-allocated and constructed from the current
// source name. No new allocation, default Model, alternate tree or binding.
// Positive source+168 returns point_light_owners_required BEFORE base effects.
// Otherwise executes the complete no-light branch, including current50,
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

// These fragments do NOT implement full B752B0. Its actual model allocation /
// canonical GuiWidgetOwnerRuntime adoption and current geometry10 B742A0 ->
// B73F50 deep section/material clone remain required. Raw slots cannot be cast
// to C++ owners. New MSVC Win32 interfaces, not native binary entry replacements.
} // namespace bsp
