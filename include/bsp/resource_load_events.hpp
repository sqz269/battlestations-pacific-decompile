#pragma once
#include "bsp/platform_loop.hpp"

namespace bsp {
// Actual native dependencies remain required: XLive pretranslation and the
// current platform owner's cursor/focus policy. No replacement defaults.
struct ResourceLoadEventHost {
    virtual ~ResourceLoadEventHost() = default;
    virtual bool pretranslate(MSG&) = 0;
    virtual void update_cursor_focus_00becb20(bool loading) = 0;
};
// Native ECX platform, no stack arguments, RET at 00BECD36. Drains all thread
// messages (including WM_QUIT), then invokes the cursor/focus policy with 1.
// There is no frame callback or loop-exit test in this load-time pump.
void pump_resource_load_events_00beccd0(ResourceLoadEventHost&);
}
