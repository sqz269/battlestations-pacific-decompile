#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual front-end Frame lifetime composition requires MSVC Win32.
#endif

namespace bsp {
struct NativePlatformFocusOwnersContext;
struct NativeGuiPageLifetimeHost;

// Pure borrows from the SAME retained application/GUI/Page authority. The GUI
// owners context supplies the real manager/GUI publications and real resources;
// page_lifetime is the SAME required current-virtual20/04 host used by the GUI
// manager. No copied manager, typed Page/Frame, new bank or ownership is added.
// actual_screen_words identifies exactly95 writable DWORD cells E18B60..CDB;
// E18CDC is a separate sentinel and is never read or written by these bodies.
struct NativeFrontEndFrameLifetimeContext {
    NativePlatformFocusOwnersContext& gui_owners;
    NativeGuiPageLifetimeHost& page_lifetime;
    void* volatile& actual_frame_publication_00e18d80;
    volatile std::uint32_t* const actual_screen_words_00e18b60;
};

// Whole00517D90..00517EAB [284 physical B /83 decoded ops, including6B of
// jumped-over padding]. Native ECX captured actual164h Frame, plain RET.
// Stamp primaryCEC39C then secondary+8CEC398. Traverse13*5,2*5,2*5 slots in
// native column-major order. For each nonnull SLOT test: WHOLE real4C12B0,
// THEN reload that SAME SLOT and call WHOLE actualAA31F0 with the retained
// Page host. A getter-mutated null/replacement is passed unchanged; no second
// null/count/profile gate, cached Page, per-slot clear or reference decrement.
//
// Normal tail unconditionally clears SAME E18D80 even if it holds a replacement,
// restores original Frame+8=CE3818, then composes whole4F71A0: primary=CEAE54,
// scan all95 current cells and clear EVERY exact captured receiver match.
// Native FH3 state1->0->-1 on a traversal exception performs that SAME base
// cleanup once, then the ordinary C++ exception propagates. No traversal replay,
// catch around the disarmed normal base tail, or new termination policy.
void destroy_native_front_end_frame_00517d90(void* captured_actual_frame,
    const NativeFrontEndFrameLifetimeContext&);

// Whole00517EB0..00517ECD [30B/11ops]. Native ECX actual Frame, stacked full
// DWORD flags, RET4/EAX captured allocation identity. Call the whole destructor
// first; ONLY after normal return read byte0 of the SAME live DWORD home.
// If bit0 is set, call existing realBF65AC free on the CAPTURED allocation.
// Never read receiver bytes after free; the return is address identity only.
void* delete_native_front_end_frame_00517eb0(void* actual_frame,
    volatile std::uint32_t& actual_flags_dword_home,
    const NativeFrontEndFrameLifetimeContext&);

// Whole00517D50..00517D57 [8B/2ops]: SUB ECX,8; JMP00517EB0. Subtract from
// captured actual secondary receiver with DWORD wrap; forward the SAME flags
// HOME and context. This is not base-construction-profile scalar00517D60.
void* delete_native_front_end_frame_secondary_00517d50(void* actual_secondary,
    volatile std::uint32_t& actual_flags_dword_home,
    const NativeFrontEndFrameLifetimeContext&);

// New ordinary Source interfaces, not physical thiscall/tail-JMP/FH3/SEH ABIs.
// Caller proves accessible aligned actual backing, all85 valid native slot
// representations, same-domain Page/manager graph and full operation lifetime.
// Constructor517D10 initializes only the prefix; it does not establish these
// slot obligations. No initialized-count/range policy or zero reseeding is added.
// Captured Frame may differ from current publication. Context/cells/providers,
// all reached backing and the flag home must remain alive through their actual
// read/write epochs; no native retain or automatic destructor owns that duty.
// A partial entered failure preserves completed effects and unresolved graph;
// retain its real domain for explicit disposition, never automatically replay.
//
// Future dispatch must bind actual CEC39C+0C ->517EB0 and secondaryCEC398+00
// ->517D50 over the SAME live allocation/contexts. BaseCEC388/517D60 remains
// separate. No application, singleton-map, global-view, CMake or activation
// binding is installed. CG wrappers remain compiler_generated_out_of_scope;
// zero additional Original/vendor/helper/CG/duplicate-body credit. Original ABI,
// invalid storage/faults, exact private aliases, FH3/SEH, game/runtime are unproved.
} // namespace bsp
