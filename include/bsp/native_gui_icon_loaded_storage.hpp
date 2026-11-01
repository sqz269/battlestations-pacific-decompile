#pragma once
#include "bsp/native_gui_icon_storage.hpp"
#include <cstdint>
#include <exception>

namespace bsp {
struct NativeGuiIconRebuildStorageContext;
struct NativeGuiIconRebuildStorageFrame;
class NativeGuiWidgetLoadedDispatch;
struct GuiWidgetBounds;
class NativeGuiIconLoadedStorageContext;
class NativeGuiIconLoadedStorageFrame;
void native_gui_icon_loaded_storage_00ab10f0(void*,
    NativeGuiIconLoadedStorageFrame&, NativeGuiIconLoadedStorageContext&);

enum class NativeGuiIconSourceCallPhase { fresh, entered, complete, failed };

// One caller-retained Source invocation of the existing CURRENT80 interface.
// Binding is address metadata only. The constructor neither reads the HOME or
// receiver nor constructs/destroys the borrowed Raw80 graph. Genuine caller
// creation, C++ subobject family and custody prove the default class; matching
// receiver/profile/target words are negative affinity checks, never authority.
class NativeGuiIconDefaultCurrent80Calls final : public NativeGuiIconCurrentCalls {
public:
    NativeGuiIconDefaultCurrent80Calls(void* actual_receiver,
        std::uint32_t& actual_state_argument_home,
        NativeGuiIconRebuildStorageFrame&, NativeGuiIconRebuildStorageContext&);
    NativeGuiIconDefaultCurrent80Calls(const NativeGuiIconDefaultCurrent80Calls&) = delete;
    NativeGuiIconDefaultCurrent80Calls& operator=(const NativeGuiIconDefaultCurrent80Calls&) = delete;
    NativeGuiIconDefaultCurrent80Calls(NativeGuiIconDefaultCurrent80Calls&&) = delete;
    NativeGuiIconDefaultCurrent80Calls& operator=(NativeGuiIconDefaultCurrent80Calls&&) = delete;
    void call_rebuild_current80(void* actual_icon, std::uint32_t captured_target,
        std::uint32_t full_state_word) override;
    NativeGuiIconSourceCallPhase source_phase() const { return phase_; }
    bool source_body_returned() const { return body_returned_; }
private:
    friend void native_gui_icon_loaded_storage_00ab10f0(void*,
        NativeGuiIconLoadedStorageFrame&, NativeGuiIconLoadedStorageContext&);
    void* const bound_receiver_;
    std::uint32_t& state_home_;
    NativeGuiIconRebuildStorageFrame& rebuild_frame_;
    NativeGuiIconRebuildStorageContext& rebuild_context_;
    NativeGuiIconSourceCallPhase phase_{NativeGuiIconSourceCallPhase::fresh};
    bool invalidated_{};
    bool body_returned_{};
    void* entered_receiver_{};
    std::uint32_t captured_target_{};
    std::uint32_t captured_state_word_{};
    std::exception_ptr failure_;
};

// Cold address-only preparation from the SAME Raw80 context and genuine mapped
// RO service. data_at may reject absent spans; no cell value is sampled. one
// and half must be the authentic D7A24C/D7A280 addresses in that live owner.
// Existing AA7170 current60 dispatch is real, including any reached OTHER arm;
// no no-op/dummy service is supplied here. Keep this context, dispatch, bounds,
// constants and all Raw80 services/backing through the entered invocation.
class NativeGuiIconLoadedStorageContext final {
public:
    NativeGuiIconLoadedStorageContext(NativeGuiIconRebuildStorageContext&,
        NativeGuiWidgetLoadedDispatch&, GuiWidgetBounds& retained_bounds_scratch,
        const volatile double& actual_half_00d7a280);
    NativeGuiIconLoadedStorageContext(const NativeGuiIconLoadedStorageContext&) = delete;
    NativeGuiIconLoadedStorageContext& operator=(const NativeGuiIconLoadedStorageContext&) = delete;
    NativeGuiIconLoadedStorageContext(NativeGuiIconLoadedStorageContext&&) = delete;
    NativeGuiIconLoadedStorageContext& operator=(NativeGuiIconLoadedStorageContext&&) = delete;
private:
    friend class NativeGuiIconLoadedStorageFrame;
    friend void native_gui_icon_loaded_storage_00ab10f0(void*,
        NativeGuiIconLoadedStorageFrame&, NativeGuiIconLoadedStorageContext&);
    NativeGuiIconRebuildStorageContext& rebuild_;
    NativeGuiWidgetLoadedDispatch& active_;
    GuiWidgetBounds& bounds_;
    const volatile double& half_;
    const volatile std::uint32_t* const default_profile_;
};

// Retained Source backing for AB10F0's three outgoing DWORD homes. This user-
// provided constructor leaves native_argument_homes_ uninitialized: first
// reached writes are ratioBits, mode0, index0 after AA7170's actual return.
// Raw80 state HOME/frame are separately retained borrowed objects, not members
// destroyed on this frame's unwind. Neither source phase nor body return is
// Native Origin, cleanup/disposition permission or physical ownership proof.
class NativeGuiIconLoadedStorageFrame final {
public:
    NativeGuiIconLoadedStorageFrame(void* actual_receiver,
        NativeGuiIconLoadedStorageContext&, std::uint32_t& actual_state_argument_home,
        NativeGuiIconRebuildStorageFrame&);
    NativeGuiIconLoadedStorageFrame(const NativeGuiIconLoadedStorageFrame&) = delete;
    NativeGuiIconLoadedStorageFrame& operator=(const NativeGuiIconLoadedStorageFrame&) = delete;
    NativeGuiIconLoadedStorageFrame(NativeGuiIconLoadedStorageFrame&&) = delete;
    NativeGuiIconLoadedStorageFrame& operator=(NativeGuiIconLoadedStorageFrame&&) = delete;
    NativeGuiIconSourceCallPhase source_phase() const { return phase_; }
    bool source_base_returned() const { return base_returned_; }
    bool source_selection_returned() const { return selection_returned_; }
private:
    friend void native_gui_icon_loaded_storage_00ab10f0(void*,
        NativeGuiIconLoadedStorageFrame&, NativeGuiIconLoadedStorageContext&);
    void* const bound_receiver_;
    NativeGuiIconLoadedStorageContext& context_;
    std::uint32_t native_argument_homes_[3]; // Ordinary backing, no DMI.
    NativeGuiIconDefaultCurrent80Calls current_;
    NativeGuiIconSourceCallPhase phase_{NativeGuiIconSourceCallPhase::fresh};
    bool invalidated_{};
    bool base_returned_{};
    bool selection_returned_{};
    void* entered_receiver_{};
    NativeGuiIconLoadedStorageContext* entered_context_{};
    std::uint32_t captured_profile_{};
    std::uint32_t captured_target_{};
    std::exception_ptr failure_;
};

// Whole32B/14-op AB10F0 NORMAL default-domain composition: genuine AA7170,
// FLD1 -> CURRENT profile/+88 TARGET VALUE -> actual ratio HOME FSTP -> mode0
// -> index0 -> real AB1710, using the concrete genuine default80 adapter.
// No CPP call occurs with this function's x87 value live. A foreign profile is
// popped then rejected outside asm, never read through the default table.
// Unsupported target is rejected only after reached home staging; no fallback.
//
// Caller proves genuine default D5C4C0 Icon birth/current C++ subobjects, Model
// generation and all Raw80/canonical-reference/provider custody. This library
// creates no production preparer/owner or Page/current18 transport. Retain fresh
// frame/context/actual HOME/Raw80 graph outside unwind; keep Source controls,
// argument homes and borrowed writable extents disjoint except declared callee
// operand aliases. Different fresh records inherit genuine provider lifetime/
// serialization contracts; same-record replay/reentry is sticky failure.
// UNKNOWN records/rethrows without reset, cleanup, retry, count repair, new
// terminal policy or Native permission. Unused80 on a fast1710 return is merely
// unused Source metadata. Extra guards/carriers/private homes change Source
// ABI/FLAGS/fault schedule; Original ABI/FH3/SEH/unmaskedFP/runtime are unproved.
void native_gui_icon_loaded_storage_00ab10f0(void* actual_icon,
    NativeGuiIconLoadedStorageFrame&, NativeGuiIconLoadedStorageContext&);
} // namespace bsp
