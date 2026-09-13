#pragma once
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
class GuiWidgetOwner;
class GuiWidgetOwnerRuntime;
struct GuiTextWidget;
struct GuiTextBufferServices;
class NativeNodeBinding;
struct NativeMeshStorage;
struct NativeMeshSectionStorage;
struct NativeMaterialStorage;
class NativeModelReference;
class NativeModelOwner;

enum class GuiTextSectionPhase { not_started, running, complete, failed };
enum class GuiTextSectionModelPhase { not_started, metadata, allocation, name, constructor,
    native_constructed, reference_registration, published, raw_cleaned };
enum class GuiTextSectionFactory { none, mesh, section, material };
enum class GuiTextSectionReleasePhase { not_called, entered, returned };

// Shared source bookkeeping for the independently recovered AB8530/AB8910
// prefixes. No native field, owner, retain, destructor cleanup or retry. Each
// caller supplies its own sites/FH3 states and retains this SAME subobject.
struct GuiTextAuxiliaryModelAcquired {
    GuiTextAuxiliaryModelAcquired() noexcept = default;
    GuiTextAuxiliaryModelAcquired(const GuiTextAuxiliaryModelAcquired&) = delete;
    GuiTextAuxiliaryModelAcquired& operator=(const GuiTextAuxiliaryModelAcquired&) = delete;
    std::uint32_t native_site{};
    std::uint32_t failure_site{};
    std::uint32_t cleanup_site{};
    std::int32_t native_unwind_state{-1};
    // One eight-byte native header, reused only after cleanup. Destruction
    // leaves its bits intact; these booleans are source bookkeeping only.
    NativeString temporary_name;
    NativeStringStorage* temporary_name_storage{};
    bool name_live{};
    bool name_cleanup_armed{};
    bool name_constructing{};
    GuiTextSectionModelPhase model_phase{GuiTextSectionModelPhase::not_started};
    GuiTextSectionModelPhase model_failure_phase{GuiTextSectionModelPhase::not_started};
    bool model_metadata_failure{};
    void* unconstructed_model_slot{};
    NativeModelOwner* constructed_model_owner{};
    NativeModelReference* model_creator{};
};

// One AB8530 invocation's native locals and diagnostic progress, held by the
// SAME Text lifetime. These are creator references, not additional retains or
// native objects. Keep this frame and its providers alive after a failure.
// No destructor releases or rolls back native state. Failed work cannot rerun.
struct GuiTextSectionOperation final : GuiTextAuxiliaryModelAcquired {
    GuiTextSectionOperation() noexcept = default;
    GuiTextSectionOperation(const GuiTextSectionOperation&) = delete;
    GuiTextSectionOperation& operator=(const GuiTextSectionOperation&) = delete;
    bool has_incomplete() const noexcept {
        return phase == GuiTextSectionPhase::running || phase == GuiTextSectionPhase::failed;
    }

    GuiTextSectionPhase phase{GuiTextSectionPhase::not_started};
    // A failed composed factory records its native caller entry site, not an
    // invented instruction inside its allocation/registration implementation.
    GuiTextSectionFactory factory_in_flight{GuiTextSectionFactory::none};

    // The inherited name header serves Shadow, shadow effect and main effect.
    // Its actual Model creator transfers to live Text+188 before name release.
    NativeMeshStorage* mesh{};
    NativeMeshSectionStorage* section{};
    NativeMaterialStorage* material{};

    // Descriptive observations of completed native calls, not replacement
    // mesh membership or ownership state. Reentrant calls still read originals.
    bool shadow_branch{};
    bool section_appended{};
    bool material_published{};
    void* last_release_identity{};
    GuiTextSectionReleasePhase release_phase{GuiTextSectionReleasePhase::not_called};

private:
    friend void ensure_gui_text_draw_sections_00ab8530(GuiWidgetOwner&, GuiTextWidget&,
        NativeNodeBinding*&, GuiTextBufferServices&, GuiTextSectionOperation&);
    GuiWidgetOwner* widget_{};
    GuiTextWidget* text_{};
    NativeNodeBinding** shadow_{};
    GuiTextBufferServices* services_{};
};

// AB8530..AB87CB: ECX same Text, no stack arguments, RET. Complete normal
// sequence and caller-owned acquisition tracking. Existing shadow skips its
// creation; absent main geometry stays absent. Retain current field reads and
// native section/material/mesh release order. FH3 cleans temporary strings
// and raw unconstructed slots only; completed creators persist on exception.
// Factory-internal host metadata cleanup and unexposed partial acquisitions
// remain provider boundaries, not a recovered native whole-callee unwind.
// A complete frame may serve a later call on the SAME identities. A failed or
// active frame is never restarted. New MSVC Win32 C++ ABI, not a native entry.
void ensure_gui_text_draw_sections_00ab8530(GuiWidgetOwner&, GuiTextWidget&,
    NativeNodeBinding*& shadow_188, GuiTextBufferServices&, GuiTextSectionOperation&);
} // namespace bsp
