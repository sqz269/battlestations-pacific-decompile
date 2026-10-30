#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw front-end Frame title/position operations require MSVC Win32.
#endif

namespace bsp::game { class GameNativeReadOnlyData; }
namespace bsp {
struct NativeStringRawPoolContext;

// Preentry borrowed capability for the SAME authenticated original-VA RO owner.
// This view holds pointer metadata, never a replacement literal/constant bank.
// The owner must also back every CURRENT numeric widget profile table below.
class NativeFrontEndFrameTitlePositionData final {
public:
    const char* title_text_name_00cec3d8() const noexcept { return title_text_name_; }
    const char* frame_box_name_00cec3c4() const noexcept { return frame_box_name_; }
    const void* divisor_00cec380() const noexcept { return divisor_; }
    const void* pad_00cec3e8() const noexcept { return pad_; }
    const void* minimum_width_00cec3e4() const noexcept { return minimum_width_; }
    const void* fixed_width_00ce3e18() const noexcept { return fixed_width_; }
private:
    NativeFrontEndFrameTitlePositionData(const game::GameNativeReadOnlyData& owner,
        const char* title_text_name, const char* frame_box_name, const void* divisor,
        const void* pad, const void* minimum_width, const void* fixed_width) noexcept
        : owner_(owner), title_text_name_(title_text_name), frame_box_name_(frame_box_name),
          divisor_(divisor), pad_(pad), minimum_width_(minimum_width), fixed_width_(fixed_width) {}
    friend NativeFrontEndFrameTitlePositionData borrow_native_front_end_frame_title_position_data(
        const game::GameNativeReadOnlyData&);
    const game::GameNativeReadOnlyData& owner_;
    const char* title_text_name_;
    const char* frame_box_name_;
    const void* divisor_;
    const void* pad_;
    const void* minimum_width_;
    const void* fixed_width_;
};
NativeFrontEndFrameTitlePositionData borrow_native_front_end_frame_title_position_data(
    const game::GameNativeReadOnlyData&);

// Genuine required operations, still unbound to the actual retained application.
// A captured receiver may differ from the CURRENT publication. No fallback or
// typed widget/Mesh substitution is supplied. A numeric target is DATA only.
struct NativeFrontEndFrameTitlePositionBindings {
    virtual ~NativeFrontEndFrameTitlePositionBindings() = default;
    virtual void* frame_004c1ac0() = 0;
    virtual void text_source_00abaed0(void* captured_text,
        const void* captured_native_header8, std::uint32_t one) = 0;
    virtual const void* widget_size_00aa6740(void* captured_widget) = 0;
    virtual void set_local_position_00aa7dc0(void* captured_widget,
        const void* captured_xyz) = 0;
    virtual void call_widget_virtual58(void* captured_widget,
        std::uint32_t captured_profile, std::uint32_t captured_target,
        void* actual_float2) = 0;
    virtual void push_page_slot_004d6790(void* captured_output_vector,
        void* actual_page_slot_address) = 0;
};

// Mandatory nonowning SAME-process references, not a copied selection owner.
// Game borrows GameNativeSettingsProcess::game_00e188a8(), also used by actual
// Game construction/lifetime. Strings borrow the SAME real singleton manager.
struct NativeFrontEndFrameTitlePositionContext {
    volatile std::int32_t& backdrop_00e18d84;
    volatile std::int32_t& panel_00e18d88;
    volatile std::int32_t& title_00e18d8c;
    void* const volatile& actual_game_00e188a8;
    NativeStringRawPoolContext& strings;
    const NativeFrontEndFrameTitlePositionData& data;
    const volatile std::uint32_t& actual_feature_word_0109eea4;
    NativeFrontEndFrameTitlePositionBindings& bindings;
};

// Real temporary homes; no implicit initialization/destruction or policy. Keep
// distinct retained scratch per invocation/reentry, including partial failures.
struct NativeFrontEndFrameTitleScratch {
    // After its return, word0 becomes the quotient FLOAT home (native ESP+0C).
    // The true size getter runs between that store and its later reload.
    std::uint32_t title_text_name_header[2];
    std::uint32_t frame_box_name_header[2];
    std::uint32_t plus_or_fixed_size_float_bits[2];
    std::uint32_t minimum_size_float_bits[2];
};
struct NativeFrontEndFramePositionScratch {
    std::uint32_t title_text_name_header[2];
    std::uint32_t frame_box_name_header[2];
};

// Whole5189B0; native ECX actual string8h, plainRET. Real ABAED0(header,1),
// extent/FDIV double960/FSTP float, FCOMI/FSTP/JBE (unordered -> minimum), true
// size getter across the spill, then FADD double pad/FSTP. No double projection.
void set_native_front_end_frame_title_005189b0(const void* captured_native_header8,
    const NativeFrontEndFrameTitlePositionContext&, NativeFrontEndFrameTitleScratch&);

// Whole5186B0; native ECX box XYZ/EDX title XYZ, plainRET. Find text then box;
// set box first and text second. Do not copy/normalize either input vector.
void set_native_front_end_frame_title_position_005186b0(const void* captured_box_xyz,
    const void* captured_title_xyz, const NativeFrontEndFrameTitlePositionContext&,
    NativeFrontEndFramePositionScratch&);

// Whole518820; native ECX box output/EDX title output, plainRET. A real getter
// precedes each group's three ordered FLD/FSTP pairs. Existing actual-pointer
// AA67F0 is reused directly; overlap stays observable.
void get_native_front_end_frame_title_position_00518820(void* captured_box_output_xyz,
    void* captured_title_output_xyz, const NativeFrontEndFrameTitlePositionContext&,
    NativeFrontEndFramePositionScratch&);

// Whole518E00; native ECX captured Frame/stack output-vector pointer, RET4.
// Read the output home ONCE after the first Page test. Three groups reload
// own+selector separately and pass real slot ADDRESSES, before the next group.
void collect_native_front_end_frame_children_00518e00(void* captured_frame,
    void* const volatile& actual_output_vector_home,
    const NativeFrontEndFrameTitlePositionContext&);

// Valid same-domain raw extents/headers, ambient FP stack/CW, and retained
// receivers/contexts/providers/scratch belong to the actual caller. CURRENT
// profile table DATA words require the SAME authenticated canonical original-VA
// RO mapping/lifetime proof; captured target dispatch must be genuine Source,
// never a numeric-codeword cast or synthesized table. No FP reset is supplied.
// Ordinary Source exceptions only: one-shot armed cleanup, original exception
// rethrows if cleanup returns; a cleanup failure escapes. No extra termination,
// retry/rollback/ack policy, Original ABI/FH3/SEH/private-frame/fault/unmasked-FP
// identity or runtime/game/activation proof. Whole frontend readiness is false.
} // namespace bsp
