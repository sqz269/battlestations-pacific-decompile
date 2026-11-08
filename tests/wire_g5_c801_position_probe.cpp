#include "bsp/gui_widgets_cc13.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>

namespace {
std::uint32_t bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void require(bool condition) {
    if (!condition) throw std::runtime_error("position publication contract failed");
}
struct Publication final : bsp::GuiWidgetTransformHost {
    bsp::GuiWidgetTransform& receiver;
    std::uint32_t expected_z;
    std::string calls;
    Publication(bsp::GuiWidgetTransform& widget, std::uint32_t z)
        : receiver(widget), expected_z(z) {}
    bool widescreen_enabled() override { throw std::runtime_error("unexpected layout query"); }
    void publish_local_transform(bsp::GuiWidgetTransform& widget,
        const bsp::GuiWidgetLocalTransform& transform) override {
        require(&widget == &receiver && bits(widget.position.z) == expected_z);
        require(widget.position.x == 3.0f && widget.position.y == 4.0f);
        require(bits(transform.translation.z) == expected_z);
        calls += 'T';
    }
    void publish_local_bounds(bsp::GuiWidgetTransform& widget,
        const bsp::GuiWidgetBounds& bounds) override {
        require(&widget == &receiver && calls == "T");
        require(bounds.half_width == 3.0f && bounds.half_height == 4.0f);
        require(bounds.half_depth == 0.0f && bounds.radius == 4.0f);
        calls += 'B';
    }
    void notify_layout_changed(bsp::GuiWidgetTransform&) override {
        throw std::runtime_error("unexpected layout notification");
    }
    void on_layout_finished(bsp::GuiWidgetTransform&) override {
        throw std::runtime_error("unexpected layout completion");
    }
};
}

int main() {
    // The native MOVSS store must reach the actual receiver before either
    // publication, with exactly one recompose and the native +74h bounds gate.
    bsp::GuiWidgetTransform widget;
    widget.position.x = 3.0f;
    widget.position.y = 4.0f;
    widget.size = {6.0f, 8.0f};
    widget.bounds_enabled = true;
    const std::uint32_t signaling_nan = 0x7f812345u;
    float z;
    std::memcpy(&z, &signaling_nan, sizeof(z));
    Publication publication(widget, signaling_nan);
    bsp::gui_widgets_cc13::gui_widget_set_local_z_and_bounds_00aa7d40(widget, z, publication);
    require(publication.calls == "TB");
    widget.bounds_enabled = false;
    publication.expected_z = 0x80000000u;
    publication.calls.clear();
    bsp::gui_widgets_cc13::gui_widget_set_local_z_and_bounds_00aa7d40(widget, -0.0f, publication);
    require(publication.calls == "T");
    std::puts("c801 position probe: MOVSS payload, single recompose, ordered/gated bounds passed");
}
