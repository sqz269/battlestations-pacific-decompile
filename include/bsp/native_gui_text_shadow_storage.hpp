#pragma once
#include <cstdint>

namespace bsp {
// Ordinary private16B backing. The user-provided constructor seeds no Native
// word, including under value-initialization. Only preset1 reaches the spills.
struct NativeGuiTextShadowStorageFrame final {
    NativeGuiTextShadowStorageFrame() {}
    NativeGuiTextShadowStorageFrame(const NativeGuiTextShadowStorageFrame&) = delete;
    NativeGuiTextShadowStorageFrame& operator=(const NativeGuiTextShadowStorageFrame&) = delete;
    NativeGuiTextShadowStorageFrame(NativeGuiTextShadowStorageFrame&&) = delete;
    NativeGuiTextShadowStorageFrame& operator=(NativeGuiTextShadowStorageFrame&&) = delete;
    std::uint32_t words_10[4];
};
static_assert(sizeof(NativeGuiTextShadowStorageFrame) == 16);
static_assert(alignof(NativeGuiTextShadowStorageFrame) == alignof(std::uint32_t));

// Cold address loans only. Bind the SAME established RO constant cells and
// game_native_gui_text_state().shadow_color_00e12fd8() quartet. No copy, new
// process role, eager initializer or value sampling belongs in the binding.
struct NativeGuiTextShadowStorageContext {
    const volatile float& offset_00d5c5c0;
    const volatile float& one_00d7a24c;
    const volatile float& white_alpha_00cee07c;
    const volatile float (&shadow_color_00e12fd8)[4];
};

// Whole NORMAL [00AB72D0,00AB73A9),217B/40 Original starts, inclusive73A8.
// Original ECX Text, one preset DWORD/RET4 becomes this ordinary Source API.
// actual_preset_home is an ordinary mutable DWORD borrowed via a volatile
// view. Read its full value ONCE at entry; no store or late reload occurs.
// The frame models the four reached private SSE spills, not Original stack,
// return-address, saved-register, FLAGS or binary calling-convention identity.
//
// Caller supplies the genuine live actual1F4 Text and valid C++ storage/alias
// domains for every reached field, operand, frame and retained constant cell.
// Source controls/address carriers remain separate from Native write extents.
// No semantic GuiText/GuiWidgetOwner cast, class/profile/count admission,
// ownership operation, provider callback, float arithmetic or Native cleanup
// is added. Original fault/FH3/SEH/NoThrow/Origin/unmasked-FP/whole-object and
// production provider/lifecycle/runtime equivalence remain excluded.
void apply_native_gui_text_shadow_storage_00ab72d0(void* actual_text1f4,
    volatile std::uint32_t& actual_preset_home, NativeGuiTextShadowStorageFrame&,
    const NativeGuiTextShadowStorageContext&);
} // namespace bsp
