#include "bsp/gui_render_order.hpp"

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
    placement.eye[2] = 0.0F;  // 00AA3E79 stores +0.0f
    placement.target[0] = placement.eye[0];
    placement.target[1] = placement.eye[1];
    placement.target[2] = kGuiCameraTargetZ;
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
    std::sort(snapshot.begin(), snapshot.end(),
        [](const GuiSnapshotChild& lhs, const GuiSnapshotChild& rhs) noexcept {
            return widget_name_less_00aa2c80(lhs.name, rhs.name);
        });
    return snapshot;
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
