#include "bsp/gui_render_order.hpp"
#include "bsp/gui_widgets_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <algorithm>

namespace bsp {
namespace {

// _stricmp in the C locale: unsigned char comparison after an ASCII fold. The
// CRT folds through the current locale's lowercase table, so a non-ASCII widget
// name could order differently in the game; no shipped interface script uses one.
char fold(char c) noexcept
{
    const unsigned char u = static_cast<unsigned char>(c);
    if (u >= 'A' && u <= 'Z') {
        return static_cast<char>(u - 'A' + 'a');
    }
    return static_cast<char>(u);
}

}  // namespace

bool camera_store_key_matches_00aa3280(
    const GuiCameraStoreKey& stored, const GuiCameraStoreKey& wanted) noexcept
{
    // 00AA32D5..00AA3306, in the native's own order. `priority` is absent on
    // purpose; the native never compares +10h.
    return stored.flags == wanted.flags && stored.near_plane == wanted.near_plane
        && stored.far_plane == wanted.far_plane && stored.scale == wanted.scale
        && stored.render_order == wanted.render_order;
}

bool store_insert_goes_left_00aa4960(float new_key, float node_key) noexcept
{
    // 00AA4991 FCOMIP compares the node key against the new key and 00AA4993
    // takes the right branch on "below or equal", so left is the strict case.
    return new_key < node_key;
}

bool store_is_drawn_00aa45a0(const GuiCameraStore& store) noexcept
{
    return store.visible_layers > 0;  // 00AA45FA CMP dword [ECX+20h], 0 / JLE
}

void GuiCameraStoreMap::insert_00aa5070(GuiCameraStore& store)
{
    const float key = store.key.render_order;
    // Equal keys go to the right of the ones already present, which is where
    // upper_bound puts them.
    const auto at = std::upper_bound(entries_.begin(), entries_.end(), key,
        [](float lhs, const Entry& rhs) noexcept { return lhs < rhs.first; });
    entries_.insert(at, Entry{key, &store});
}

GuiCameraStore* GuiCameraStoreMap::find_00aa3280(
    const GuiCameraStoreKey& wanted) const noexcept
{
    for (const Entry& entry : entries_) {
        if (camera_store_key_matches_00aa3280(entry.second->key, wanted)) {
            return entry.second;
        }
    }
    return nullptr;
}

bool GuiCameraStoreMap::remove_owned_00aa4b30(GuiCameraStore* store) noexcept
{
    const auto at = std::find_if(entries_.begin(), entries_.end(),
        [store](const Entry& entry) { return entry.second == store; });
    if (at == entries_.end()) return false;
    if (at->second) {
        at->second->~GuiCameraStore();
        singleton_lifetime_free(at->second);
        at->second = nullptr;
    }
    entries_.erase(at);
    return true;
}

GuiCameraPlacement gui_camera_placement_00aa3e00(
    std::int32_t back_buffer_width, std::int32_t back_buffer_height) noexcept
{
    GuiCameraPlacement placement{};
    const double half_x = 0.5 + (back_buffer_width != 0
            ? 0.5 / static_cast<double>(back_buffer_width)
            : 0.0);
    const double half_y = 0.375 + (back_buffer_height != 0
            ? 0.5 / static_cast<double>(back_buffer_height)
            : 0.0);
    placement.eye[0] = static_cast<float>(half_x);
    placement.eye[1] = static_cast<float>(half_y);
    placement.eye[2] = kGuiCameraEyeZ; // 00AA3E87 stores eye+8, not target+8.
    placement.target[0] = placement.eye[0];
    placement.target[1] = placement.eye[1];
    placement.target[2] = 0.0F; // 00AA3E79 stores target+8.
    placement.up[0] = 0.0F;
    placement.up[1] = 1.0F;
    placement.up[2] = 0.0F;
    return placement;
}

int compare_widget_names_00aa2c80(std::string_view lhs, std::string_view rhs) noexcept
{
    // 00AA2CB4..00AA2D04. `lhs` is the element being placed, `rhs` the one it is
    // compared against, which is the argument order the native uses for _stricmp.
    if (lhs.empty()) {
        return rhs.empty() ? 0 : -1;
    }
    if (rhs.empty()) {
        return 1;
    }
    const std::size_t common = std::min(lhs.size(), rhs.size());
    for (std::size_t i = 0; i < common; ++i) {
        const unsigned char a = static_cast<unsigned char>(fold(lhs[i]));
        const unsigned char b = static_cast<unsigned char>(fold(rhs[i]));
        if (a != b) {
            return a < b ? -1 : 1;
        }
    }
    if (lhs.size() == rhs.size()) {
        return 0;
    }
    return lhs.size() < rhs.size() ? -1 : 1;
}

bool widget_name_less_00aa2c80(std::string_view lhs, std::string_view rhs) noexcept
{
    return compare_widget_names_00aa2c80(lhs, rhs) < 0;
}

std::vector<GuiSnapshotChild> collect_sorted_children_00aa5a00(
    const std::vector<GuiSnapshotChild>& children)
{
    std::vector<GuiSnapshotChild> snapshot(children);
    // 00AA5AF7..00AA5AFD: PUSH predicate byte; PUSH count (_Ideal); ECX = begin,
    // EDX = end; CALL 00AA4E00. The image's
    // introsort (msvc9_sort) is not std::sort; using it reproduces the order of
    // equal names. Swapping a {widget, name} pair permutes exactly as swapping
    // the image's widget pointers, since the name is a function of the widget.
    auto pred = [](const GuiSnapshotChild& lhs, const GuiSnapshotChild& rhs) noexcept {
        return widget_name_less_00aa2c80(lhs.name, rhs.name);
    };
    if (!snapshot.empty()) {
        GuiSnapshotChild* const first = snapshot.data();
        GuiSnapshotChild* const last = first + snapshot.size();
        gui_widgets_cc13::msvc9_sort::sort(first, last, last - first, pred);
    }
    return snapshot;
}

void set_gui_ortho_projection_00aa2020(const void* camera, const GuiOrthoVolume& volume,
    GuiOrthoProjectionHost& host)
{
    const float base = host.platform_widescreen_0d() ? 1.7777778F : 1.3333334F; // 00D5BD9C / 00D5BD98
    const float sx = static_cast<float>(static_cast<double>(base) / 1.3333333730697632); // 00CF5750
    const float sy = static_cast<float>(static_cast<double>(base) / host.platform_aspect_10());
    const float top = sy * volume.top;
    const float bottom = sy * volume.bottom;
    const float right = sx * volume.right;
    const float left = sx * volume.left;
    void* const matrix = host.ortho_off_center_00b62a10(
        left, right, bottom, top, volume.near_plane, volume.far_plane);
    host.set_projection_00b6fd60(camera, matrix);
    const std::array<std::uint32_t, 2> size = host.device_size_00f8d394_30();
    host.viewport_set_dimensions_00b1f940(host.camera_viewport_00b6fde0(camera, size));
}
void prepare_and_start_render_worker_00aa4040(const void* descriptor, void* callback, void* context,
    std::uint32_t rate, GuiRenderWorkerHost& host)
{
    host.queue_set_mode_00b1c460(1);
    host.submit_gui_pass_00aa3e00(host.descriptor_scene_ec(descriptor), host.descriptor_camera_f0_18(descriptor));
    host.queue_set_mode_00b1c460(2);
    host.queue_start_worker_00b1c4a0(callback, context, rate);
}
void submit_gui_pass_00aa3e00(
    const void* scene, const void* camera, GuiRenderOrderHost& host)
{
    const GuiBackBufferSize size = host.back_buffer_size_00f8d394_virtual80();
    const GuiCameraPlacement placement
        = gui_camera_placement_00aa3e00(size.width, size.height);
    host.set_camera_look_at_00b71490(camera, placement);
    host.set_camera_ortho_00aa2020(camera, kGuiOrthoVolume);
    host.set_pass_label_0041e870("X");  // 00CE9A38
    void* const queue = host.render_command_queue_004c11f0();
    void* const context = host.render_pass_context_00b0d0d0();
    host.submit_scene_00b1f4d0(queue, scene, camera, context);
}

std::size_t draw_gui_layers_00aa45a0(
    const GuiCameraStoreMap& stores, GuiRenderOrderHost& host)
{
    if (host.has_pre_draw_object()) {
        host.pre_draw_virtual40();
    }
    std::size_t submitted = 0;
    for (const GuiCameraStoreMap::Entry& entry : stores.entries()) {
        GuiCameraStore* const store = entry.second;
        if (store == nullptr || !store_is_drawn_00aa45a0(*store)) {
            continue;
        }
        submit_gui_pass_00aa3e00(store->scene, store->camera, host);
        ++submitted;
    }
    return submitted;
}

}  // namespace bsp
