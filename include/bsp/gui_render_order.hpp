#pragma once
// GUI draw order: the `RenderOrder` key, the camera-store map the GUI manager
// draws from, the orthographic camera each pass gets, and the alphabetical child
// snapshot that is NOT part of the draw path.
// Addresses: 00aa45a0, 00aa3e00, 00aa5070, 00aa3280, 00aa4960, 00aa2020,
// 00ac4c50, 00ac4450, 00ac59a0, 00ac6040, 00aa5a00, 00aa4e00, 00aa2c80,
// 00aa7f70.
// Supporting addresses read but not owned: 004ca440 (BSP_Game_Render), 004c12b0
// (BSP_GuiManager_GetOrCreate), 00aa52a0 (BSP_GuiManager_RegisterPageByPriority),
// 00aaaed0 (BSP_GuiWidget_DescribeProperties), 00aaa480 (the second std::sort
// instantiation), 00b6d890 (node -> scene bind), 00b63f10 (look-at view),
// 00b71490 (camera view set), 00b62a10 (ortho matrix), 00b6fd60
// (BSP_Camera_SetProjectionMatrix), 004c11f0 (BSP_RenderCommandQueue_GetSingleton),
// 00b1f4d0 (the queue submit), 004bda70 (_Tree_iterator::operator++).
// Every name below is a hypothesis, not a recovered symbol. Evidence is in
// docs/GUI_RENDER_ORDER.md. Nothing here is binary compatible with the original:
// the native camera store is a 24h-byte heap block held by value in an MSVC
// std::multimap node, and the camera and scene are engine objects this packet
// only names.
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// The authored keys
// ---------------------------------------------------------------------------

// `cGuiLayer::ReadProperties` (00AC4C50) reads exactly two ordering properties
// through the visitor's vtable +0Ch. Both use the 8-byte {tag, value} blocks of
// docs/GUI_LAYOUT_LOADER.md; tag 1 is a signed int and is not yet in
// GuiValueTag; gui_text.hpp already names it kGuiValueTagInt. Tag 2 is the float
// GuiValueTag::Float names.
inline constexpr std::string_view kGuiLayerPriorityKey = "Priority";        // 00D5CB00
inline constexpr std::string_view kGuiLayerRenderOrderKey = "RenderOrder";  // 00D5CAF4
inline constexpr std::int32_t kGuiLayerPriorityDefault = 0;      // 00AC4C5E stages 0
inline constexpr float kGuiLayerRenderOrderDefault = 0.0F;       // 00AC4CAB stages +0.0f

// `geOrder`, which appears on almost every widget in the shipped interface
// scripts, has no matching literal anywhere in the image: a byte scan for
// "geOrder" over the whole file finds nothing. It is authoring metadata the
// game never reads, and it is not the draw order.

// The camera-store descriptor at layer+108h, copied verbatim into the store by
// 00AA5070 (00AA50C7..00AA50F6). Defaults are the block 00AC6040 writes at
// layer+108h..+11Ch before 00AC59A0 runs.
struct GuiCameraStoreKey {
    std::int32_t flags{0};       // +108h, OR'ed with 6 for the scene at 00AC5EF6
    float near_plane{0.1F};      // +10Ch, 00D7A2F0
    float far_plane{1000.0F};    // +110h, 00CE3804
    float scale{1.0F};           // +114h, 00D7A24C
    std::int32_t priority{0};    // +118h, the authored Priority; NOT matched
    float render_order{0.0F};    // +11Ch, the authored RenderOrder; the map key
};

// 00AA3280's match: flags compared as an integer, then +4h, +8h, +0Ch and +14h
// compared as floats. +10h (Priority) is deliberately skipped, so two layers
// that differ only in Priority share one camera, one scene and one draw pass.
bool camera_store_key_matches_00aa3280(
    const GuiCameraStoreKey& stored, const GuiCameraStoreKey& wanted) noexcept;

// ---------------------------------------------------------------------------
// The ordered container
// ---------------------------------------------------------------------------

// The GUI manager holds an MSVC std::multimap<float, cGuiCameraStore*> at
// manager+8h (_Myhead at +0Ch). 00AA4960 is its _Tree::insert: it descends left
// only when the new key is strictly less than the node key (00AA4991 FCOMIP /
// 00AA4993 JBE), so equal keys always land to the right of the ones already
// there. Ordering is therefore ascending RenderOrder, ties in insertion order.
bool store_insert_goes_left_00aa4960(float new_key, float node_key) noexcept;

// The value half of the map node (_Myval.second at node+10h).
struct GuiCameraStore {
    GuiCameraStoreKey key{};
    const void* camera{nullptr};    // +18h, the "GuiCam_<layer name>" camera
    const void* scene{nullptr};     // +1Ch, the scene every layer widget hangs under
    std::int32_t visible_layers{0}; // +20h, maintained by 00AC4450 / 00AC6040
};

// Drawn only when the store has at least one visible layer (00AA45FA CMP / JLE).
bool store_is_drawn_00aa45a0(const GuiCameraStore& store) noexcept;

// The manager's map, reproduced as a flat vector so the iteration order is the
// map's own. Not the native layout: the native is a red-black tree whose nodes
// hold {float, cGuiCameraStore*}.
class GuiCameraStoreMap {
  public:
    using Entry = std::pair<float, GuiCameraStore*>;

    // 00AA5070's tail: build the pair {descriptor RenderOrder, store} and insert
    // it. Equal keys keep insertion order.
    void insert_00aa5070(GuiCameraStore& store);

    // 00AA3280: the first entry, in map order, whose descriptor matches.
    // Returns nullptr when there is none, as the native returns 0.
    GuiCameraStore* find_00aa3280(const GuiCameraStoreKey& wanted) const noexcept;

    const std::vector<Entry>& entries() const noexcept { return entries_; }
    std::size_t size() const noexcept { return entries_.size(); }

  private:
    std::vector<Entry> entries_{};
};

// ---------------------------------------------------------------------------
// The orthographic GUI camera
// ---------------------------------------------------------------------------

// The six floats 00AA3E00 pushes to 00AA2020 (00AA3EE4..00AA3F01, in stack
// order). Y runs downwards: bottom is +0.375f and top is -0.375f, which is why
// authored `Pos` Y grows towards the bottom of the screen. 00AA2020 scales the
// four side planes by the widescreen factors at 00D5BD98/00D5BD9C, 00CF5750 and
// platform+10h before handing them to 00B62A10; that scaling is a host call.
struct GuiOrthoVolume {
    float left{-0.5F};        // 00CE69D0
    float right{0.5F};        // 00CE3800
    float bottom{0.375F};     // 00D5BF4C
    float top{-0.375F};       // 00D5BF50
    float near_plane{1.0F};   // FLD1
    float far_plane{19984.0F};// 00CE3CC0
};
inline constexpr GuiOrthoVolume kGuiOrthoVolume{};

// The look-at 00AA3E00 builds for 00B63F10. The eye sits half a back-buffer
// pixel past the centre of the unit volume, which is the Direct3D 9 half-texel
// correction for pixel-exact 2D.
struct GuiCameraPlacement {
    float eye[3]{};
    float target[3]{};
    float up[3]{0.0F, 1.0F, 0.0F}; // 00AA3E6B..00AA3E74 stage (0, 1, 0)
};
inline constexpr float kGuiCameraTargetZ = -9992.0F; // 00D19620

// 00AA3E30..00AA3E93: eye = (0.5 + 0.5/width, 0.375 + 0.5/height, 0),
// target = (eye.x, eye.y, -9992). The native divides in x87 extended precision
// and rounds each sum to float; this uses double and rounds once, which agrees
// for every back-buffer size the game can set.
GuiCameraPlacement gui_camera_placement_00aa3e00(
    std::int32_t back_buffer_width, std::int32_t back_buffer_height) noexcept;

// ---------------------------------------------------------------------------
// The alphabetical child snapshot (NOT the draw order)
// ---------------------------------------------------------------------------

// 00AA5A00 copies the payload of every node of the widget's child list at +68h
// into a fresh std::vector and sorts it with the std::sort instantiation
// 00AA4E00. Its sole caller is 00AAAED0, the reflection descriptor walk, which
// sorts the same vector a second time with the duplicate instantiation 00AAA480
// and then visits each child. Nothing in the draw path reads it, and it is
// rebuilt from scratch on every call: there is no dirty flag and no cached
// count. The predicate lives in the insertion-sort halves 00AA2C80 and 00AA7F70,
// which are byte-identical: it reads each widget's name through vtable +28h
// (a {length, char*} pair) and compares with _stricmp, treating a zero length as
// least so an empty name never dereferences its pointer.
int compare_widget_names_00aa2c80(std::string_view lhs, std::string_view rhs) noexcept;
bool widget_name_less_00aa2c80(std::string_view lhs, std::string_view rhs) noexcept;

// One child as the snapshot sees it: the payload pointer plus the name vtable
// +28h returns for it.
struct GuiSnapshotChild {
    const void* widget{nullptr};
    std::string name{};
};

// 00AA5A00, __fastcall(ECX = out vector, EDX = widget), RET 0. `children` is the
// widget's +68h list in list order; the result is the sorted snapshot. std::sort
// is not stable, so two children whose names compare equal have an unspecified
// relative order; below the 32-element insertion-sort threshold of 00AA4E00 the
// observed order is the list order.
std::vector<GuiSnapshotChild> collect_sorted_children_00aa5a00(
    const std::vector<GuiSnapshotChild>& children);

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

struct GuiBackBufferSize {
    std::int32_t width{0};
    std::int32_t height{0};
};

// One method per native call site the GUI draw leaves through. There are no
// default implementations: nothing here stands in for unrecovered behaviour.
struct GuiRenderOrderHost {
    virtual ~GuiRenderOrderHost() = default;

    // 00AA45A7..00AA45BB: when manager+40h is non-null, its vtable +40h is
    // called before the walk. The object at +40h was not identified.
    virtual bool has_pre_draw_object() = 0;
    virtual void pre_draw_virtual40() = 0;

    // 00AA3E18..00AA3E2E: the device singleton at 00F8D394, vtable +80h, fills a
    // two-int back-buffer size in the caller's frame.
    virtual GuiBackBufferSize back_buffer_size_00f8d394_virtual80() = 0;

    // 00AA3EB1 then 00AA3EC0: 00B63F10 builds the look-at basis, 00B71490 sets
    // it on the camera (ECX = camera).
    virtual void set_camera_look_at_00b71490(
        const void* camera, const GuiCameraPlacement& placement) = 0;

    // 00AA3F01: 00AA2020, __thiscall(ECX = camera, six floats), RET 18h. It
    // applies the widescreen scale and calls BSP_Camera_SetProjectionMatrix.
    virtual void set_camera_ortho_00aa2020(
        const void* camera, const GuiOrthoVolume& volume) = 0;

    // 00AA3F12: BSP_NativeString_Assign of the literal at 00CE9A38 into an
    // 8-byte temporary. The literal is the single character "X"; what the queue
    // does with it is a queue contract, not a GUI one.
    virtual void set_pass_label_0041e870(std::string_view label) = 0;

    // 00AA3F2D: BSP_RenderCommandQueue_GetSingleton.
    virtual void* render_command_queue_004c11f0() = 0;

    // 00AA3F36: 00B0D0D0 on the object at 00F8D39C, result passed straight
    // through as the queue submit's third argument.
    virtual void* render_pass_context_00b0d0d0() = 0;

    // 00AA3F52: 00B1F4D0, __thiscall(ECX = queue, scene, camera, context).
    virtual void submit_scene_00b1f4d0(
        void* queue, const void* scene, const void* camera, void* context) = 0;
};

// 00AA3E00, __thiscall(ECX = manager, scene, camera), RET 8. ECX is loaded over
// immediately and never read, so the manager is not an input. Camera setup then
// one queue submit.
void submit_gui_pass_00aa3e00(
    const void* scene, const void* camera, GuiRenderOrderHost& host);

// 00AA45A0, __thiscall(ECX = manager), RET 0, no return value. The GUI draw that
// BSP_Game_Render reaches at 004CA658 through BSP_GuiManager_GetOrCreate: the
// optional pre-draw virtual, then the camera-store map walked in ascending
// RenderOrder, skipping any store with no visible layer. Returns the number of
// passes submitted so a caller can see the walk without observing the host.
std::size_t draw_gui_layers_00aa45a0(
    const GuiCameraStoreMap& stores, GuiRenderOrderHost& host);

}  // namespace bsp
