#pragma once
#include <cstdint>
struct lua_State;

namespace bsp::oct18b_nA2 {
// Unclassified leaf bodies: names describe instructions, not recovered symbols.
// Source interfaces below do not reproduce the original register/stack ABI.
// 004F16F0 / 004F1710: entity vtable slots 96/95, ECX ignored, RET4.
// 006578B0 / 006BA440 also contain only RET4 (latter vtable CFB9C8 slot1).
void ignore_argument_004f16f0(std::uint32_t) noexcept;
// 004F17D0: shared entity slot93, XOR EAX,EAX; RET.
std::uint32_t return_zero_004f17d0() noexcept;
// 0057FAC0 / 00612AF0: RET, no observed callers; purpose remains unknown.
void empty_leaf_0057fac0() noexcept;
// 0052D500: ECX=word, MOV EAX,ECX; [EAX]=0; RET. Preserves identity return.
void* clear_word_0052d500(void*) noexcept;
// 00534550: ECX=value, PUSH ECX; MOV AL,[ESP+3]; POP ECX; RET; AL only.
std::uint8_t high_byte_00534550(std::uint32_t) noexcept;
// 0054D450: ECX=object, returns DWORD +ECh, RET.
std::uint32_t word_ec_0054d450(const void*) noexcept;
// 00553060: ECX=object, returns this+Ch, RET.
void* member_c_00553060(void*) noexcept;
// 0057D350: ECX=object; two DWORDs and byte on stack, RET0C.
void set_fields_0057d350(void*,std::uint32_t,std::uint32_t,std::uint8_t) noexcept;
// 005457F0: ECX=object,requested/RET4; current kind at+8. Native alters AL
// only and leaves its high24 bits, reproduced in the returned DWORD.
std::uint32_t class_kind_matches_bits_005457f0(const void*,std::uint32_t) noexcept;
// 006B7EE0/006B7F00: ECX=context whose +4 is lua_State*,stack index/RET4.
// Exact comparisons with lua_type 0 and5. Link the stock interpreter binding.
bool lua_stack_is_nil_006b7ee0(const void*,std::int32_t);
bool lua_stack_is_table_006b7f00(const void*,std::int32_t);
// 005828D0: EAX=ECX; RET8. 00612970: same with RET4. Argument meanings unknown.
void* identity_005828d0(void*,std::uint32_t,std::uint32_t) noexcept;
void* identity_00612970(void*,std::uint32_t) noexcept;

struct PhaseWidgetCalls {
    virtual ~PhaseWidgetCalls() = default;
    // 00605070, ECX=float slot, RET; no guessed angle-normalization formula.
    virtual void wrap_angle_00605070(float*) = 0;
    // Original widget vtable+44h, ECX=widget, float on stack, RET4.
    virtual void widget_set_float_44(void*,float) = 0;
};
// 006054C0: ECX=object, float phase on stack, RET4. Object +0 reversal byte,
// +4/+8 widgets, +14 offset, +18/+1C periods, +20 current phase. Assembly uses
// x87 FDIV/FADD/FMUL(double CE3D28=400921FB60000000)/FADD, one float spill,
// wrap call, then optional SSE SUBSS(-0.0f,angle). 53-bit x87 model matches
// this project's established model; native x87 control-word/fault ABI unproved.
void update_phase_widgets_006054c0(void*,float,PhaseWidgetCalls&);
} // namespace bsp::oct18b_nA2
