#pragma once
#include <cstdint>

namespace bsp {
struct NativeGuiIconStateRepeatStorageFrame;
struct NativeGuiIconStorageContext;

// NORMAL AB56C0..AB56F5,54B/23ops. Native has entryECX and three stacked
// DWORD homes [destination,count,prototype], RET0Ch; this ordinary six-argument
// interface explicitly borrows real entryECX bits/tag HOME/four callee homes
// and the EXISTING retained AB5400 Frame/context, never a register hook/default.
//
// Full tagHOME=entryECX; capture outerHOME2, then HOME1 count, then HOME0 dest.
// Clear ONLY tag LOWBYTE, read its FULL DWORD, store calleeHOME3. Independently
// reread outerHOME2; calleeHOME2 gets FIRST prototype, HOME1 ORIGINAL entryECX,
// HOME0 SECOND prototype. Call actual AB5400 with captured destination/count
// and SAME mutable four-home array/frame/context. It rereads CURRENT callee
// HOME0 on EACH reached nonnull iteration, not a frozen prototype value.
//
// After NORMAL return, return capturedDestination+(capturedCount<<6), DWORD
// wrap, even count0. No callee/outer-home/count reread or outer copyback changes
// that result. EntryECX/tag upper bits and full count are not booleans/enums or
// narrowed policy inputs. Original POP-ECX/private stack/FLAGS/fault ABI remain
// excluded; no numeric result is ownership/birth/retirement/Origin authority.
//
// Tag/home backing is genuine live ordinary C++ operand storage, with legal
// aliases and genuine callback writes at reached accesses. Only private Source
// parameters/controls/spills are separated from operand/record/provider-write
// extents; no blanket no-write rule blocks the callee's declared live homes.
// Distinct active calls need distinct retained frame/home/control backing.
// Inherit the existing AB5400 normal individual no-write State birth and actual
// AB3A70 distinct-record/rawpool/string/canonical texture+04 domains unchanged;
// no T[n], exactBF0/overlap upgrade, new Frame/provider/allocator/owner exists.
//
// Count0 still stages every wrapper operand/home before the genuine zero-count
// call. UNKNOWN escape retains partial/completed/remaining graph and all homes,
// tag/frame/providers. AB5400's13Nativecatch rows/prefix cleanup remain HELD;
// no catch/refund/retry/rollback/noexcept/terminal/cleanup or Native Origin is
// supplied. Existing HIGH STL/vendor names/tags and prior credit remain; zero
// new Native/Original/vendor/helper/CG/duplicate-body credit. Full growth,
// Original ABI/FH3/SEH/hardwareFP/caller/custody/runtime/activation are unproved.
std::uint32_t repeat_native_gui_icon_states_wrapper_storage_00ab56c0(
    std::uint32_t actual_entry_ecx_bits,
    const volatile std::uint32_t (&actual_outer_stack_homes)[3],
    std::uint32_t& actual_tag_home,
    volatile std::uint32_t (&actual_callee_stack_homes)[4],
    NativeGuiIconStateRepeatStorageFrame& frame,
    NativeGuiIconStorageContext& context);
} // namespace bsp
