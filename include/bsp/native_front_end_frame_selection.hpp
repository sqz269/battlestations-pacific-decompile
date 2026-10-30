#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual front-end Frame selection requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeGuiPageLifetimeHost;

// Required genuine operations on actual receivers. No semantic FrontEndScreen,
// GuiLayoutPage, supplied ID, copied name or numeric-codeword invocation.
// All contexts/table backing survive callbacks and old Frame generations.
struct NativeFrontEndFrameSelectionBindings {
    virtual ~NativeFrontEndFrameSelectionBindings() = default;
    // Whole raw4F71D0: capture receiver, current primary00 -> genuine ID/EAX,
    // publish captured receiver unchecked. 517ED0 ignores its normal return.
    virtual std::uint32_t register_screen_004f71d0(void* captured_screen) = 0;
    virtual void* gui_manager_004c12b0() = 0;
    virtual void* load_page_00aa5840(void* actual_manager,
        const void* actual_name_header8, std::int32_t screen_flag,
        std::uint8_t add_reference) = 0;
    // Side-effect-free metadata lookup of the captured CURRENT numeric profile,
    // not a table of host function pointers. Return its SAME live >=1 DWORD table.
    virtual const volatile std::uint32_t* current_page_table(void* captured_page,
        std::uint32_t captured_profile) = 0;
    // Invoke this captured actual CURRENT00 target on this SAME page. May throw.
    // BD30E0 requires its real fresh-current04/lifetime binding; a canonical
    // RenderCommandReference noexcept zero terminal is not this transport.
    virtual void call_page_virtual00(void* captured_page,
        std::uint32_t captured_target) = 0;
};
using NativeFrontEndFrameReferenceDecrement = long (__stdcall*)(volatile long*);

// Borrowed actual authority only. Bases are stable metadata, cells stay CURRENT
// and writable: E08500[5*13], E08604[5*2], E0862C[5*2]. No default bank, values,
// initialized names, per-Frame selectors or new publication/manager are owned.
// strings and all GUI providers must use the SAME application manager/string/
// Page domain. page_lifetime is the SAME provider retained by the GUI manager.
struct NativeFrontEndFrameSelectionContext {
    NativeStringRawPoolContext& strings;
    NativeFrontEndFrameSelectionBindings& bindings;
    NativeGuiPageLifetimeHost& page_lifetime;
    volatile std::uint32_t* const backdrop_name_words_00e08500;
    volatile std::uint32_t* const panel_name_words_00e08604;
    volatile std::uint32_t* const title_name_words_00e0862c;
    // Read THIS target cell separately at EVERY reached CE2220 call, after
    // capturing its Page. Do not snapshot its value into the context.
    NativeFrontEndFrameReferenceDecrement const volatile& decrement_00ce2220;
};

// Caller-retained raw8B temporary headers, not owning NativeString wrappers.
// Entry initializes only the header reached by an actual nonnull-name arm.
// No constructor/destructor, default zeroing, automatic release or retry.
// Keep each entered invocation's scratch distinct and alive through failure.
struct NativeFrontEndFrameStringTemporary {
    std::uint32_t words[2];
};
struct NativeFrontEndFrameRegistrationScratch {
    // Native517ED0 reuses its first header for backdrop and panel.
    NativeFrontEndFrameStringTemporary backdrop_panel;
    NativeFrontEndFrameStringTemporary title;
};
struct NativeFrontEndFrameSelectionScratch {
    NativeFrontEndFrameStringTemporary backdrop;
    NativeFrontEndFrameStringTemporary panel;
    NativeFrontEndFrameStringTemporary title;
};

// Whole517ED0[891B/276ops], ECX actual164h Frame, plain RET; current primary+10.
// Raw4F71D0 first; own+160=4; column-major then all5 sets for13/2/2 columns.
// Capture each outer CURRENT name using CURRENT own; reread own for every row.
// Actual8B string/41DD40/memcpy/getter/AA5840(header,1,0); publish before release.
void register_native_front_end_frame_00517ed0(void* captured_actual_frame,
    const NativeFrontEndFrameSelectionContext&,
    NativeFrontEndFrameRegistrationScratch&);

// Whole518250[1105B/345ops], ECX actual164h Frame, stack DWORD set/BYTE commit,
// RET8. Borrow the REAL caller homes: entry, every retirement row, each acquired
// group and final publication retain their native reloads. No bounds guard.
// Count1: real GUI getter, then reload SLOT, wholeAA31F0 with same page_lifetime.
// Other count: capture Page, reached CURRENT CE2220(+4), CURRENT profile/slot00
// at zero, throwing genuine Source target. Clear SLOT only after returned call.
// Acquisition loads SLOT then CURRENT name BEFORE branching, even for a hit.
// Native header cleanup is armed only after resize/copy, disarmed before normal
// pool return; covered ordinary C++ unwind destroys its current header once.
void select_native_front_end_frame_set_00518250(void* captured_actual_frame,
    const volatile std::uint32_t& requested_dword_home,
    const volatile std::uint8_t& commit_byte_home,
    const NativeFrontEndFrameSelectionContext&,
    NativeFrontEndFrameSelectionScratch&);

// New Source ABIs. Valid aligned same-domain raw storage/terminated names and
// returning genuine providers are required; no policy narrows requested values.
// Accessible 32-bit address arithmetic is retained, not arbitrary fault parity.
// No Original FH3/SEH/private-frame aliases/unmasked-FP/drop-in ABI/game binding.
// The bodies contain no x87 arithmetic. Partial native effects and raw scratch
// remain with the caller on failure; no rollback, reset, repeat or slot repair.
} // namespace bsp
