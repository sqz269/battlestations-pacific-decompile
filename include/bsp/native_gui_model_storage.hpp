#pragma once
#include "bsp/native_gui_widget_copy_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// Actual GUI class9 EC..11B tail, NOT the renderer's184h Model payload.
struct NativeGuiModelTailStorage {
    void* root_ec;    // borrowed actual root from generated wrapperF0+0C
    void* wrapper_f0; // actual ref-counted generated resource wrapper
    float scale_f4[3];
    float rotation_100[3];
    std::uint32_t model_name_length_10c;
    char* model_name_data_110;
    std::uint32_t texture_override_length_114;
    char* texture_override_data_118;
};
NativeGuiModelTailStorage& native_gui_model_tail(void* actual_widget);

using NativeGuiModelInterlocked = long (__stdcall*)(volatile long*);
struct NativeGuiModelStorageContext {
    NativeGuiWidgetLifetimeContext& lifetime;
    NativeStringRawPoolContext& strings;
    NativeRenderActualOwnerRegistry& actual_owners; // SAME wrapper/widget +04 companions
    NativeGuiModelInterlocked const volatile& decrement_00ce2220;
};
// Mandatory genuine raw ACCCF0 on the SAME widget. It must use actual resource
// default-factory/current+08(6,1.0) wrapperF0/rootEC, same raw Model4C parenting,
// raw transform/material/texture/clip services and canonical resource owners.
// Inputs are the widget's SAME actual8h headers, not std::string copies.
// No empty loader, synthetic wrapper or projected GuiWidgetOwner is supplied.
// The provider/context retains all partial-load obligations on interruption;
// native copy unwind does not release a newly published wrapperF0.
class NativeGuiModelLoadCalls {
public:
    virtual ~NativeGuiModelLoadCalls() = default;
    virtual void load_model_00acccf0(void* actual_widget,
        const void* own_model_name_header_10c,
        const void* own_texture_override_header_114) = 0;
};

// External diagnostics, never another raw/native resource owner or count.
// Entered/failed records and SAME raw slot/providers must remain retained;
// no retry, pool return, reuse or dependency drain after interrupted work.
struct NativeGuiModelStorageAcquired {
    enum class Phase { fresh, entered, complete, failed };
    enum class Entry { copy, destroy, scalar };
    Phase phase{Phase::fresh};
    Entry entry{Entry::copy};
    void* actual{};
    const void* source{};
    std::uint32_t active_call_site{}, failed_call_site{};
    std::int32_t native_eh_state{-1}, failed_eh_state{-1};
    void* captured_wrapper{};
    NativeGuiModelInterlocked captured_decrement{};
    long wrapper_release_result{};
    bool wrapper_decremented{}, wrapper_zero_entered{}, wrapper_zero_returned{};
    bool cleanup_entered[3]{}; // state0 base,1 name10C,2 override114
    bool slot_returned{};
    NativeGuiModelStorageAcquired() = default;
    ~NativeGuiModelStorageAcquired() noexcept;
    NativeGuiModelStorageAcquired(const NativeGuiModelStorageAcquired&) = delete;
    NativeGuiModelStorageAcquired& operator=(const NativeGuiModelStorageAcquired&) = delete;
};
struct NativeGuiModelCopyContext {
    NativeGuiWidgetCopyContext& base;
    NativeGuiModelStorageContext& model;
    NativeGuiModelLoadCalls& load;
};
struct NativeGuiModelCopyAcquired {
    NativeGuiWidgetCopyAcquired base;
    NativeGuiModelStorageAcquired operation; // dies before base's acquired frame
    bool base_returned{}, name_returned{}, override_returned{};
    bool load_entered{}, load_returned{};
};

// ACC740 ECX actual11Ch/EAX same/RET: real AA9390(type9), EC/F0=0, D5CF38,
// strings10C/110/114/118=0. SIX float wordsF4..108 remain UNWRITTEN, as do base
// unwritten bytes and the separate pool trailer11C. No substitute transform.
void* construct_native_gui_model_default_00acc740(void* actual_widget,
    const volatile std::uint32_t& actual_one_00d7a24c);
// ACCF70 placement copy: real AA9520, EC/F0=0, six native x87 float copies,
// real pooled string10C then114 copies, mandatory same-widget ACCCF0 last.
// Exact C++ cleanup states2/1/0:114,10C,base; no invented wrapper cleanup.
// Failed raw slot/loader obligations stay with caller's sole existing owner.
// Native register/FH3/SEH/fault ABI is not supplied by this source interface.
void* construct_native_gui_model_copy_00accf70(void* actual_destination,
    const void* actual_source, NativeGuiModelCopyContext&, NativeGuiModelCopyAcquired&);
// CLOSED default AA16E0 arm only: SAME initialized F8C09C11Ch/120h process
// pool, ACC490/ACC350 allocate, ACC1C0 failed-constructor slot return.
// No copy factory: its original unconditional EH slot return cannot qualify
// an arbitrary partial ACCCF0 provider's resource/parameter lifetime. This is
// not retained-factory admission or permission to activate a native Page.
void* create_native_gui_model_default_00aa16e0(const volatile std::uint32_t& actual_one);
// ACC7B0 stamps D5CF38, captured wrapperF0+04/current import/current zero,
// clearF0 only after return, strings114 then10C, genuine AA9730 last. EC is
// NOT cleared. Exact C++ unwind states2/1/0; second cleanup throw terminates.
void destroy_native_gui_model_00acc7b0(void* actual_widget,
    NativeGuiModelStorageContext&, NativeGuiModelStorageAcquired&);
// ACC880 direct scalar, no reference decrement/count gate. Read CURRENT flags
// low byte after dtor; same ACC050 pool return iff bit0, none on interruption.
void* delete_native_gui_model_00acc880(void* actual_widget,
    const volatile std::uint32_t& actual_flags_slot,
    NativeGuiModelStorageContext&, NativeGuiModelStorageAcquired&);
} // namespace bsp
