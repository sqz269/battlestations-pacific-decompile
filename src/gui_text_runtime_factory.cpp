#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/native_mesh_owner.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <utility>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void require_services(GuiTextRuntimeFactoryServices& s) {
    auto& b = s.buffers;
    auto& content = s.properties.submit.content;
    auto& actual = b.geometry.actual_owners();
    require(&s.dispatch.buffers == &b && &s.clip.buffers == &b &&
        &content.content.buffers == &b &&
        &s.properties.font_names.names.widgets == &b.widgets &&
        &content.wrapped.widgets == &b.widgets &&
        &content.wrapped.parenting == &b.parenting &&
        &content.wrapped.children == &s.children &&
        &content.nonempty.style.widgets == &b.widgets,
        "Text factory requires the same canonical widget, buffer and child domains");
    require(&s.properties.font_names.names.actual_owners == &actual &&
        &content.nonempty.style.actual_owners == &actual &&
        &content.single_line.actual_owners == &actual &&
        &content.wrapped.actual_owners == &actual &&
        &b.materials.retained_owners == &actual &&
        &b.widgets.environment().models.retained_owners == &actual &&
        &b.widgets.environment().models.nodes == &b.parenting.nodes,
        "Text factory requires the same actual render/model ownership domains");
    require(&s.properties.font_names.fonts == &content.nonempty.fonts &&
        &content.single_line.fonts == &content.nonempty.fonts &&
        &content.wrapped.fonts == &content.nonempty.fonts &&
        &content.single_line.mapping == &content.wrapped.mapping &&
        &content.single_line.layouts == &content.nonempty.layouts &&
        &s.clip.parameters == &content.nonempty.parameters,
        "Text factory requires the same fonts, mappings, layouts and material parameters");
    require(&s.constructor.one_00d7a24c == &s.properties.constants.one_00d7a24c &&
        &s.constructor.one_00d7a24c == &s.properties.font_names.one_00d7a24c &&
        &s.constructor.shadow_offset_00d5c5c0 == &s.properties.constants.shadow_offset_00d5c5c0 &&
        &s.clip.aspect_ratio_00e12fc0 == &content.nonempty.style.aspect_ratio_00e12fc0,
        "Text factory requires the same borrowed constructor/property/clip constant storage");
    require(s.dispatch.compare_names_00bf7fbf != nullptr,
        "Text current60 requires its actual current CRT comparer");
}
void copy_word(float& to, const float& from) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &from, sizeof(bits));
    std::memcpy(&to, &bits, sizeof(bits));
}
NativeModelOwner& actual_model(NativeNodeBinding* node, GuiTextBufferServices& b) {
    static_assert(sizeof(void*) == 4, "Text runtime requires MSVC Win32");
    require(node != nullptr, "Text alpha requires the current actual model");
    auto* reference = dynamic_cast<NativeModelReference*>(
        b.parenting.nodes.attachments.find_actual_node(
            reinterpret_cast<std::uint32_t>(&node->storage)));
    require(reference != nullptr, "Text drawable has no canonical Model reference");
    auto& model = reference->model_owner();
    require(&model.node == node && &model.storage.node == &node->storage &&
        &model.environment.nodes == &b.parenting.nodes &&
        &model.environment.retained_owners == &b.geometry.actual_owners() &&
        model.phase == NativeModelOwner::Phase::live && reference->reference_count.load() > 0,
        "Text drawable must be the same live actual Model storage/domain");
    return model;
}
// Returns the SAME actual material; null only at the native absent-geometry
// or zero-section gates. No temporary MaterialCloneState or diffuse copy.
NativeMaterialStorage* drawable_material(NativeNodeBinding* node, GuiTextBufferServices& b) {
    auto& model = actual_model(node, b);
    if (!gui_model_has_geometry_00b74650(model)) return nullptr;
    auto& owners = b.geometry.actual_owners();
    auto* raw_mesh = gui_model_geometry_00b74640(model.storage.model, 0);
    auto* mesh = dynamic_cast<NativeMeshReference*>(&owners.resolve_actual(raw_mesh));
    require(mesh && &mesh->storage() == raw_mesh && mesh->reference_count.load() > 0,
        "Text alpha requires its actual live mesh");
    if (!gui_mesh_element_count_00b72b40(raw_mesh)) return nullptr;
    raw_mesh = gui_model_geometry_00b74640(model.storage.model, 0);
    auto* raw_section = gui_geometry_element_00b732c0(raw_mesh, 0);
    auto* section = dynamic_cast<NativeMeshSectionReference*>(&owners.resolve_actual(raw_section));
    require(section && &section->storage() == raw_section && section->reference_count.load() > 0,
        "Text alpha requires its actual live section0");
    auto* raw_material = section->storage().material_20;
    auto* material = dynamic_cast<NativeMaterialReference*>(&owners.resolve_actual(raw_material));
    require(material && &material->storage() == raw_material &&
        material->storage().vtable_00 == 0x00d5e520u && material->reference_count.load() > 0,
        "Text alpha requires its actual live material profile");
    return &material->storage();
}
// Complete AB6BD0. The source font+14 is its CURRENT signed lowword, not the
// descriptor's scale_ratio or a cached metric. Native returns a float32 spill
// reloaded into ST0; C++ float transports that same value.
float normalized_height(GuiTextLifetime& lifetime, NativeFontResourceOwners& fonts,
    const volatile double& divisor) {
    const volatile double* divisor_address = &divisor;
    float result;
    if (lifetime.text().multiline) {
        const float* height = &lifetime.fields().field_178;
        __asm {
            mov eax, height
            mov edx, divisor_address
            fld dword ptr [eax]
            fdiv qword ptr [edx]
            fstp result
        }
    } else {
        const auto* font = lifetime.text().font;
        const std::int32_t height = font ? fonts.resolve(font).signed_height_14() : 0;
        __asm {
            mov edx, divisor_address
            fild height
            fdiv qword ptr [edx]
            fstp result
        }
    }
    return result;
}
} // namespace

GuiTextRuntimeFactory::GuiTextRuntimeFactory(GuiTextRuntimeFactoryServices services)
    : services_(services) { require_services(services_); }
GuiTextRuntimeFactory::~GuiTextRuntimeFactory() noexcept {
    if (!allocations_.empty()) std::terminate();
}
std::unique_ptr<GuiWidgetTypeImplementation> GuiTextRuntimeFactory::make_type(GuiWidgetOwner& owner) {
    require(owner.layout().type == GuiWidgetType::Text && owner.layout().transform.type_id == 3 &&
        &services_.buffers.widgets.owner(owner.layout()) == &owner && !owner.node_binding() &&
        !owner.text_lifetime(), "Text factory requires its fresh primary-null canonical owner");
    require(allocations_.count(&owner.layout()) == 0, "Text layout already has raw allocation transport");
    void* slot = allocate_gui_text_raw_slot_00ab79e0(services_.pool);
    if (!slot) throw std::bad_alloc();
    bool published = false;
    try {
        published = allocations_.emplace(&owner.layout(), Allocation{slot, false}).second;
        require(published, "Text allocation callback occupied the canonical layout transport");
        return std::unique_ptr<GuiWidgetTypeImplementation>(new GuiTextRuntimeImplementation(*this, owner));
    } catch (...) {
        if (published) allocations_.erase(&owner.layout());
        return_gui_text_failed_slot_00ab76f0(slot, services_.pool);
        throw;
    }
}
std::unique_ptr<GuiLayoutWidget> GuiTextRuntimeFactory::construct_unbound_glyph_child() {
    auto child = std::make_unique<GuiLayoutWidget>();
    child->type = GuiWidgetType::Text;
    auto& owner = services_.buffers.widgets.construct_unbound_text_00ab9650(*child);
    auto* implementation = dynamic_cast<GuiTextRuntimeImplementation*>(&owner.implementation());
    require(implementation && &implementation->factory_ == this &&
        owner.text_lifetime() != nullptr && !owner.node_binding(),
        "Text glyph prerequisite must retain the constructed primary-null companion");
    return child;
}
void GuiTextRuntimeFactory::implementation_destroyed(GuiLayoutWidget& layout, bool flags0) noexcept {
    const auto found = allocations_.find(&layout);
    if (found == allocations_.end() || found->second.completed_flags0) std::terminate();
    if (flags0) {
        found->second.completed_flags0 = true;
        return;
    }
    services_.pool.return_raw_slot_00ab75a0(found->second.raw_slot);
    allocations_.erase(found);
}
void GuiTextRuntimeFactory::release_completed_storage(GuiLayoutWidget& layout) {
    const auto found = allocations_.find(&layout);
    require(found != allocations_.end() && found->second.completed_flags0 && !layout.before_destroy,
        "Text storage release requires the retained completed flags0 wrapper");
    services_.pool.return_raw_slot_00ab75a0(found->second.raw_slot);
    allocations_.erase(found);
}
GuiTextRuntimeImplementation::GuiTextRuntimeImplementation(GuiTextRuntimeFactory& factory,
    GuiWidgetOwner& owner) : factory_(factory), owner_(owner) {
    auto& s = factory_.services_;
    lifetime_ = std::make_unique<GuiTextLifetime>(owner, s.buffers, s.children, s.constructor);
}
GuiTextRuntimeImplementation::~GuiTextRuntimeImplementation() noexcept {
    if (has_pending_operation()) std::terminate();
    const bool flags0 = lifetime_->scalar_deletion_phase() == GuiTextScalarDeletionPhase::complete &&
        !(lifetime_->scalar_deletion_flags() & 1u);
    // Lifetime teardown precedes physical storage return. Failed/partial scalar
    // teardown terminates in GuiTextLifetime, never silently freeing the slot.
    auto& layout = owner_.layout();
    lifetime_.reset();
    factory_.implementation_destroyed(layout, flags0);
}
void GuiTextRuntimeImplementation::require_owner(GuiWidgetOwner& owner) const {
    require(&owner == &owner_ && owner.text_lifetime() == lifetime_.get(),
        "Text virtual dispatch requires its same canonical implementation/lifetime");
}
bool GuiTextRuntimeImplementation::has_pending_operation() const noexcept {
    return properties_ || submission_ || clip_.has_value();
}
void GuiTextRuntimeImplementation::require_idle() const {
    if (has_pending_operation())
        throw GuiTextRuntimePending("Text still retains an unfinished content or child70 operation");
}
void GuiTextRuntimeImplementation::constructed74(GuiWidgetOwner& owner) {
    require_owner(owner); require_idle();
    construct_gui_text74_00ab7700(*lifetime_, factory_.services_.dispatch);
}
void GuiTextRuntimeImplementation::properties_bound(GuiWidgetOwner& owner, const GuiTable& table) {
    require_owner(owner); require_idle();
    auto result = read_gui_text_properties_after_base_00abb630(*lifetime_, table, factory_.services_.properties);
    properties_ = std::move(result.pending);
    if (result.status == GuiTextPropertiesStatus::pending_content) {
        require(properties_ != nullptr, "Text property reader lost its required continuation");
        throw GuiTextRuntimePending("Text properties await their actual glyph-child/content continuation");
    }
}
void GuiTextRuntimeImplementation::loaded78(GuiWidgetOwner& owner) {
    require_owner(owner); require_idle();
    load_gui_text78_00ab6aa0(*lifetime_, factory_.services_.dispatch);
}
void GuiTextRuntimeImplementation::set_active60(GuiWidgetOwner& owner, bool active) {
    require_owner(owner);
    set_gui_text_active60_00ab87d0(*lifetime_, active, factory_.services_.dispatch);
}
bool GuiTextRuntimeImplementation::is_visible38(GuiWidgetOwner& owner) {
    require_owner(owner); return owner.base_is_visible38_00a9e0d0();
}
void GuiTextRuntimeImplementation::visibility_changed3c(GuiWidgetOwner& owner, bool visible) {
    require_owner(owner); owner.base_visibility_changed3c_00a9e100(visible);
}
void GuiTextRuntimeImplementation::before_scalar_deletion4(GuiWidgetOwner& owner) {
    require_owner(owner); require_idle();
}
void GuiTextRuntimeImplementation::before_scene_release(GuiWidgetOwner& owner) {
    require_owner(owner); require_idle();
    for (const auto word : owner.extra_fields().pointers_88_90)
        require(word == nullptr, "Text page retirement supports only the empty native timed-entry header");
    lifetime_->destroy_derived_00ab8250_fragment();
}
void GuiTextRuntimeImplementation::release_secondary_scene_nodes(GuiWidgetOwner& owner) {
    require_owner(owner); require_idle();
    lifetime_->release_secondary_scene_nodes_00aa8320_fragment(factory_.services_.lineage_00f8be28);
}
void GuiTextRuntimeImplementation::refresh_clip70(GuiWidgetOwner& owner) {
    require_owner(owner); require_idle();
    auto pending = begin_gui_text_clip_refresh_00ab7a40(*lifetime_, factory_.services_.clip);
    if (pending) clip_.emplace(*pending);
    continue_clip();
}
void GuiTextRuntimeImplementation::continue_clip() {
    while (clip_) {
        // A throwing child leaves the parent's exact BEFORE-current70 frame.
        // No automatic retry, advance or material-registration replay.
        clip_->child_owner().refresh_clip70();
        auto next = resume_gui_text_clip_refresh_after_child70_00ab7a40(*clip_);
        clip_.reset();
        if (next) clip_.emplace(*next);
    }
}
const GuiTextClipRefreshContinuation* GuiTextRuntimeImplementation::pending_clip() const noexcept {
    return clip_ ? &*clip_ : nullptr;
}
void GuiTextRuntimeImplementation::resume_clip_after_child70() {
    require(clip_.has_value(), "Text clip resume requires its current child70 frame");
    auto next = resume_gui_text_clip_refresh_after_child70_00ab7a40(*clip_);
    clip_.reset();
    if (next) clip_.emplace(*next);
    continue_clip();
}
GuiTextRuntimeContentContinuation* GuiTextRuntimeImplementation::pending_content() noexcept {
    if (properties_ && properties_->submission) return properties_->submission->content.get();
    return submission_ ? submission_->content.get() : nullptr;
}
void GuiTextRuntimeImplementation::retain_submission(GuiTextSubmitResult result) {
    submission_ = std::move(result.pending);
    if (result.status == GuiTextSubmitStatus::pending_content) {
        require(submission_ != nullptr, "Text submission lost its required continuation");
        throw GuiTextRuntimePending("Text content awaits its actual glyph-child/builder continuation");
    }
}
void GuiTextRuntimeImplementation::resume_after_glyph_child() {
    if (properties_) {
        if (resume_gui_text_properties_after_child(properties_) == GuiTextPropertiesStatus::pending_content)
            throw GuiTextRuntimePending("Text properties await another actual glyph-child/content continuation");
    } else {
        require(submission_ != nullptr, "Text child resume requires the exact pending outer caller");
        if (resume_gui_text_submit_after_child(submission_) == GuiTextSubmitStatus::pending_content)
            throw GuiTextRuntimePending("Text submission awaits another actual glyph-child/content continuation");
    }
}
void GuiTextRuntimeImplementation::submit_utf16_00ab6ab0(std::u16string_view text) {
    require_idle(); retain_submission(submit_gui_text_utf16_00ab6ab0(*lifetime_, text, factory_.services_.properties.submit));
}
void GuiTextRuntimeImplementation::submit_source_00abaed0(const std::string& text, bool localize) {
    require_idle(); retain_submission(submit_gui_text_source_00abaed0(*lifetime_, text, localize, factory_.services_.properties.submit));
}
void GuiTextRuntimeImplementation::submit_ellipsis_00abb000(const std::string& text, float width, bool localize) {
    require_idle(); retain_submission(submit_gui_text_ellipsis_00abb000(*lifetime_, text, width, localize, factory_.services_.properties.submit));
}
void GuiTextRuntimeImplementation::rebuild_content_00abb1d0() {
    require_idle(); retain_submission(rebuild_gui_text_content_00abb1d0(*lifetime_, factory_.services_.properties.submit));
}
void GuiTextRuntimeImplementation::resize58_00abbf30(const GuiWidgetSize& size) {
    require_idle(); retain_submission(resize_gui_text_00abbf30(*lifetime_, size, factory_.services_.properties.submit));
}
void GuiTextRuntimeImplementation::set_color50_00ab6b50(const float (&rgba)[4]) {
    require_idle();
    auto binding = lifetime_->style_binding(factory_.services_.properties.submit.content.nonempty.style);
    set_gui_text_color50_00ab6b50(binding, rgba);
}
void GuiTextRuntimeImplementation::set_state80_00ab7200(std::int32_t state) {
    require_idle();
    auto binding = lifetime_->style_binding(factory_.services_.properties.submit.content.nonempty.style);
    set_gui_text_state80_00ab7200(binding, state);
}
void GuiTextRuntimeImplementation::set_alpha4c_00ab6ad0(float alpha) {
    require_idle();
    auto& s = factory_.services_;
    actual_model(owner_.node_binding(), s.buffers); // Validate before native stores.
    // AB6AD0 pushes an x87 float32-spilled copy into base AA6980. The later
    // shadow multiplication uses the original caller's stack argument.
    float base_alpha;
    __asm {
        fld alpha
        fstp base_alpha
    }
    copy_word(owner_.layout().color[3], base_alpha);
    copy_word(owner_.layout().transform.alpha, base_alpha);
    copy_word(lifetime_->text().color.a, base_alpha);
    if (auto* main = drawable_material(owner_.node_binding(), s.buffers))
        copy_word(native_material_diffuse_00b179f0(*main, 0)[3], base_alpha);
    if (!lifetime_->shadow_slot_188()) return;
    if (auto* shadow = drawable_material(lifetime_->shadow_slot_188(), s.buffers)) {
        const float* shadow_alpha = &lifetime_->text().shadow_color.a;
        float product;
        __asm {
            mov eax, shadow_alpha
            fld dword ptr [eax]
            fmul alpha
            fstp product
        }
        float* destination = native_material_diffuse_00b179f0(*shadow, 0) + 3;
        __asm {
            mov eax, destination
            fld product
            fstp dword ptr [eax]
        }
    }
}
void GuiTextRuntimeImplementation::align_bounds64_00ab6d70(float& left, float& top,
    float& right, float& bottom) {
    require_idle();
    auto& s = factory_.services_;
    auto& text = lifetime_->text();
    // AB6D73 always performs FLD, even when both align branches discard half.
    // Keep its exact extended value in a host-only80-bit spill between C++
    // branches; no implicit SSE load or narrowing to a C++ double replaces it.
    std::byte half_extended[10];
    const volatile double* initial_half = &s.bounds.half_00d7a280;
    __asm {
        mov eax, initial_half
        fld qword ptr [eax]
        fstp tbyte ptr [half_extended]
    }
    // Native XMM0/XMM1 preserve the original endpoint words until byte1F0.
    float original_left, original_right;
    copy_word(original_left, left); copy_word(original_right, right);
    const auto horizontal = static_cast<std::int32_t>(text.align);
    const volatile double* divisor = &s.bounds.width_divisor_00cec380;
    const volatile double* padding = &s.bounds.padding_00d5c5c8;
    // Native loads half once before the horizontal half, and again only for
    // the centered vertical height after AB6BD0.
    const float* width = &text.measured_width;
    float* left_out = &left;
    float* right_out = &right;
    float width_spill, midpoint, half_width;
    if (horizontal == 0) {
        __asm {
            mov eax, width
            mov edx, divisor
            fld dword ptr [eax]
            fdiv qword ptr [edx]
            fstp width_spill
            fld width_spill
            fadd original_left
            mov edx, padding
            fadd qword ptr [edx]
            mov eax, right_out
            fstp dword ptr [eax]
        }
    } else if (horizontal == 1) {
        __asm {
            fld original_right
            fadd original_left
            fld tbyte ptr [half_extended]
            fmulp st(1), st(0)
            fstp midpoint
            mov eax, width
            mov edx, divisor
            fld dword ptr [eax]
            fdiv qword ptr [edx]
            fstp width_spill
            fld width_spill
            fld tbyte ptr [half_extended]
            fmulp st(1), st(0)
            fstp half_width
            fld midpoint
            fld st(0)
            fld half_width
            fld st(0)
            fsubp st(2), st(0)
            mov edx, padding
            fld qword ptr [edx]
            fsub st(2), st(0)
            fxch st(2)
            mov eax, left_out
            fstp dword ptr [eax]
            faddp st(2), st(0)
            faddp st(1), st(0)
            mov eax, right_out
            fstp dword ptr [eax]
        }
    } else if (horizontal == 2) {
        __asm {
            fld original_right
            mov eax, width
            mov edx, divisor
            fld dword ptr [eax]
            fdiv qword ptr [edx]
            fstp width_spill
            fsub width_spill
            mov edx, padding
            fsub qword ptr [edx]
            mov eax, left_out
            fstp dword ptr [eax]
        }
    }
    float* top_out = &top;
    float* bottom_out = &bottom;
    // Preserve the literal EAX/ECX branches: top and bottom test horizontal
    // alignment; only center tests VerticalAlign. This is not a typo.
    if (horizontal == 0) {
        const float height = normalized_height(*lifetime_, s.properties.font_names.fonts,
            s.bounds.height_divisor_00cef1b8);
        __asm {
            fld height
            mov eax, top_out
            fadd dword ptr [eax]
            mov eax, bottom_out
            fstp dword ptr [eax]
        }
    } else if (text.vertical_align == GuiTextVerticalAlign::Center) {
        __asm {
            mov eax, top_out
            mov edx, bottom_out
            fld dword ptr [eax]
            fadd dword ptr [edx]
            fld tbyte ptr [half_extended]
            fmulp st(1), st(0)
            fstp midpoint
        }
        const float height = normalized_height(*lifetime_, s.properties.font_names.fonts,
            s.bounds.height_divisor_00cef1b8);
        const volatile double* current_half = &s.bounds.half_00d7a280;
        float half_height;
        __asm {
            fld height
            mov edx, current_half
            fmul qword ptr [edx]
            fstp half_height
            fld midpoint
            fsub half_height
            mov eax, top_out
            fstp dword ptr [eax]
            fld half_height
            fadd midpoint
            mov eax, bottom_out
            fstp dword ptr [eax]
        }
    } else if (horizontal == 2) {
        double captured_bottom;
        __asm {
            mov eax, bottom_out
            fld dword ptr [eax]
            fstp captured_bottom
        }
        const float height = normalized_height(*lifetime_, s.properties.font_names.fonts,
            s.bounds.height_divisor_00cef1b8);
        __asm {
            fld height
            fsubr captured_bottom
            mov eax, top_out
            fstp dword ptr [eax]
        }
    }
    if (!lifetime_->fields().byte_1f0) {
        copy_word(left, original_left); copy_word(right, original_right);
    }
}
} // namespace bsp
