#include "bsp/native_tactical_library_collect.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);
static_assert(sizeof(NativeTacticalLibraryCollectFrame) == 16);
static_assert(offsetof(NativeTacticalLibraryCollectFrame, actual_parameter4) == 0);
static_assert(offsetof(NativeTacticalLibraryCollectFrame, actual_push_parameter4) == 4);
static_assert(offsetof(NativeTacticalLibraryCollectFrame, actual_push_private_output8) == 8);
static_assert(offsetof(NativeTacticalLibraryCollectFrame, child_frame) == 12);

void collect_native_tactical_library_children_00509da0(void* actual_tactical,
    const NativeTacticalLibraryCollectFrame& frame) {
    using Word = std::uint32_t;
    const Word receiver = reinterpret_cast<Word>(actual_tactical);
    const Word destination = *static_cast<const volatile Word*>(frame.actual_parameter4);
    void* const home = frame.actual_push_parameter4;
    const NativeSceneDwordVectorPushBackFrame child{
        home, frame.actual_push_private_output8, frame.child_frame};

    *static_cast<volatile Word*>(home) = receiver + 0x4dcu;
    push_back_native_scene_dword_vector_0046c450(reinterpret_cast<void*>(destination), child);
    *static_cast<volatile Word*>(home) = receiver + 0x4e0u;
    push_back_native_scene_dword_vector_0046c450(reinterpret_cast<void*>(destination), child);
    *static_cast<volatile Word*>(home) = receiver + 0x4e4u;
    push_back_native_scene_dword_vector_0046c450(reinterpret_cast<void*>(destination), child);
}

} // namespace bsp
