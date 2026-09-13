#pragma once
#include "bsp/native_gui_text_model_clone.hpp"

namespace bsp {
class GuiWidgetOwner;
class GuiWidgetOwnerRuntime;

// Host lifetime borrow, not a native retain or a second widget. A complete
// constructor operation can hold this across pool callbacks and pending
// derived work. Only its matching AA9520 call may cross the source preflight.
// The source must outlive this token; owner retirement is rejected while held.
class GuiWidgetCopySourceBorrow final {
public:
    explicit GuiWidgetCopySourceBorrow(GuiWidgetOwner&);
    ~GuiWidgetCopySourceBorrow() noexcept;
    GuiWidgetCopySourceBorrow(const GuiWidgetCopySourceBorrow&) = delete;
    GuiWidgetCopySourceBorrow& operator=(const GuiWidgetCopySourceBorrow&) = delete;
    GuiWidgetOwner& source() const noexcept { return source_; }
private:
    friend class GuiWidgetOwnerRuntime;
    GuiWidgetOwner& source_;
};

// Explicit allocation preimages for represented fields AA9520 does not write.
// Destination layout.transform.authored_x(+08) and visible(+E4) stay in place.
// Bits, rather than float arguments, preserve allocation FP payloads.
struct GuiWidgetBaseCopyPreimage {
    std::uint32_t fields_7c_80_bits[2];
    bool active_85;
    std::uint32_t clip_enabled_e8_bits;
};

class GuiWidgetModelCopyCalls {
public:
    virtual ~GuiWidgetModelCopyCalls() = default;
    // AA96FC: source Model current10, ECX=source, stack flags,parent, RET8.
    // Complete the actual B752B0 operation using this source's SAME Model,
    // geometry/material/pool domains. Return the canonical reference owning
    // ONE creator, also published in acquired.model, or an actual null return.
    // Success consumes all temporary mesh/section/material creators. Throws
    // preserve acquired references and native effects; this call cannot retry.
    // Text3 uses flags3Eh, not the flags26h-only AB98F0 Model clone helper.
    virtual NativeModelReference* clone_current10(NativeModelOwner& source,
        std::uint32_t flags, NativeNodeBinding* parent,
        NativeGuiTextModelCloneAcquired& acquired) = 0;
};
struct GuiWidgetCopyServices {
    // Borrow the actual current DWORD table, not names or a copied table.
    // Native entries0..16 are3Eh; entries17/18 are26h. Text3 uses D5C0C4.
    const volatile std::uint32_t (&clone_flags_00d5c0b8)[19];
    GuiWidgetModelCopyCalls& models;
};
} // namespace bsp
