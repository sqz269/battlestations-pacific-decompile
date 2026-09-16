#pragma once

#include "bsp/native_render_batch_preparation_actual.hpp"
#include "bsp/native_renderer_cache_clear.hpp"
#include "bsp/native_renderer_surface_save_publish.hpp"
#include "bsp/native_renderer_vertex_binding.hpp"
#include "bsp/native_renderer_viewport_clear.hpp"
#include <cstdint>

namespace bsp {
class XLiveLibrary;

// Borrows the already loaded library. The library must outlive every call.
// Original C2F1CC is a JMP through CE25D8: xlive.dll ordinal 5002, stdcall, no args.
class NativeXLiveRenderImport final {
public:
    explicit NativeXLiveRenderImport(const XLiveLibrary&);
    std::uint32_t render_00c2f1cc() const;
private:
    using Render = std::uint32_t (__stdcall*)();
    Render render_;
};

struct NativePhysicalBufferRewindContext {
    void* const volatile& actual_renderer_00f8d394;
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    // Original numeric profile words, readable through slot +08; never executed.
    const volatile std::uint32_t* actual_logical_vertex_profile_00d61d6c;
    const volatile std::uint32_t* actual_logical_index_profile_00d61de0;
};

// Complete eight-byte ECX/RET leaves on the actual logical owners.
void __fastcall rewind_native_logical_vertex_00b48d40(void*) noexcept;
void __fastcall rewind_native_logical_index_00b48dd0(void*) noexcept;

// Complete 162-byte physical-owner bodies. Actual +08 array, signed +0C count,
// +1C cursor, +24 lock count. Only the observed D61D6C/D61DE0 child profiles bind.
void rewind_native_physical_vertex_00b232b0(void*, NativePhysicalBufferRewindContext&);
void rewind_native_physical_index_00b231c0(void*, NativePhysicalBufferRewindContext&);

// These bodies are not yet available for the actual owner graph. A production
// adapter must execute them substantively, including current callback dispatch.
// There are intentionally no default implementations or success fallbacks.
struct NativeRendererEndFrameRemaining {
    virtual ~NativeRendererEndFrameRemaining() = default;
    virtual void execute_queue_00b1ebe0(NativeRenderCommandQueueStorage&) = 0;
    virtual void render_00b2bb90(void* actual_renderer) = 0;
    virtual void render_00b2b580(void* actual_renderer) = 0;
};

struct NativeRendererDebugLinesContext;

struct NativeRendererEndFrameContext {
    NativeRenderBatchPreparationContext& actual_queue_getter;
    NativeRendererBindingResetContext& actual_bindings;
    NativeRendererCacheClearContext& actual_cache;
    const NativeRendererSurfaceSavePublishContext& actual_surface_save;
    const NativeRendererViewportClearContext& actual_clear;
    NativePhysicalBufferRewindContext& actual_rewind;
    const NativeXLiveRenderImport& actual_xlive;
    NativeRendererEndFrameRemaining& remaining;
    volatile std::uint8_t& actual_in_end_frame_0108d4cc;
    volatile std::uint8_t& actual_clear_request_00e1306c;
    volatile std::uint8_t& actual_present_failure_0108d4b9;
    void* const volatile& actual_counter_0108fe88;
    // Current numeric query table, borrowed through slot +10. Eligible pending
    // D62AD0 owners dispatch the complete raw B5FCA0 body directly.
    const volatile std::uint32_t* actual_query_profile_00d62ad0;
    // Substantive current +C4/B28D00 provider. Borrow the same actual renderer,
    // synchronization, vertex/index and profile domains. May be null only when
    // B28D00 observes an empty current +1D04 header and returns before use.
    const NativeRendererDebugLinesContext* actual_debug_lines;
};

// Complete B2D8E0 call schedule through B2DBCC, with the required frontier above.
// Context, publications, tables and all provider domains are borrowed and must
// describe the SAME actual renderer/device/cache/string/synchronization graph.
// An inactive DWORD +1998 returns before touching context (which may then be null).
// New C++ interfaces: original ECX/RET4 and FH3 stack identity are not promised.
void end_native_renderer_frame_00b2d8e0(void* actual_renderer,
    const void* actual_save_header, NativeRendererEndFrameContext*);
void end_native_renderer_frame_default_00b2f4a0(void* actual_renderer,
    NativeRendererEndFrameContext*);
void end_native_renderer_frame_with_save_00b2f4b0(void* actual_renderer,
    const void* actual_save_header, NativeRendererEndFrameContext*);
} // namespace bsp
