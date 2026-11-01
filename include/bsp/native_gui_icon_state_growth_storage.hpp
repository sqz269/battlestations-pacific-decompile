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

// AB59E0 normal payload plus ordinary Source C++ cleanup schedule. The633B/
// 243physical Native instructions include25 landing rows; Source catches below
// model their reached cleanup effects, not binary/private-FH3 exception identity. Actual raw16B vector [proxy,first,last,end],
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
// alive until actual specified deallocation. Published lower5400/4C50 catches
// clean their own completed prefix; this parent never replays those calls.
// Completed-temp cleanup starts only after whole AB3A70 normal return. An outer
// Source catch then destroys SAME temp once and barethrows only after that
// destructor returns. The normal footer stores state-1 BEFORE temp destruction
// and lies outside this catch. No failed-temp/current-slot dtor is invented.
// Narrow catch1 covers only new prefix/repeat/suffix construction: CURRENT
// HOME2 then HOME3 then receiver14; real57 on captured begin/end; free captured
// HOME3 identity only after57 normal return; barethrow only after free returns.
// Narrow catch3 covers tail<count repeat: CURRENT receiver14/HOME2/currentlast,
// wrapped countbytes/end, THEN late HOME1/begin; real57, no free, barethrow.
// A range-cleanup escape skips later free/rethrow and reaches the outer temp
// cleanup once. A temp-cleanup escape propagates without replay; existing raw
// AB3280 name-unwind/secondary-terminate Source policy remains inherited. This
// ordinary C++ replacement/propagation is not Native FH3/double-fault identity.
// Every late cleanup interval/free identity needs genuine actual state and
// allocation custody. Enclosing real free ends trivial C++ record lifetimes;
// it does NOT prove logical texture/name retirement or discharge external debt.
// Afterwards retain numeric history plus genuinely independent provider debt,
// NEVER raw-pointee rescan/replay. Passive state/return/progress grants no such
// custody or Native Origin/cleanup/retirement permission. No new owner, cause,
// cursor, phase authority, retry/rollback/NoThrow or terminal wrapper is added.
// Original private-stack/register/FLAGS/fault/FH3/SEH/hardwareFP and actual
// caller/Page/current18/current78/current80/disposition/activation remain held.
// Preserve existing STL/vendor names/tags; zero new Original/vendor/helper/CG/
// duplicate-body credit. Source ABI/support does not prove compiler/runtime.
void insert_native_gui_icon_states_storage_00ab59e0(void* actual_vector_f0,
    volatile std::uint32_t (&actual_stack_homes)[4],
    NativeGuiIconStateGrowthStorageFrame&, NativeGuiIconStateGrowthStorageContext&);
} // namespace bsp
