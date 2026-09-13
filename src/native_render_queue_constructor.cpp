#include "bsp/native_render_queue_constructor.hpp"
#include "bsp/native_render_queue_rows.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render queue construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_members(NativeRenderCommandQueueStorage& queue,
    NativeRenderQueueConstructorContext& context, int state) noexcept {
    __try {
        if (state >= 2)
            destroy_native_render_queue_rows_00b1f150(&queue.rows_data_24,
                context.actual_row_strings);
        if (state >= 1)
            destroy_native_render_command_pointers_00b1d590(queue.commands_14);
        if (state >= 0)
            destroy_native_render_queue_base_00b1c3c0(&queue, context.actual_queue_00f8d440);
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct QueueCleanup {
    NativeRenderCommandQueueStorage& queue;
    NativeRenderQueueConstructorContext& context;
    int state = 0;
    ~QueueCleanup() noexcept { if (state >= 0) unwind_members(queue, context, state); }
};
} // namespace

NativeRenderCommandQueueStorage* construct_native_render_queue_00b1f280(
    NativeRenderCommandQueueStorage& queue, NativeRenderQueueConstructorContext& context) {
    volatile auto& actual = queue;
    actual.native_vtable_00 = 0x00d5e5f4u;
    actual.configurations_04[0].enabled_00 = 0;
    actual.configurations_04[0].value_04 = 0;
    actual.configurations_04[1].enabled_00 = 0;
    actual.configurations_04[1].value_04 = 0;
    QueueCleanup cleanup{queue, context}; // Native state0 at B1F2B2.
    actual.commands_14.data_00 = nullptr;
    actual.commands_14.count_04 = 0;
    actual.commands_14.capacity_08 = 0;
    actual.rows_data_24 = nullptr;
    actual.rows_count_28 = 0;
    actual.rows_capacity_2c = 0;
    const bool must_stop = actual.control_20 == 2; // BEFORE the +30 store.
    cleanup.state = 2; // Native B1F2CC, after comparison and before +30.
    actual.context_30 = nullptr;
    if (must_stop) {
        auto* const renderer = const_cast<void*>(context.actual_renderer_00f8d394);
        const auto profile = word(renderer);
        __assume(profile == 0x00d5f0a8u);
        const auto target = word(
            context.actual_stop.actual_bindings.actual_renderer_profile_00d5f0a8, 0x11c);
        __assume(target == 0x00b28a90u);
        stop_native_renderer_worker_mode_00b28a90(renderer, context.actual_stop);
    }
    actual.control_20 = 0;
    for (Word index = 0; index != 2; ++index) {
        actual.configurations_04[index].enabled_00 = 0;
        actual.configurations_04[index].value_04 = 0;
    }
    actual.configurations_04[0].value_04 = 0;
    actual.configurations_04[0].enabled_00 = 1;
    actual.configurations_04[1].value_04 = 0;
    actual.configurations_04[1].enabled_00 = 1;
    cleanup.state = -1;
    return &queue;
}
} // namespace bsp
