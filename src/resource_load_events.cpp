#include "bsp/resource_load_events.hpp"

namespace bsp {
void pump_resource_load_events_00beccd0(ResourceLoadEventHost& host) {
    MSG message;
    while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
        if (!host.pretranslate(message)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    host.update_cursor_focus_00becb20(true);
}
}
