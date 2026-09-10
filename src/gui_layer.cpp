// Reconstruction of `cGuiLayer` and the GUI manager routines that create,
// update, route the pointer into and tear down a page. Evidence and addresses
// are in docs/GUI_LAYER_MANAGER.md and in include/bsp/gui_layer.hpp.
#include "bsp/gui_layer.hpp"

#include <algorithm>

namespace bsp {
namespace {

// 00AC6624..00AC6710 and 00AA3852..00AA3877 write the same defaults. Kept in one
// place so the two constructors cannot drift.
GuiLayerImage default_layer_fields() {
    GuiLayerImage layer{};
    layer.key.flags = 0;            // +108h
    layer.key.near_plane = 0.1F;    // +10Ch, 00D7A2F0
    layer.key.far_plane = 1000.0F;  // +110h, 00CE3804
    layer.key.scale = 1.0F;         // +114h, 00D7A24C
    layer.key.priority = 0;         // +118h
    layer.key.render_order = 0.0F;  // +11Ch
    return layer;
}

}  // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

GuiLayerImage construct_00aa3840() {
    // 00AA3840 leaves +0ECh, +0F0h, +0F4h, +0F5h and +120h to whatever the widget
    // constructor put there; the projection starts them at their zero values.
    GuiLayerImage layer = default_layer_fields();
    layer.name.clear();  // +100h length and +104h pointer both zeroed
    return layer;
}

GuiLayerImage construct_from_script_00ac6600(std::string_view name, bool share_scene) {
    GuiLayerImage layer = default_layer_fields();
    layer.scene = nullptr;                  // 00AC662D
    layer.store = nullptr;                  // 00AC6634
    layer.visible = false;                  // 00AC663B, byte +0F5h
    layer.base.applied_priority = 0;        // 00AC6649, +0FCh
    layer.name.assign(name);                // 00AC6666 resize + 00AC6696 memcpy
    layer.share_scene = share_scene;        // 00AC66F1, byte +120h
    layer.reserved_121 = 0;                 // 00AC66F7
    return layer;
}

GuiLayerCreateKind create_kind_00aa3bd0(bool has_source) noexcept {
    // 00AA3BF3 TEST ESI,ESI: a null source takes the default constructor.
    return has_source ? GuiLayerCreateKind::Copy : GuiLayerCreateKind::Default;
}

// ---------------------------------------------------------------------------
// The two authored properties
// ---------------------------------------------------------------------------

void apply_read_properties_00ac4c50(
    GuiLayerImage& layer, std::int32_t priority, float render_order) noexcept {
    layer.base.applied_priority = priority;  // 00AC4C94, +0FCh
    layer.key.priority = priority;           // 00AC4C9A, +118h
    layer.key.render_order = render_order;   // 00AC4CBB, +11Ch
}

bool layer_property_is_described_00ac4d30(std::string_view key) noexcept {
    // 00AC4D38..00AC4D55 describes `Priority` and nothing else before chaining to
    // 00AAAED0. `RenderOrder` is read by 00AC4C50 but never described.
    return key == kGuiLayerPriorityKey;
}

// ---------------------------------------------------------------------------
// Ordering
// ---------------------------------------------------------------------------

bool needs_reregistration_00ac59a0(const GuiLayerImage& layer) noexcept {
    return layer.base.applied_priority != layer.key.priority;  // 00AC5EFE
}

void adopt_authored_priority_00ac59a0(GuiLayerImage& layer) noexcept {
    layer.base.applied_priority = layer.key.priority;  // 00AC5F06
}

bool reuses_store_00ac59a0(bool share_scene, bool store_found) noexcept {
    return share_scene && store_found;  // 00AC59CB CMP byte +120h / 00AC59D2
}

std::int32_t scene_flags_00ac59a0(std::int32_t layer_flags) noexcept {
    return layer_flags | kGuiLayerSceneFlagMask;  // 00AC5EF6 OR 6
}

// ---------------------------------------------------------------------------
// Visibility
// ---------------------------------------------------------------------------

int store_count_delta_00ac4450(bool current, bool requested) noexcept {
    if (current == requested) {
        return 0;  // 00AC445E JZ skips the adjustment entirely
    }
    return requested ? 1 : -1;  // 00AC446A ADD +1 / 00AC4470 ADD -1
}

GuiPropagateVisibilityCall propagate_call_00ac4450(
    const GuiLayerImage& layer, bool visible) noexcept {
    GuiPropagateVisibilityCall call{};
    call.arg0 = 1;
    call.arg1 = 1;
    call.visible = visible;
    call.propagate_flag = layer.base.propagate_flag;  // 00AC4474 MOVZX byte +75h
    call.arg4 = 1;
    return call;
}

GuiLayerSceneBinding scene_binding_00ac4450(
    const GuiLayerImage& layer, bool visible) noexcept {
    GuiLayerSceneBinding binding{};
    if (layer.base.scene_node == nullptr) {
        return binding;  // 00AC448A JZ: no node, no bind call at all
    }
    binding.called = true;
    binding.scene = visible ? layer.scene : nullptr;  // 00AC4490 / 00AC4499
    return binding;
}

bool notifies_hook_00ac4450(bool hook_installed, bool current, bool requested) noexcept {
    return hook_installed && current != requested;  // 00AC44A0 and 00AC44A9
}

void set_visible_00ac4450(GuiLayerImage& layer, bool visible) noexcept {
    const int delta = store_count_delta_00ac4450(layer.visible, visible);
    if (delta != 0 && layer.store != nullptr) {
        layer.store->visible_layers += delta;  // store +20h
    }
    layer.visible = visible;  // 00AC44C4, written last
}

// ---------------------------------------------------------------------------
// The per-frame update
// ---------------------------------------------------------------------------

bool updates_this_frame_00aa87b0(bool visible, bool has_live_entries) noexcept {
    // 00AA87FB then 00AA880C: visible short-circuits, otherwise the +88h/+8Ch
    // entry array is scanned for a non-null slot. The manager's page walk at
    // 00AA4FFB/00AA500C applies the identical rule.
    return visible || has_live_entries;
}

float advance_elapsed_00aa87b0(float elapsed, float seconds) noexcept {
    return elapsed + seconds;  // 00AA87BE, before the child walk
}

// ---------------------------------------------------------------------------
// Pointer routing
// ---------------------------------------------------------------------------

float scale_pointer_delta_00aa3910(float raw_delta) noexcept {
    // 00AA3A45 FLD double [00D5BEC8], FMUL, FSTP float -- rounded to float --
    // then 00AA3A65 FLD double [00D7A308], FMUL, FSTP float. Two roundings, not
    // one, so the product is written the way the native writes it.
    const float first = static_cast<float>(
        static_cast<double>(raw_delta) * kGuiPointerScaleFirst);
    return static_cast<float>(static_cast<double>(first) * kGuiPointerScaleSecond);
}

GuiPointerBounds pointer_x_bounds_00aa3910(bool wide_screen) noexcept {
    // 00AA3AD4 CMP byte [0109CF04+0Dh]: set widens the X range past the unit box.
    return wide_screen ? kGuiPointerBoundsWide : kGuiPointerBoundsNarrow;
}

float clamp_pointer_004155b0(float value, GuiPointerBounds bounds) noexcept {
    if (value < bounds.lo) {
        return bounds.lo;
    }
    if (value > bounds.hi) {
        return bounds.hi;
    }
    return value;
}

float clamp_pointer_y_00aa3910(float value) noexcept {
    // 00AA3B31 COMISS against 0.01f, then 00AA3B4B FCOMIP against the double
    // 0.99. Both bounds are the narrow pair regardless of the widescreen flag.
    if (!(kGuiPointerBoundsNarrow.lo <= value)) {
        return kGuiPointerBoundsNarrow.lo;
    }
    if (static_cast<double>(value) > static_cast<double>(kGuiPointerBoundsNarrow.hi)) {
        return kGuiPointerBoundsNarrow.hi;
    }
    return value;
}

bool wakes_pointer_00aa3910(bool icon_visible, float delta_length) noexcept {
    // 00AA3AA1 TEST AL,AL then 00AA3AB4 FCOMIP: strictly greater than 0.01f.
    return !icon_visible && delta_length > kGuiPointerWakeLength;
}

void advance_pointer_00aa3910(
    GuiPointerState& state, GuiPointerLatch& latch,
    std::int32_t device_x, std::int32_t device_y, bool wide_screen) noexcept {
    const float raw_x = static_cast<float>(device_x);
    const float raw_y = static_cast<float>(device_y);

    if (!latch.primed) {
        // 00AA3985: bit 0 of 00F8BC6C. The first call only latches; the frame
        // contributes no motion.
        latch.primed = true;
        latch.x = raw_x;
        latch.y = raw_y;
    }

    const float delta_x = raw_x - latch.x;  // 00AA39D9
    const float delta_y = raw_y - latch.y;  // 00AA39F5
    latch.x = raw_x;                        // 00AA3A26
    latch.y = raw_y;                        // 00AA3A2E

    state.delta_x = scale_pointer_delta_00aa3910(delta_x);  // +64h
    state.delta_y = scale_pointer_delta_00aa3910(delta_y);  // +68h
    state.x += state.delta_x;                               // 00AA3A8C, +5Ch
    state.y += state.delta_y;                               // 00AA3A94, +60h

    state.x = clamp_pointer_004155b0(state.x, pointer_x_bounds_00aa3910(wide_screen));
    state.y = clamp_pointer_y_00aa3910(state.y);
}

// ---------------------------------------------------------------------------
// Hit testing
// ---------------------------------------------------------------------------

bool page_is_hit_tested_00aa2f10(
    bool page_visible, const void* page, const void* exclusive_page) noexcept {
    if (!page_visible) {
        return false;  // 00AA2FAF TEST AL,AL
    }
    if (exclusive_page == nullptr) {
        return true;  // 00AA2FBB: no filter installed
    }
    return page == exclusive_page;  // 00AA2FCA CMP / JNZ
}

bool hit_candidate_wins_00aa8bd0(float half_extent_x, float best_score) noexcept {
    return half_extent_x < best_score;  // 00AA8E0C, strict: ties keep the first
}

bool hit_limits_allow_00aa8bd0(const GuiHitBox& box, const GuiHitLimits& limits) noexcept {
    if (limits.gate_right != 0.0F && limits.gate_right > box.right) {
        return false;  // 00AA8CD3: the outer gate, skips the self-test
    }
    if (limits.min_left != 0.0F && limits.min_left < box.left) {
        return false;  // 00AA8CE7
    }
    if (limits.min_bottom != 0.0F && box.bottom < limits.min_bottom) {
        return false;  // 00AA8CFB
    }
    if (limits.min_top != 0.0F && limits.min_top < box.top) {
        return false;  // 00AA8D0F
    }
    return true;
}

bool hit_box_contains_00aa8bd0(
    const GuiHitBox& box, float pointer_x, float pointer_y) noexcept {
    if (pointer_x < box.left) {
        return false;  // 00AA8D28, its own early return
    }
    return pointer_x <= box.right && box.top <= pointer_y && pointer_y <= box.bottom;
}

void hit_test_widget_00aa8bd0(
    const GuiHitNode& widget, const float origin[3], float pointer_x,
    float pointer_y, GuiHitResult& result) noexcept {
    const float half_x = widget.size[0] * widget.scale[0];  // 00AA8BDC
    const float half_y = widget.size[1] * widget.scale[1];  // 00AA8BEA

    const float abs_x = widget.pos[0] + origin[0];  // 00AA8BF4
    const float abs_y = widget.pos[1] + origin[1];  // 00AA8BFD
    const float abs_z = widget.pos[2] + origin[2];  // 00AA8C07

    const float child_origin[3] = {
        abs_x - half_x,  // 00AA8C14
        abs_y - half_y,  // 00AA8C20
        static_cast<float>(static_cast<double>(abs_z) - kGuiHitChildZBias),  // 00AA8C2C
    };

    // 00AA8C3A..00AA8C7B: children first, in list order, visible ones only. The
    // parent is tested afterwards, so a parent can still win when no child does.
    for (const GuiHitNode& child : widget.children) {
        if (child.visible) {
            hit_test_widget_00aa8bd0(child, child_origin, pointer_x, pointer_y, result);
        }
    }

    // 00AA8CA0: MouseHit at +78h set and the byte at +77h clear.
    if (!widget.mouse_hit || widget.suppressed) {
        return;
    }
    if (!hit_limits_allow_00aa8bd0(widget.box, widget.limits)) {
        return;
    }
    if (!hit_box_contains_00aa8bd0(widget.box, pointer_x, pointer_y)) {
        return;
    }
    if (hit_candidate_wins_00aa8bd0(half_x, result.score)) {
        result.widget = &widget;  // 00AA8DEA, 00F8BC70
        result.score = half_x;    // 00AA8DF0, 00F8BC7C
    }
}

GuiHitResult hit_test_pages_00aa2f10(
    const std::vector<const GuiHitNode*>& pages, float pointer_x, float pointer_y,
    const GuiHitNode* exclusive_page) noexcept {
    GuiHitResult result{};  // 00AA2F46 clears 00F8BC70, 00AA2F4C seeds 1e10f
    const float origin[3] = {0.0F, 0.0F, 0.0F};  // 00AA2FC1 stages three zeroes
    for (const GuiHitNode* page : pages) {
        if (page == nullptr) {
            continue;
        }
        if (!page_is_hit_tested_00aa2f10(page->visible, page, exclusive_page)) {
            continue;
        }
        hit_test_widget_00aa8bd0(*page, origin, pointer_x, pointer_y, result);
    }
    return result;
}

std::int32_t pointer_icon_state_00aa3910(
    const GuiHitNode* hit_widget, bool hit_widget_byte_84) noexcept {
    // 00AA3B83..00AA3B9C: 1 when something was hit and its byte at +84h is clear.
    if (hit_widget != nullptr && !hit_widget_byte_84) {
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Teardown
// ---------------------------------------------------------------------------

GuiLayerTeardownPath teardown_path_00ac5480(const GuiLayerImage& layer) noexcept {
    // 00AC549F CMP byte +0F4h,0.
    return layer.owns_store ? GuiLayerTeardownPath::DestroyOwnedStore
                            : GuiLayerTeardownPath::ReleaseSharedScene;
}

bool frees_block_00aa38f0(std::uint8_t deleting_flags) noexcept {
    return (deleting_flags & 1U) != 0U;  // 00AA38F8 TEST byte [ESP+8],1
}

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

bool create_layer_for_page(
    GuiLayerHost& host, GuiLayerImage& layer, std::string_view name,
    bool share_scene) {
    layer = construct_from_script_00ac6600(name, share_scene);

    // 00AC6744..00AC67A5: the `_Common` prelude, then the page's own script.
    host.run_common_script_00b69d40(kGuiCommonScriptName);
    if (!host.run_page_script_00b69d40(name)) {
        return false;
    }

    // 00AC6806 takes the `GuiScreen` global, 00AC6814 calls vtable +18h with it.
    const std::int32_t priority =
        host.read_priority_00ac4c50(kGuiLayerPriorityDefault);
    const float render_order =
        host.read_render_order_00ac4c50(kGuiLayerRenderOrderDefault);
    apply_read_properties_00ac4c50(layer, priority, render_order);

    // 00AC4C9E: vtable +78h, which is 00AC59A0.
    GuiCameraStore* const found = host.find_camera_store_00aa3280(layer.key);
    if (reuses_store_00ac59a0(layer.share_scene, found != nullptr)) {
        layer.store = found;
        layer.owns_store = false;   // 00AC59D9
        layer.scene = found->scene;  // store +1Ch
        host.retain_scene_00ce221c(layer.scene);
    } else {
        layer.owns_store = true;  // 00AC59BE
        layer.scene = host.create_scene_00b724e0(layer.name);
        const void* const camera =
            host.create_camera_00b71a80(std::string(kGuiCameraNamePrefix) + layer.name);
        layer.store =
            host.create_camera_store_00aa5070(camera, layer.scene, layer.key);
        host.add_scene_lights_00b83c50(layer.scene);
        host.set_scene_flags_00b6fe10(layer.scene, scene_flags_00ac59a0(layer.key.flags));
    }

    if (needs_reregistration_00ac59a0(layer)) {
        adopt_authored_priority_00ac59a0(layer);
        host.register_page_00aa52a0(layer);
    }
    return true;
}

bool update_layer(GuiLayerHost& host, GuiLayerImage& layer, float seconds) {
    if (!updates_this_frame_00aa87b0(
            layer.visible, host.page_has_live_entries_00aa7ef0(layer))) {
        return false;
    }
    host.update_page_00aa87b0(layer, seconds);
    return true;
}

void destroy_layer(GuiLayerHost& host, GuiLayerImage& layer, std::uint8_t deleting_flags) {
    switch (teardown_path_00ac5480(layer)) {
        case GuiLayerTeardownPath::ReleaseSharedScene:
            // 00AC54A9: InterlockedDecrement on scene+4h, then the scene's own
            // vtable slot 0 at zero, and +0ECh is nulled.
            host.release_scene_00ce2220(layer.scene);
            layer.scene = nullptr;
            break;
        case GuiLayerTeardownPath::DestroyOwnedStore:
            // 00AC54DA: the manager drops the store, then the scene is destroyed.
            host.remove_camera_store_00aa4b30(layer.store);
            if (layer.scene != nullptr) {
                host.destroy_scene_00b72250(layer.scene);
                layer.scene = nullptr;
            }
            break;
    }
    layer.store = nullptr;

    if (!layer.name.empty()) {
        host.free_name_buffer_00bd1510(layer.name);  // 00AC5510
        layer.name.clear();
    }
    host.destruct_widget_base_00aa9730(layer);  // 00AC5525

    // 00AA38F8: the pool free is the caller's business, not the destructor's.
    static_cast<void>(frees_block_00aa38f0(deleting_flags));
}

}  // namespace bsp
