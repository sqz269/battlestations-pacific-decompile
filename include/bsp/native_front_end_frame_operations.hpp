#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw front-end Frame operations require MSVC Win32.
#endif

namespace bsp::game { class GameNativeReadOnlyData; }
namespace bsp {
struct NativeStringRawPoolContext;

// Preentry borrowed literal/canonical-RO lifetime capability. No copied name
// bank or default supplier. The SAME authenticated original-VA mapping must
// also back every CURRENT raw widget profile table reached by these bodies.
class NativeFrontEndFrameOperationLiterals final {
public:
    const char* flag_icon_name_00cec3f0() const noexcept { return flag_icon_name_; }
private:
    NativeFrontEndFrameOperationLiterals(const game::GameNativeReadOnlyData& owner,
        const char* flag_icon_name) noexcept : owner_(owner), flag_icon_name_(flag_icon_name) {}
    friend NativeFrontEndFrameOperationLiterals borrow_native_front_end_frame_operation_literals(
        const game::GameNativeReadOnlyData&);
    const game::GameNativeReadOnlyData& owner_;
    const char* flag_icon_name_;
};
NativeFrontEndFrameOperationLiterals borrow_native_front_end_frame_operation_literals(
    const game::GameNativeReadOnlyData&);

// Genuine required application operations, not implementations or fallbacks.
// Receivers remain the captured raw identities even after global replacement.
// Codewords are DATA tokens; never cast them to host callable pointers.
struct NativeFrontEndFrameOperationsBindings {
    virtual ~NativeFrontEndFrameOperationsBindings() = default;
    virtual void* frame_004c1ac0() = 0;
    virtual void* find_child_00aa7e00(void* actual_parent,
        const void* actual_name_header8, std::uint32_t one) = 0;
    virtual void call_widget_virtual34(void* captured_widget,
        std::uint32_t captured_profile, std::uint32_t captured_target,
        std::uint32_t actual_argument_word) = 0;
    virtual void call_widget_virtual88(void* captured_widget,
        std::uint32_t captured_profile, std::uint32_t captured_target,
        std::uint32_t frame_word, std::uint32_t zero_word,
        std::uint32_t alpha_float_bits) = 0;
    // Whole actual5189B0 is still required; no typed/copied-string substitute.
    virtual void title_005189b0(const void* captured_native_header8) = 0;
};

// Mandatory nonowning references to the SAME process cells; no four-field
// selection owner, defaults, snapshots, name table or new Game publication.
// Game is the existing GameNativeSettingsProcess::game_00e188a8() Source cell,
// SAME as actual Game construction/lifetime; not a numeric E188A8 shadow.
struct NativeFrontEndFrameOperationsContext {
    volatile std::int32_t& backdrop_00e18d84;
    volatile std::int32_t& panel_00e18d88;
    volatile std::int32_t& title_00e18d8c;
    volatile std::uint32_t& flag_00e08654;
    void* const volatile& actual_game_00e188a8;
    NativeStringRawPoolContext& strings;
    const NativeFrontEndFrameOperationLiterals& literals;
    NativeFrontEndFrameOperationsBindings& bindings;
};

// Actual8B caller temporary, initialized only by reached real41E870. Retain
// distinct scratch for each invocation/reentry, including partial failures.
// No implicit initialization, destructor cleanup, phase, retry or policy.
struct NativeFrontEndFrameFlagScratch { std::uint32_t name_header[2]; };

// Whole518B90, native ECX flag, plainRET. Signed panel comparison precedes flag
// DWORD store. Repeated real getter pairs can name DIFFERENT Frame generations.
// Real41E870/AA7E00/current88, then actual header return. Late LOW WORD flag and
// x87 FLD1/profile+88/flag/FSTP sequence preserve the actual call words.
void set_native_front_end_frame_flag_00518b90(std::uint32_t flag,
    const NativeFrontEndFrameOperationsContext&, NativeFrontEndFrameFlagScratch&);

// Whole518C80, captured native ECX Frame, plainRET/tail518B90. Preserve exact
// CURRENT raw profile/target reads and complete SETZ DWORD carriers (upper24
// computed index bits retained), not bool shown. Mode0D skips backdrop AND flag.
void apply_native_front_end_frame_selection_00518c80(void* captured_frame,
    const NativeFrontEndFrameOperationsContext&, NativeFrontEndFrameFlagScratch&);

// Whole518D60, native ECX backdrop/EDX panel, stack titleDWORD/text pointer,
// RET8. Read title home FIRST; write84/88/8C; actual getter/apply; only THEN
// read CURRENT text-pointer home/header length, and call genuine title if nonzero.
void select_native_front_end_frame_00518d60(std::uint32_t backdrop,
    std::uint32_t panel, const volatile std::uint32_t& title_dword_home,
    const void* const volatile& text_pointer_home,
    const NativeFrontEndFrameOperationsContext&, NativeFrontEndFrameFlagScratch&);

// Whole518DA0, native ECX backdrop; write84, getter then tail whole apply.
void select_native_front_end_frame_backdrop_00518da0(std::uint32_t backdrop,
    const NativeFrontEndFrameOperationsContext&, NativeFrontEndFrameFlagScratch&);

// Caller proves valid aligned same-domain raw receivers/fields and CURRENT
// numeric profile tables backed at original VAs by the SAME retained canonical
// authenticated RO owner. Loads of those table DATA words are direct; captured
// targets require genuine finite Source dispatch. No synthesized table, target
// fallback, numeric-codeword invocation or early/current publication shortcut.
// All providers/cells/scratch survive callbacks. Ordinary Source exception
// transport only: no added noexcept/termination/retry/rollback/ack policy.
// No Original ABI/FH3/SEH/private-frame/fault/x87-stack/unmasked-FP/runtime proof.
} // namespace bsp
