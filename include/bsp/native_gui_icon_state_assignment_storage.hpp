#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
// Whole raw AB3C10..AB3CA9 assignment schedule,154B/61ops/8x87 forms.
// Native ECX=destination, one stacked sourceDWORD, EAX=captured destination,
// RET4 become this ordinary Source ABI. Actual40B records/raw8B name headers
// and their ordinary C++ backing/subobjects must remain genuinely live.
// No typed GuiIconState/std::string/owner projection or new state birth exists.
//
// Copy sourceTexture0 with ZERO retain/release. Compare name-header addresses
// before that store. Self skips only the string calls and preserve-home store;
// all eight DWORDs/four x87 FLD-FSTP pairs/two BYTE tail stores still execute.
// Nonself captures CURRENTsrcLEN then writes full preserveHOME1. Borrow a real
// ordinary caller-retained DWORD, not volatile backing cast through raw helpers;
// keep its unseeded preimage on self. The reached write uses a volatile view of
// that SAME object. Exact resize reads its BYTE after allocation when reached.
//
// strings is the SAME genuine RawPool context. feature is the SAME stable live
// canonical0109EEA4 cell, borrowed without copying its value or making a bank.
// After real resize, CURRENTsrcLEN gates outer BF; read CURRENTdstLEN, srcDATA,
// dstDATA in that order and call genuine BF even for count0, with no+1/repair.
// Valid record/header/payload aliases preserve actual read/store order; inherit
// helper extent/DF0/SSE2/feature/pool/CRT/lifetime and escape qualifications.
// Private Source control/spills and the preserve-home backing are separate
// from those data/provider-write extents and active call/return/provider frames.
//
// No C++float, FPreset or callback while relativeST0is live. Self still reaches
// hardware FP effects. No local catch, cleanup/refund/retry/rollback, NoThrow,
// terminal or Native-Origin permission is supplied. Retain the actual graph,
// homes/backing/providers and incomplete pool/header/allocation debt on UNKNOWN
// escape; a recoverable-pointer or ownership receipt is not fabricated.
// Original private-stack/register/FLAGS/fault/FH3/SEH/unmaskedFP/caller/runtime/
// custody/activation remain qualified. Old semantic body and all prior credit
// remain unchanged; new Original/vendor/helper/CG/duplicate-body credit is zero.
void* assign_native_gui_icon_state_storage_00ab3c10(
    void* actual_destination, const void* actual_source,
    std::uint32_t& actual_preserve_home, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
