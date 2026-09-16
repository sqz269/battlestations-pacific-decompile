#include "bsp/native_render_queue_destruction.hpp"
#include "bsp/native_render_queue_rows.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render queue destruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_members(NativeRenderCommandQueueStorage& queue,
    NativeRenderQueueDestructionContext& context, int state) noexcept {
    __try {
        if (state >= 2)
            destroy_native_render_queue_rows_00b1f150(&queue.rows_data_24,
                context.actual_row_strings);
        if (state >= 1)
            destroy_native_render_command_pointers_00b1d590(queue.commands_14);
        if (state >= 0)
            destroy_native_render_queue_base_00b1c3c0(&queue,
                context.actual_queue_00f8d440);
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct QueueCleanup {
    NativeRenderCommandQueueStorage& queue;
    NativeRenderQueueDestructionContext& context;
    int state = 2;
    ~QueueCleanup() noexcept {
        if (state >= 0) unwind_members(queue, context, state);
    }
};
} // namespace

void destroy_native_render_queue_00b1f330(NativeRenderCommandQueueStorage& queue,
    NativeRenderQueueDestructionContext& context) {
    volatile auto& actual = queue;
    actual.native_vtable_00 = 0x00d5e5f4u;
    QueueCleanup cleanup{queue, context};
    execute_native_render_queue_00b1ebe0(queue,
        context.actual_execution, context.actual_execution_frame);
    cleanup.state = 1; // B1F368, before the row resize/free sequence.
    resize_native_render_queue_rows_00b1e7f0(&queue.rows_data_24,
        context.actual_row_strings, 0);
    singleton_lifetime_free(actual.rows_data_24);
    cleanup.state = 0; // B1F384, before the command resize/free sequence.
    resize_native_render_command_pointers_00b1cc80(queue.commands_14, 0);
    singleton_lifetime_free(actual.commands_14.data_00);
    destroy_native_render_queue_base_00b1c3c0(&queue,
        context.actual_queue_00f8d440);
    cleanup.state = -1;
}

NativeRenderCommandQueueStorage* delete_native_render_queue_00b1f6b0(
    NativeRenderCommandQueueStorage& queue, std::uint32_t flags,
    NativeRenderQueueDestructionContext& context) {
    auto* const owner = &queue;
    destroy_native_render_queue_00b1f330(queue, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
