#pragma once
#include <cstdint>
#include <memory>

namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;
class GameSingletonHost;
class GameNativeReadOnlyData;
class GameInputRuntime;
class GameFontHost;
class GameLocaleHost;

enum class GameNativeGuiLabelTransition { not_entered, entered, unknown };

// The mission's existing command owner supplies the queue and pass context.
// B1F4D0 consumes the actual eight-byte label argument. This interface adds
// no queue, BeginScene/EndScene/Present, command owner, or frame clock.
class GameNativeGuiQueue {
public:
    virtual ~GameNativeGuiQueue() = default;
    virtual void* queue_004c11f0() = 0;
    virtual void* pass_context_00b0d0d0() = 0;
    virtual void submit_scene_00b1f4d0(void* queue, void* scene, void* camera,
        void* pass_context, void* actual_label, GameNativeGuiLabelTransition&) = 0;
};

// One optional renderer-side GUI domain. No GUI manager, front-end frame,
// screen-base manager or in-mission interface manager is constructed.
// Native Text uses the canonical glyph buffers/materials/font resources;
// AA3E00 submits its own actual scene/camera into the mission's SAME queue.
// Retain this graph through caller retirement and the shared native drain.
class GameNativeGuiRenderApplication final {
public:
    // BSP_NATIVE_GUI_TEXT=1; default OFF. Startup also requires the shared
    // BSP_NATIVE_MISSION_SHIP=1 and BSP_NATIVE_MISSION_FRAME=1 routes.
    static bool text_requested() noexcept;
    GameNativeGuiRenderApplication(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameSingletonHost&, GameNativeReadOnlyData&, GameInputRuntime&,
        GameFontHost&, GameLocaleHost&, const bool& crt_sse2_conversion);
    ~GameNativeGuiRenderApplication() noexcept;
    GameNativeGuiRenderApplication(const GameNativeGuiRenderApplication&) = delete;
    GameNativeGuiRenderApplication& operator=(const GameNativeGuiRenderApplication&) = delete;

    // Invoke inside the mission resource preparation's existing B3A600/B3B140
    // bracket. The renderer owns that bracket; this application never opens it.
    void prepare_text_in_open_shader_cache();
    void submit_mission_frame(GameNativeGuiQueue&);
    // After every command which borrowed this domain has actually retired.
    // Text scalar deletion precedes GameFontHost::retire_native_fonts().
    void retire_before_fonts();
    // After the renderer's genuine singleton/cache drain, before destroying
    // this metadata. Text services remain alive while the effect binding closes.
    void after_native_renderer_drain();
    bool ready() const noexcept;
    bool requires_process_retention() const noexcept;
    std::uint64_t submitted_frames() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
