#include "bsp/gui_widget_owner.hpp"
#include "bsp/gui_widget_copy.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include "bsp/gui_timed_entry_owner.hpp"
#include "bsp/gui_widget_clip_refresh.hpp"
#include "bsp/gui_widget_color_dispatch.hpp"
#include "bsp/gui_widget_frame_runtime.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
std::uint32_t identity(NativeNodeBinding* node) noexcept {
    static_assert(sizeof(void*) == 4, "GUI runtime requires MSVC Win32");
    return node ? reinterpret_cast<std::uint32_t>(&node->storage) : 0;
}
CameraMatrix unit_matrix() {
    CameraMatrix result{};
    result[0] = result[5] = result[10] = result[15] = 1.0f;
    return result;
}
}

void build_gui_rotation_z_00b64780(CameraMatrix& destination, const float& angle) {
    float sine, cosine;
    const float* input = &angle;
    // Separate input reloads and float32 spills match00B6478E..00B647A7.
    __asm {
        mov eax, input
        fld dword ptr [eax]
        fsin
        fstp sine
        fld dword ptr [eax]
        fcos
        fstp cosine
    }
    destination.fill(0.0f);
    destination[0] = destination[5] = cosine;
    destination[1] = sine;
    destination[4] = -0.0f - sine;
    destination[10] = destination[15] = 1.0f;
}
void* gui_model_geometry_00b74640(const NativeModelTailStorage& model, std::uint32_t) noexcept {
    return model.geometry_180;
}
void* gui_geometry_element_00b732c0(const void* geometry, std::int32_t index) noexcept {
    void** elements;
    std::memcpy(&elements, static_cast<const std::byte*>(geometry) + 0x54, sizeof(elements));
    return elements[index];
}
void set_gui_element_bounds_00b855b0(void* element, const GuiWidgetBounds& bounds) noexcept {
    static_assert(sizeof(GuiWidgetBounds) == 16);
    const auto* source = &bounds;
    __asm {
        mov eax, source
        mov ecx, element
        fld dword ptr [eax]
        fstp dword ptr [ecx + 24h]
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx + 28h]
        fld dword ptr [eax + 8]
        fstp dword ptr [ecx + 2ch]
        fld dword ptr [eax + 0ch]
        fstp dword ptr [ecx + 30h]
    }
}

struct GuiWidgetOwnerRuntime::ModelRecord {
    std::unique_ptr<NativeModelOwner> owner;
    std::unique_ptr<NativeModelReference> reference;
};

GuiWidgetOwner::GuiWidgetOwner(GuiLayoutWidget& layout, GuiWidgetOwnerRuntime& runtime)
    : layout_(layout), runtime_(runtime) {
    if (layout.parent || !layout.children.empty() || !layout.transform.children.empty())
        throw std::invalid_argument("widget construction requires an unattached fresh layout");
    auto& transform = layout.transform;
    //00AA9390 writes only the proven fields, preserving authored_x(+8).
    transform.position = {};
    transform.pivot_x = transform.pivot_y = 0.0f;
    transform.size = {};
    transform.scale_x = transform.scale_y = 1.0f;
    transform.rotate = 0.0f;
    transform.alpha = 1.0f;
    transform.type_id = static_cast<std::int32_t>(layout.type);
    transform.parent = nullptr;
    transform.bounds_enabled = transform.mouse_hit = transform.mouse_block = false;
    transform.widescreen_align = GuiWideScreenAlign::None;
    std::fill(std::begin(layout.color), std::end(layout.color), 1.0f);
    std::fill(std::begin(layout.low_color), std::end(layout.low_color), 0.0f);
    layout.low_color[3] = 1.0f;
    std::fill(std::begin(layout.high_color), std::end(layout.high_color), 1.0f);
    layout.blend_factor = 0.0f;
    layout.node_id = 0;
    //+85 and+E4 are NOT constructor stores. Keep the existing diagnostic
    // projection's preimage until a real setter/property visit initializes it.
    scene_.authored_visible = layout.visible;
}
GuiWidgetTypeImplementation& GuiWidgetOwner::implementation() {
    if (!implementation_) throw std::logic_error("GUI type implementation is not constructed");
    return *implementation_;
}
GuiWidgetOwner::GuiWidgetOwner(GuiLayoutWidget& layout, GuiWidgetOwnerRuntime& runtime,
    const GuiWidgetBaseCopyPreimage& preimage)
    : layout_(layout), runtime_(runtime), base_copy_untyped_(true) {
    // Only establish the host companion. AA9520's writes happen in native
    // order after registration; none of AA9390 or the derived default runs.
    std::memcpy(extra_.fields_7c_80, preimage.fields_7c_80_bits,
        sizeof(extra_.fields_7c_80));
    std::memcpy(&extra_.clip_enabled_e8, &preimage.clip_enabled_e8_bits, sizeof(float));
    scene_.active = preimage.active_85;
    scene_.authored_visible = layout.visible;
}
GuiWidgetOwner::~GuiWidgetOwner() noexcept = default;
GuiTimedEntryOwner& GuiWidgetOwner::timed_entries(const volatile float& one) {
    if (!timed_entries_) timed_entries_ = std::make_unique<GuiTimedEntryOwner>(*this, one);
    if (&timed_entries_->constructor_one() != &one)
        throw std::logic_error("widget timed entries require the original constant storage");
    return *timed_entries_;
}
void GuiWidgetOwner::require_timed_entry_ownership() const {
    if (timed_entries_) {
        if (timed_entries_->retired()) return; // cleanup phase already completed
        timed_entries_->validate_live();
        return;
    }
    for (auto word : extra_.pointers_88_90)
        if (word) throw std::logic_error("nonzero timed header has no actual allocation owner");
}
void GuiWidgetOwner::retire_timed_entries_00aa9730_fragment() {
    require_timed_entry_ownership();
    if (timed_entries_ && !timed_entries_->retired())
        timed_entries_->destroy_entries_00aa9730_fragment();
}
bool GuiWidgetOwner::has_pending_base_clip() const noexcept {
    return base_clip_ && base_clip_->has_pending();
}
void GuiWidgetOwner::require_no_active_owned_operation() const {
    if (base_lifetime_.phase != GuiWidgetBaseDeletionPhase::not_started || scene_release_active_ ||
        base_copy_active_ || (text_lifetime_ && text_lifetime_->has_incomplete_copy()) ||
        (implementation_ && implementation_->has_active_operation()) ||
        (runtime_.frame_runtime_ && runtime_.frame_runtime_->operation_active(*this)) ||
        has_pending_base_clip() || (timed_entries_ && timed_entries_->operation_active()))
        throw std::logic_error("widget still owns an active or pending native operation");
}
void GuiWidgetOwner::base_refresh_clip70_00aaa3e0() {
    auto* services = runtime_.environment().clip;
    if (!services || &services->widgets != &runtime_)
        throw std::logic_error("base clip70 requires the same actual configured services");
    if (!base_clip_) {
        base_clip_ = std::make_unique<GuiWidgetClipRefreshOperation>(*this, *services);
        base_clip_services_ = services;
    }
    if (base_clip_services_ != services)
        throw std::logic_error("base clip70 service storage changed during its owner lifetime");
    base_clip_->begin();
}
void GuiWidgetOwner::resume_base_clip_after_child70() {
    if (!base_clip_) throw std::logic_error("base clip70 has no pending child frame");
    auto* services = runtime_.environment().clip;
    if (!services || services != base_clip_services_ || &services->widgets != &runtime_)
        throw std::logic_error("base clip70 resume requires the original configured services");
    base_clip_->resume_after_child70();
}
void GuiWidgetTypeImplementation::refresh_clip70(GuiWidgetOwner& owner) {
    if (!gui_widget_uses_base_clip70_profile(owner.layout().type))
        throw std::logic_error("current GUI clip70 profile has no established implementation");
    owner.base_refresh_clip70_00aaa3e0();
}
float* GuiWidgetTypeImplementation::read_color54(GuiWidgetOwner& owner, float (&output)[4]) {
    if (!gui_widget_has_base_color54_profile(owner.layout().type))
        throw std::logic_error("current GUI color54 profile is not established");
    return read_gui_widget_color_00aa68f0(owner, output);
}
void GuiWidgetTypeImplementation::set_alpha4c(GuiWidgetOwner& owner, float alpha) {
    if (!gui_widget_has_base_alpha4c_profile(owner.layout().type))
        throw std::logic_error("current GUI alpha4C profile is not established");
    set_gui_widget_alpha_00aa6980(owner, alpha);
}
std::int32_t GuiWidgetTypeImplementation::type5c(GuiWidgetOwner& owner) {
    if (!gui_widget_has_base_color54_profile(owner.layout().type) ||
        owner.layout().transform.type_id != static_cast<std::int32_t>(owner.layout().type))
        throw std::logic_error("current GUI type5C profile is not established");
    return owner.layout().transform.type_id;
}
void GuiWidgetTypeImplementation::set_size58(GuiWidgetOwner& owner, const GuiWidgetSize& size) {
    // Verified Group D5CB80, Screen D5BE38 and Listbox D5BBF8 current58.
    // Section has its own ABE670; unsupported profiles must not use this base.
    const auto type = owner.layout().type;
    if (type != GuiWidgetType::Group && type != GuiWidgetType::Screen &&
        type != GuiWidgetType::Listbox)
        throw std::logic_error("Current GUI size58 profile has no established implementation");
    owner.base_set_size58_00aa7970(size);
}
void GuiWidgetOwner::refresh_clip70() {
    implementation().refresh_clip70(*this);
}
void GuiWidgetTypeImplementation::set_visible34(GuiWidgetOwner& owner, bool visible) {
    owner.set_visible_00aa8530(visible);
}
void GuiWidgetOwner::set_visible34(bool visible) {
    implementation().set_visible34(*this, visible);
}
void GuiWidgetOwner::propagate_visibility_00aa8450(const GuiWidgetVisibilityArgs& args) {
    runtime_.propagate_visibility(*this, args);
}
NativeModelReference* GuiWidgetOwner::model_reference() noexcept {
    const auto found = runtime_.models_.find(scene_.scene_node);
    return found == runtime_.models_.end() ? nullptr : found->second->reference.get();
}
void GuiWidgetOwner::bind_scene_00aa6720(NativeNodeBinding* node) noexcept {
    //Native binding does not retain/release either pointer.
    node_ = node;
    scene_.scene_node = node ? &node->storage : nullptr;
    layout_.node_id = identity(node);
    if (node) node->storage.auxiliary_flags_138 &= ~kGuiSceneNodeBindingClearMask;
}
void GuiWidgetOwner::base_constructed74_00a9ac00() noexcept {}
void GuiWidgetOwner::base_set_active60_00aa6a30(bool active) noexcept { scene_.active = active; }
void GuiWidgetOwner::base_visibility_changed3c_00a9e100(bool) noexcept {}
bool GuiWidgetOwner::base_is_visible38_00a9e0d0() const noexcept {
    if (!node_) return false;
    float factor;
    std::memcpy(&factor, &node_->storage.scalar_ac, sizeof(factor));
    return widget_is_visible(scene_, factor);
}
void GuiWidgetOwner::base_loaded78_00aa7170() {
    implementation().set_active60(*this, false);
    refresh_bounds_00aa70e0();
}
void GuiWidgetOwner::set_visible_00aa8530(bool visible) {
    bool ancestors_visible = true;
    for (auto* ancestor = layout_.transform.parent; ancestor && ancestors_visible;
         ancestor = ancestor->parent) {
        auto& retained = runtime_.owner(*ancestor);
        ancestors_visible = retained.implementation().is_visible38(retained);
    }
    runtime_.propagate_visibility(*this, {ancestors_visible, ancestors_visible,
        visible, scene_.visibility_recurses, true});
    //Reload after callbacks, matching00AA8576.
    if (node_) runtime_.stamp_visibility(*node_, visibility_factor_for(visible),
        scene_.visibility_recurses);
}
void GuiWidgetOwner::recompose_00aa7220() {
    //Native dereferences+4C before composition. Null is an explicit unsupported
    //call boundary, never a successful no-op transform update.
    if (!node_) throw std::logic_error("GUI transform requires its bound native node");
    const auto parts = local_transform(layout_.transform);
    auto translation = unit_matrix();
    translation[12] = parts.translation.x;
    translation[13] = parts.translation.y;
    translation[14] = parts.translation.z;
    auto scale = unit_matrix();
    scale[0] = parts.scale_x;
    scale[5] = parts.scale_y;
    auto pivot = unit_matrix();
    pivot[12] = parts.pivot_translation_x;
    pivot[13] = parts.pivot_translation_y;
    CameraMatrix rotation, first, second, final;
    build_gui_rotation_z_00b64780(rotation, parts.rotation_z);
    multiply_camera_matrices_00413920(first, pivot, scale);
    multiply_camera_matrices_00413920(second, first, rotation);
    multiply_camera_matrices_00413920(final, second, translation);
    //Children are B75030 models, whose current virtual38 is B6DB10. Root
    //binding's adapter must supply this same base transform contract.
    set_transform_local_matrix_00b6db10(node_->transform, final);
}
void GuiWidgetOwner::refresh_bounds_00aa70e0() {
    if (!layout_.transform.bounds_enabled) return;
    if (!node_) throw std::logic_error("GUI bounds require their bound native node");
    //Native code directly uses model+180 even for a dynamically derived node.
    //The binding's actual allocation must cover that field (all supported GUI
    //nodes do). Read its original storage, never a host geometry owner address.
    void* geometry;
    std::memcpy(&geometry, reinterpret_cast<const std::byte*>(&node_->storage) + 0x180,
        sizeof(geometry));
    if (!geometry) throw std::logic_error("GUI bounds require actual model geometry+180");
    void* element = gui_geometry_element_00b732c0(geometry, 0);
    if (!element) throw std::logic_error("GUI bounds require actual geometry element0");
    set_gui_element_bounds_00b855b0(element, local_bounds(layout_.transform));
}
void GuiWidgetOwner::set_position_00aa7dc0(const GuiWidgetPoint& position) {
    layout_.transform.position = position;
    recompose_00aa7220();
    refresh_bounds_00aa70e0();
}
void GuiWidgetOwner::base_set_size58_00aa7970(const GuiWidgetSize& size) {
    const auto* input = &size;
    auto* output = &layout_.transform.size;
    __asm {
        mov eax, input
        mov edx, output
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax + 4]
        fstp dword ptr [edx + 4]
    }
    recompose_00aa7220(); // Native base58 does not refresh bounds.
}
void GuiWidgetOwner::release_scene_nodes_00aa8320() {
    require_no_active_owned_operation();
    struct SceneCall {
        bool& active;
        explicit SceneCall(bool& value) : active(value) { active = true; }
        ~SceneCall() { active = false; }
    } scene_call(scene_release_active_);
    const auto child_count = layout_.children.size();
    for (std::size_t index = 0; index < child_count; ++index) {
        auto* child = layout_.children[index].get();
        if (!child) throw std::logic_error("current20 requires a live child");
        runtime_.owner(*child).release_scene_nodes_00aa8320();
        if (layout_.children.size() != child_count || layout_.children[index].get() != child)
            throw std::logic_error("current20 changed the borrowed host child collection");
    }
    implementation().release_secondary_scene_nodes(*this);
    if (node_) {
        auto& lifetime = runtime_.environment_.models.nodes.attachments.resolve(node_->transform);
        unlink_and_release_render_model_00b6dfa0(lifetime);
        node_ = nullptr;
        scene_.scene_node = nullptr;
        layout_.node_id = 0;
    }
}
void GuiWidgetOwner::update40(float seconds) {
    require_no_active_owned_operation();
    if (!runtime_.frame_runtime_)
        throw std::logic_error("current40 requires an actual registered frame runtime");
    runtime_.frame_runtime_->update40(*this, seconds);
}

GuiWidgetOwnerRuntime::GuiWidgetOwnerRuntime(GuiWidgetOwnerEnvironment environment)
    : environment_(std::move(environment)) {
    if (!environment_.make_type) throw std::invalid_argument("GUI runtime requires its actual type factory");
}
GuiWidgetOwnerRuntime::~GuiWidgetOwnerRuntime() {
    //Layouts/queued retained references must be retired by their real owners.
    if (frame_runtime_ || !widgets_.empty() || !models_.empty()) std::terminate();
}
void GuiWidgetOwnerRuntime::bind_frame_runtime(GuiWidgetFrameRuntime& frame) {
    if (&frame.widgets() != this || frame_runtime_)
        throw std::logic_error("widget runtime requires one original live frame service");
    frame_runtime_ = &frame;
}
void GuiWidgetOwnerRuntime::unbind_frame_runtime(GuiWidgetFrameRuntime& frame) noexcept {
    if (frame_runtime_ != &frame) std::terminate();
    frame_runtime_ = nullptr;
}
GuiWidgetOwner& GuiWidgetOwnerRuntime::construct_base(GuiLayoutWidget& layout) {
    if (widgets_.count(&layout) || layout.before_destroy)
        throw std::logic_error("GUI layout already has a retained owner");
    auto companion = std::unique_ptr<GuiWidgetOwner>(new GuiWidgetOwner(layout, *this));
    auto& result = *companion;
    widgets_.emplace(&layout, std::move(companion));
    try {
        layout.before_destroy = [this](GuiLayoutWidget& dying) { retire_tree(dying); };
        result.implementation_ = environment_.make_type(result);
        if (!result.implementation_) throw std::invalid_argument("unsupported GUI widget type");
    } catch (...) {
        layout.before_destroy = {};
        widgets_.erase(&layout);
        throw;
    }
    return result;
}
NativeNodeBinding* GuiWidgetOwnerRuntime::create_model(const std::string& name,
    NativeNodeBinding** publication) {
    auto& environment = environment_.models;
    auto record = std::make_unique<ModelRecord>();
    void* slot = environment.pool_01090054.allocate_raw_slot_00b74d00();
    if (!slot) {
        if (publication) *publication = nullptr;
        return nullptr;
    }
    // A broken allocator returning an occupied slot did not transfer ownership
    // of that slot. Do not erase its live record or return it in the unwind.
    if (models_.count(slot))
        throw std::logic_error("canonical model pool returned an occupied slot");
    PooledStringStorage strings(environment.nodes.strings);
    NativeString native_name;
    try {
        //Reserve the map node before constructing a native live object.
        models_.emplace(slot, nullptr);
        record->owner = std::make_unique<NativeModelOwner>(slot, NativeModelPool::slot_bytes, environment);
        native_name.assign_0041e870(strings, name.c_str());
        construct_native_model_00b75030(*record->owner, native_name);
        // The ordinary widget path keeps its existing cleanup point. Text's
        // auxiliary path publishes+188 before native temporary-name release.
        if (!publication) native_name.release_to(strings);
        //Insert before reference creation: its terminal retirement callback
        //must always resolve the one owning record.
        models_.at(slot) = std::move(record);
        auto& inserted = *models_.at(slot);
        try {
            inserted.reference = std::make_unique<NativeModelReference>(*inserted.owner,
                NativeModelCompanionDisposal{this, retire_model});
        } catch (...) {
            destroy_native_model_00b750c0(*inserted.owner);
            models_.erase(slot);
            throw;
        }
        auto* const created_node = &inserted.owner->node;
        if (publication) {
            *publication = created_node;
            native_name.release_to(strings);
        }
        // Name release can reenter through the published auxiliary slot.
        // Return the captured value without dereferencing a retired companion.
        return created_node;
    } catch (...) {
        native_name.release_to(strings);
        if (record && record->owner && record->owner->phase == NativeModelOwner::Phase::live)
            destroy_native_model_00b750c0(*record->owner);
        record.reset();
        models_.erase(slot);
        environment.pool_01090054.return_raw_slot_00b74750(slot);
        throw;
    }
}
void GuiWidgetOwnerRuntime::create_auxiliary_model_00ab8530_fragment(
    NativeNodeBinding*& publication, const std::string& name) {
    if (publication)
        throw std::logic_error("GUI auxiliary model publication must be empty");
    create_model(name, &publication);
}
NativeModelReference& GuiWidgetOwnerRuntime::create_model_clone_destination_00b752b0_fragment(
    NativeModelOwner& source) {
    auto& environment = environment_.models;
    const auto found = models_.find(&source.storage.node);
    if (&source.environment != &environment || source.phase != NativeModelOwner::Phase::live ||
        found == models_.end() || !found->second || found->second->owner.get() != &source ||
        !found->second->reference)
        throw std::logic_error("Model clone source must be this runtime's live canonical owner");
    auto record = std::make_unique<ModelRecord>();
    void* const slot = environment.pool_01090054.allocate_raw_slot_00b74d00();
    if (!slot) throw std::bad_alloc();
    if (models_.count(slot))
        throw std::logic_error("canonical Model pool returned an occupied clone slot");
    bool inserted = false;
    try {
        models_.emplace(slot, nullptr);
        inserted = true;
        record->owner = std::make_unique<NativeModelOwner>(slot, NativeModelPool::slot_bytes, environment);
        // B6D800 returns this SAME header only after allocation. B75030's
        // existing native-string copy preserves later callback-driven reloads.
        construct_native_model_00b75030(*record->owner,
            native_node_name_00b6d800(source.storage.node));
        models_.at(slot) = std::move(record);
        auto& retained = *models_.at(slot);
        retained.reference = std::make_unique<NativeModelReference>(*retained.owner,
            NativeModelCompanionDisposal{this, retire_model});
        return *retained.reference;
    } catch (...) {
        auto* active = record.get();
        if (!active && inserted) active = models_.at(slot).get();
        if (active && active->owner && active->owner->phase == NativeModelOwner::Phase::live)
            destroy_native_model_00b750c0(*active->owner);
        if (inserted) models_.erase(slot);
        record.reset();
        environment.pool_01090054.return_raw_slot_00b74750(slot);
        throw;
    }
}
void GuiWidgetOwnerRuntime::retire_model(void* context, NativeModelReference& reference) noexcept {
    auto& runtime = *static_cast<GuiWidgetOwnerRuntime*>(context);
    void* slot = &reference.model_owner().storage.node;
    runtime.models_.erase(slot);
}
void GuiWidgetOwnerRuntime::require_model_copy_reference(NativeModelReference& reference) const {
    auto& model = reference.model_owner();
    const auto found = models_.find(&model.storage.node);
    if (&model.environment != &environment_.models || model.phase != NativeModelOwner::Phase::live ||
        found == models_.end() || !found->second || found->second->owner.get() != &model ||
        found->second->reference.get() != &reference)
        throw std::logic_error("widget copy requires this runtime's canonical live Model reference");
}
GuiWidgetOwner& GuiWidgetOwnerRuntime::construct_child_00aa6560(GuiLayoutWidget& layout) {
    if (layout.type != GuiWidgetType::Group && layout.type != GuiWidgetType::Icon &&
        layout.type != GuiWidgetType::FrameBox && layout.type != GuiWidgetType::ClipBox &&
        layout.type != GuiWidgetType::Text && layout.type != GuiWidgetType::Section &&
        layout.type != GuiWidgetType::Listbox)
        throw std::invalid_argument("unsupported retained GUI widget type");
    auto& result = construct_base(layout);
    try { result.bind_scene_00aa6720(create_model(layout.key)); }
    catch (...) { retire_tree(layout); throw; }
    return result;
}
GuiWidgetOwner& GuiWidgetOwnerRuntime::construct_unbound_text_00ab9650(GuiLayoutWidget& layout) {
    if (layout.type != GuiWidgetType::Text)
        throw std::invalid_argument("unbound Text construction requires the actual Text type");
    return construct_base(layout);
}
GuiWidgetOwner& GuiWidgetOwnerRuntime::construct_root(GuiLayoutWidget& layout, NativeNodeBinding& node) {
    if (layout.type != GuiWidgetType::Screen) throw std::invalid_argument("page root requires Screen type");
    auto& result = construct_base(layout);
    result.bind_scene_00aa6720(&node);
    return result;
}
GuiWidgetOwner& GuiWidgetOwnerRuntime::create_with_scene_00aa6640(GuiLayoutWidget& layout) {
    auto& result = construct_child_00aa6560(layout);
    result.implementation().constructed74(result);
    return result;
}
GuiWidgetOwner& GuiWidgetOwnerRuntime::owner(GuiLayoutWidget& layout) const {
    const auto found = widgets_.find(&layout);
    if (found == widgets_.end()) throw std::logic_error("GUI layout has no retained owner");
    return *found->second;
}
GuiWidgetOwner& GuiWidgetOwnerRuntime::owner(GuiWidgetTransform& transform) const {
    for (const auto& entry : widgets_)
        if (&entry.first->transform == &transform) return *entry.second;
    throw std::logic_error("GUI transform has no retained owner");
}
NativeNodeBinding& GuiWidgetOwnerRuntime::node(std::uint32_t address) const {
    for (const auto& entry : widgets_)
        if (entry.second->node_ && identity(entry.second->node_) == address)
            return *entry.second->node_;
    throw std::logic_error("GUI node identity has no actual binding");
}
void GuiWidgetOwnerRuntime::set_node_parent(std::uint32_t child, std::uint32_t parent) {
    environment_.native.set_parent_00b6e680(node(child), parent ? &node(parent) : nullptr);
}
void GuiWidgetOwnerRuntime::constructed74(GuiLayoutWidget& layout) {
    auto& retained = owner(layout);
    retained.implementation().constructed74(retained);
}
void GuiWidgetOwnerRuntime::before_properties(GuiLayoutWidget& layout, const GuiTable& table) {
    auto& retained = owner(layout);
    retained.implementation().before_properties(retained, table);
}
void GuiWidgetOwnerRuntime::base_properties_bound(GuiLayoutWidget& layout) {
    auto& retained = owner(layout);
    retained.scene_.authored_visible = layout.visible;
    retained.recompose_00aa7220();
    if (layout.type != GuiWidgetType::Screen) retained.set_visible34(layout.visible);
}
void GuiWidgetOwnerRuntime::properties_bound(GuiLayoutWidget& layout, const GuiTable& table) {
    auto& retained = owner(layout);
    retained.implementation().properties_bound(retained, table);
}
void GuiWidgetOwnerRuntime::loaded78(GuiLayoutWidget& layout) {
    auto& retained = owner(layout);
    retained.implementation().loaded78(retained);
}
void GuiWidgetOwnerRuntime::stamp_visibility(NativeNodeBinding& binding, float factor, bool recurse) {
    std::memcpy(&binding.storage.scalar_ac, &factor, sizeof(factor));
    if (recurse) {
        for (auto* child = binding.transform.first_child.get(); child; child = child->next_sibling) {
            //Use the actual node binding even for descendants that aren't widgets.
            float child_factor;
            __asm {
                fld factor
                fstp child_factor
            }
            stamp_visibility(environment_.native.resolve_node(*child), child_factor, recurse);
        }
    }
}
void GuiWidgetOwnerRuntime::set_node_visibility_factor_00b6da70(
    NativeNodeBinding& binding, float factor, bool recurse) {
    if (&node(identity(&binding)) != &binding)
        throw std::logic_error("GUI visibility factor requires the same actual widget node");
    stamp_visibility(binding, factor, recurse);
}
void GuiWidgetOwnerRuntime::propagate_visibility(GuiWidgetOwner& retained,
    const GuiWidgetVisibilityArgs& args) {
    if (!retained.node_) return;
    //Preserve native virtual dispatch count/short circuit ordering; a derived
    //visibility reader may have side effects (the diagnostic helper caches it).
    const bool before = args.old_ancestor_visible && retained.implementation().is_visible38(retained);
    bool after = false;
    if (args.new_ancestor_visible)
        after = args.apply_requested ? args.requested : retained.implementation().is_visible38(retained);
    if (before != after) retained.implementation().visibility_changed3c(retained, after);
    const auto child_args = child_visibility_args(args, before, after);
    for (const auto& child : retained.layout_.children)
        propagate_visibility(owner(*child), child_args);
}
void GuiWidgetOwnerRuntime::erase_tree(GuiLayoutWidget& layout) {
    auto& retained = owner(layout);
    retained.require_no_active_owned_operation();
    // Current deleting destructor: derived teardown precedes base AA9730,
    // whose AA8320 call is safe after the manager's separate virtual20 pass.
    retained.implementation().before_scene_release(retained);
    retained.release_scene_nodes_00aa8320();
    for (const auto& child : layout.children) erase_tree(*child);
    retained.retire_timed_entries_00aa9730_fragment();
    layout.before_destroy = {};
    widgets_.erase(&layout);
}
void GuiWidgetOwnerRuntime::retire_tree(GuiLayoutWidget& layout) {
    const auto found = widgets_.find(&layout);
    if (found == widgets_.end()) return;
    if (found->second->base_copy_untyped_) {
        retire_base_copy_admission(*found->second);
        return;
    }
    // Reject unsupported descendant retirement before the first recursive
    // virtual20 releases any scene nodes. This is pure host validation, not
    // derived teardown; AA31F0's release-before-delete ordering stays intact.
    const auto preflight = [this](auto&& self, GuiLayoutWidget& current) -> void {
        auto& retained = owner(current);
        retained.require_no_active_owned_operation();
        retained.implementation().before_host_tree_retirement(retained);
        for (const auto& child : current.children) self(self, *child);
    };
    preflight(preflight, layout);
    // AA31F0 calls current virtual20 at AA326A BEFORE deleting virtual04(1)
    // at AA3276. A scene's final release may consume its remaining roots, so
    // derived Screen teardown must never precede this logical-node release.
    found->second->release_scene_nodes_00aa8320();
    erase_tree(layout);
}
} // namespace bsp
