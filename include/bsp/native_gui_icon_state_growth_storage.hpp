#pragma once
#include "bsp/native_gui_icon_storage.hpp"
#include "bsp/native_gui_icon_state_range_construction_storage.hpp"
#include "bsp/native_gui_icon_state_repeat_storage.hpp"

namespace bsp {
struct NativeGuiIconStateGrowthStorageContext {
    NativeGuiIconStorageContext& storage;
    const volatile std::uint32_t& feature_word_0109eea4;
};

// Retained ordinary operand storage, not a Native EBP/FS/stack replica. The
// user-provided empty constructor prevents value-initialization from zeroing
// the native preimages; no field/home initializer or Native action occurs.
// In particular scratch_18's upper bytes need a genuine initialized preimage.
// Native local aliases are preserved: scratch_18 is tag then saved old-last;
// incoming HOME2 count becomes allocation/progress, HOME3 source becomes new
// backing or bytecount. Declared operand homes may receive genuine callbacks.
// Private Source controls/spills remain separate from operand/provider writes.
struct NativeGuiIconStateGrowthStorageFrame {
    NativeGuiIconStateGrowthStorageFrame() {}
    NativeGuiIconStateGrowthStorageFrame(const NativeGuiIconStateGrowthStorageFrame&) = delete;
    NativeGuiIconStateGrowthStorageFrame& operator=(const NativeGuiIconStateGrowthStorageFrame&) = delete;
    NativeGuiIconStateGrowthStorageFrame(NativeGuiIconStateGrowthStorageFrame&&) = delete;
    NativeGuiIconStateGrowthStorageFrame& operator=(NativeGuiIconStateGrowthStorageFrame&&) = delete;
    std::uint32_t receiver_14, scratch_18, state_04;
    NativeGuiIconStateStorage temporary_58;
    std::uint32_t direct_range_homes[4];
    NativeGuiIconStateRangeConstructionStorageFrame direct_range_frame;
    std::uint32_t repeat_outer_homes[3], repeat_tag_home, repeat_callee_homes[4];
    NativeGuiIconStateRepeatStorageFrame repeat_frame;
    std::uint32_t range_outer_homes[3], range_tag_home, range_callee_homes[4];
    NativeGuiIconStateRangeConstructionStorageFrame wrapped_range_frame;
    std::uint32_t destroy_homes[2], backward_homes[3], fill_homes[3];
    std::uint32_t assignment_preserve_home;
};

// Whole NORMAL AB59E0 payload/control schedule:633B/243physical instructions,
// with25Native landing rows HELD. Actual raw16B vector [proxy,first,last,end],
// incoming four DWORD homes [unused,position,count,prototype]; Original ECX /
// RET10h become an ordinary four-argument void Source API. No defined EAX.
// Count0 STILL copies a real40B temp BEFORE count read and destroys it normally.
// All capacity/size differences use wrappedDWORD/SAR6 and unsigned decisions;
// Native max03FFFFFF/growth1.5 arithmetic is retained, not a new domain cap.
//
// Uses only genuine existing raw copy/dtor/allocation/range/repeat/assignment
// suppliers, SAME storage/rawpool/canonical live feature and ordinary preserve
// HOME. The actual legacy vector-length-error service owns its existing18-byte
// message/payload; Source CRT/FH3 policy is inherited, no Native ThrowInfo/Origin.
// Direct/wrapped callee homes and frames remain real distinct retained objects;
// sequential direct calls reuse their SAME backing without blanket resets.
// Source register captures remain distinct from live homes/current vector
// fields. All reached current reloads/output-home writes/publication order stay.
// No semantic vector, T[n] family, owner, callback, new allocator or fallback.
//
// Caller proves all individual state/subobject/backing/custody and provider /
// allocator/module/DF/CPU/FP lifetimes and genuine scratch preimages. UNKNOWN
// propagates with current/partial/completed/remaining graph/temporary/homes and
// any unpublished allocation, already-consumed or incomplete callee debt kept
// alive. Existing lower normal helpers'13landing rows are not reconstructed
// here; later cleanup suppliers must not be replayed or duplicated. This body
// has no catch/automatic Native cleanup/retry/rollback/NoThrow/terminal policy.
// Passive state is not cleanup/ownership/retirement or Native Origin authority.
// Original private-stack/register/FLAGS/fault/FH3/SEH/hardwareFP and actual
// caller/Page/current18/current78/current80/disposition/activation remain held.
// Preserve existing STL/vendor names/tags; zero new Original/vendor/helper/CG/
// duplicate-body credit. Source ABI/support does not prove compiler/runtime.
void insert_native_gui_icon_states_storage_00ab59e0(void* actual_vector_f0,
    volatile std::uint32_t (&actual_stack_homes)[4],
    NativeGuiIconStateGrowthStorageFrame&, NativeGuiIconStateGrowthStorageContext&);
} // namespace bsp
