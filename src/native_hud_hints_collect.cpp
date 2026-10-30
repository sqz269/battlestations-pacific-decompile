#include "bsp/native_hud_hints_collect.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);
static_assert(sizeof(NativeHudHintsCollectFrame) == 16);
static_assert(offsetof(NativeHudHintsCollectFrame, actual_parameter4) == 0);
static_assert(offsetof(NativeHudHintsCollectFrame, actual_push_parameter4) == 4);
static_assert(offsetof(NativeHudHintsCollectFrame, actual_push_private_output8) == 8);
static_assert(offsetof(NativeHudHintsCollectFrame, child_frame) == 12);

void collect_native_hud_hints_children_0054e370(void* actual_hints,
    const NativeHudHintsCollectFrame& frame) {
    using Word = std::uint32_t;
    const Word page_slot = reinterpret_cast<Word>(actual_hints) + 0x1cu;
    *static_cast<volatile Word*>(frame.actual_push_parameter4) = page_slot;
    const Word destination = *static_cast<const volatile Word*>(frame.actual_parameter4);
    const NativeSceneDwordVectorPushBackFrame child{
        frame.actual_push_parameter4, frame.actual_push_private_output8,
        frame.child_frame};
    push_back_native_scene_dword_vector_0046c450(
        reinterpret_cast<void*>(destination), child);
}
} // namespace bsp
