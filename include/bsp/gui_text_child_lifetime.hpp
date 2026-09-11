#pragma once
#include "bsp/gui_text_lifetime.hpp"
#include <list>
#include <stdexcept>

namespace bsp {
// Outside the currently reconstructed domain. The original flags and precise
// outer scalar stage remain on the SAME companion. This is not a resumable
// destructor: nested AB80C0 also requires a retained native parent-loop frame.
class GuiTextDeletionBoundary final : public std::logic_error {
public:
    using std::logic_error::logic_error;
};

// Concrete C++ allocation transport and bounded00AB8EE0 semantic deletion.
// Uses the one widget runtime/tree and each owner's exact borrowed companion.
// No Text factory or substitute virtual implementation is created.
//
// Domain: live canonical Text children, empty native timed-entry header+88/8C/90,
// valid stable child collections across scene callbacks, and no destructive
// reentry. Non-Text or unbound remaining children and nonzero entry headers
// throw GuiTextDeletionBoundary AT the missing native phase, retaining owner,
// companion and any transferred wrapper. Never treat that exception as success.
// Native Text slot pool00AB75A0, native CRT/SEH and raw vtable writes remain out
// of this new C++ ABI for BOTH flags0 and flags1.
class GuiTextChildDeletion final : public GuiTextGlyphChildCalls {
public:
    GuiTextChildDeletion(GuiWidgetOwnerRuntime&, NativeNodeParentingRuntime&);
    ~GuiTextChildDeletion() noexcept override;
    GuiTextChildDeletion(const GuiTextChildDeletion&) = delete;
    GuiTextChildDeletion& operator=(const GuiTextChildDeletion&) = delete;

    // Same removed unique_ptr, no native-visible call or state change.
    void accept_detached_child(std::unique_ptr<GuiLayoutWidget>) noexcept override;
    // AB8250 derived -> AA9730 base -> flags&1 storage release. The zero flag
    // performs the SAME semantic teardown, including base list allocations,
    // then preserves only the wrapper allocation for explicit later disposal.
    // flags1 requires transferred ownership or an existing owning parent entry.
    void delete_text_child_virtual4(GuiLayoutWidget&, std::uint32_t flags) override;

    // Transfer an ordinary live detached handle for reattachment, or completed
    // flags0 storage for disposal. Pending destruction cannot be taken/dropped.
    std::unique_ptr<GuiLayoutWidget> take_detached_storage(GuiLayoutWidget&);
    std::size_t retained_storage_count() const noexcept { return detached_.size(); }
private:
    using Handles = std::list<std::unique_ptr<GuiLayoutWidget>>;
    Handles::iterator find_storage(GuiLayoutWidget&) noexcept;
    void release_primary(GuiWidgetOwner&);
    GuiWidgetOwnerRuntime& owners_;
    NativeNodeParentingRuntime& parenting_;
    // Allocation handles only, never a second widget/companion lookup map/tree.
    // All live/pending handles must be completed or explicitly reattached before
    // destroying this transport. Its destructor terminates on such misuse rather
    // than silently invoking page retirement on unfinished native destruction.
    Handles detached_;
};
} // namespace bsp
