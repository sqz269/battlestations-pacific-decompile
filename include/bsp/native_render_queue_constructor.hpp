#pragma once

#include "bsp/native_render_queue_access.hpp"
#include "bsp/native_renderer_stop_pipeline.hpp"

namespace bsp {
class NativeStringStorage;

struct NativeRenderQueueConstructorContext {
    // Borrow the current publication CELLS, not snapshots of either owner.
    const void* volatile& actual_renderer_00f8d394;
    NativeRenderCommandQueueStorage* volatile& actual_queue_00f8d440;
    NativeRendererStopPipelineContext& actual_stop;
    NativeStringStorage& actual_row_strings;
};

// Complete B1F280..B1F320. Original ECX=actual 34h queue, EAX=same queue,
// no stack arguments, RET. Preserve preexisting control+20: capture ==2
// before clearing context+30; only that path calls the complete B28A90 stop
// selected by current renderer D5F0A8+11C. Stop retains its real Sleep(100).
// Padding+05..07/+0D..0F survives. Stop changes to command/row headers and
// context+30 survive success. No allocation, registration or publication.
// A C++ exception from stop destroys current rows, current command storage,
// then the base (unconditionally clearing actual F8D440). The original FH3
// state map is represented; native frame ABI/hardware SEH are not supplied.
// Context domains must be those of the SAME actual renderer, queue, global
// synchronization storage, allocators and strings. Reached storage/profiles
// remain valid; arbitrary profile replacement/concurrent mutation is unproved.
// New C++ interface; not a drop-in original ABI replacement or full lifecycle.
NativeRenderCommandQueueStorage* construct_native_render_queue_00b1f280(
    NativeRenderCommandQueueStorage&, NativeRenderQueueConstructorContext&);

} // namespace bsp
