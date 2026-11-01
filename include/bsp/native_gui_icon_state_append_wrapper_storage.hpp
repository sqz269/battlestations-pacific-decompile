#pragma once
#include "bsp/native_gui_icon_state_iterator_insert_storage.hpp"

namespace bsp {
// Caller-retained ordinary homes/actual callee frames. Empty user-provided
// construction leaves all Native preimages unseeded. local_08 is the SAME8B
// backing for fast-path tagWORD and slow-path output iterator; its firstWORD
// needs genuine initialized upper bytes before the lowBYTE write/full read.
struct NativeGuiIconStateAppendWrapperStorageFrame {
    NativeGuiIconStateAppendWrapperStorageFrame() {}
    NativeGuiIconStateAppendWrapperStorageFrame(const NativeGuiIconStateAppendWrapperStorageFrame&) = delete;
    NativeGuiIconStateAppendWrapperStorageFrame& operator=(const NativeGuiIconStateAppendWrapperStorageFrame&) = delete;
    NativeGuiIconStateAppendWrapperStorageFrame(NativeGuiIconStateAppendWrapperStorageFrame&&) = delete;
    NativeGuiIconStateAppendWrapperStorageFrame& operator=(NativeGuiIconStateAppendWrapperStorageFrame&&) = delete;
    std::uint32_t local_08[2];
    std::uint32_t repeat_homes[4];
    NativeGuiIconStateRepeatStorageFrame repeat_frame;
    std::uint32_t iterator_homes[4], growth_homes[4];
    NativeGuiIconStateGrowthStorageFrame growth_frame;
};

// Whole131B/53op AB63A0 ordinary Source schedule. Original ECX vector plus
// one actual incoming prototype DWORD HOME/RET4 becomes this four-argument API.
// Preserve captured first and unsigned wrapped SUB/SAR6 size/capacity decisions.
// Spare capacity: read prototype HOME TWICE, capture CURRENTlast, clear ONLY
// scratch0 LOWBYTE, read FULLscratch0, then stage tag/firstproto/vector/secondproto
// to real5400. After normalreturn publish capturedlast+40h, not a fresh last.
// Slow path: capture CURRENTlast; real returning BF6713 if capturedfirst>last;
// only then read CURRENT prototype HOME and stage genuine6070's four homes.
// Its output pointer is SAME local_08 backing. Ignore its result as Native does.
//
// No new Native owner/callback/allocator/lookup, default state, input cap,
// count/profile filter, cleanup/retry/rollback/NoThrow or Origin is introduced.
// Real current SDK handler and lower5400/59E0 policies remain inherited;
// actual6070 is a separately pinned ignored dependency, not published/compiled
// by this packet. Source91 parent cleanup is separately qualified at binding.
// Caller proves actual vector/state/prototype/backing/texture/pool/feature and
// provider custody. All callee home arrays/frames and scratch are real stable
// distinct objects, with local_08's stated tag/output alias; declared callee
// operand/temp writes remain live under those suppliers. Private Source
// controls/spills stay disjoint from operand/provider writes. Keep all entered
// backing/partial/current/completed graph/provider debt retained on escape;
// no implicit frame destruction may stand in for actual Native cleanup.
// No T[n] array family, Native ABI/private-stack/FLAGS/fault/FH3/Origin/FP,
// actual caller/production/disposition/runtime/activation or extra Original/
// vendor/helper/CG/duplicate credit follows. Existing STL name/tag stays.
void append_native_gui_icon_state_storage_00ab63a0(void* actual_vector_f0,
    const volatile std::uint32_t& actual_prototype_home,
    NativeGuiIconStateAppendWrapperStorageFrame&, NativeGuiIconStateGrowthStorageContext&);
} // namespace bsp
