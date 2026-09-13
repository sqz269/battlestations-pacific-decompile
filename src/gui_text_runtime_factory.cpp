#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/gui_text_child_lifetime.hpp"
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
struct CopyDispatch {
    bool& active;
    explicit CopyDispatch(bool& value) : active(value) {
        require(!active, "Text copy constructor dispatch cannot reenter its native continuation");
        active = true;
    }
    ~CopyDispatch() { active = false; }
};
void require_services(GuiTextRuntimeFactoryServices& s) {
    auto& b = s.buffers;
    auto& content = s.properties.submit.content;
    auto& actual = b.geometry.actual_owners();
    const auto& registration = b.geometry.registration();
    require(&registration.owners == &actual && registration.bind && registration.unbind && registration.find &&
        dynamic_cast<GuiTextChildDeletion*>(&s.children),
        "Text identity requires its same canonical registration and concrete child deletion transport");
    require(&s.dispatch.buffers == &b && &s.clip.buffers == &b &&
        &content.content.buffers == &b &&
        &s.properties.font_names.names.widgets == &b.widgets &&
        &content.wrapped.widgets == &b.widgets &&
        &content.wrapped.parenting == &b.parenting &&
        &content.wrapped.children == &s.children &&
        &content.content.calls.glyph_child_calls() == &s.children &&
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
    // Reserve the host record BEFORE any native prefix/body construction.
    try {
        require(allocations_.try_emplace(&owner.layout()).second,
            "Text allocation callback occupied the canonical layout transport");
    } catch (...) {
        return_gui_text_failed_slot_00ab76f0(slot, services_.pool);
        throw;
    }
    auto& allocation = allocations_.at(&owner.layout());
    allocation.raw_slot = slot;
    // The current host runtime established its semantic base before make_type.
    // No native base callbacks are replayed here: this is the actual prefix
    // admission boundary, not a claim of full default AA9390 callback ordering.
    allocation.prefix = initialize_native_gui_widget_identity_00aa9390_fragment(slot);
    try {
        allocation.default_construction = std::unique_ptr<GuiTextRuntimeImplementation>(
            new GuiTextRuntimeImplementation(*this, owner, GuiTextRuntimeImplementation::DefaultAdmission{}));
        services_.buffers.widgets.begin_default_type_admission(owner, *allocation.default_construction);
        bind_identity(owner);
        allocation.default_construction->initialize_default();
        // construct_base receives this SAME shell and only then clears its
        // borrowed constructor admission. On throw the record keeps it alive.
        return std::move(allocation.default_construction);
    } catch (...) {
        allocation.failure = std::current_exception();
        throw; // Preserve the raw prefix, registration and native acquired effects.
    }
}
void GuiTextRuntimeFactory::bind_identity(GuiWidgetOwner& owner) {
    auto& allocation = allocations_.at(&owner.layout());
    require(allocation.prefix && !allocation.identity && !allocation.registered,
        "Text canonical identity may be bound exactly once after prefix construction");
    auto* deletion = dynamic_cast<GuiTextChildDeletion*>(&services_.children);
    require(deletion != nullptr, "Text terminal requires its concrete canonical deletion transport");
    const auto& registration = services_.buffers.geometry.registration();
    require(registration.find(registration.context, allocation.raw_slot) == nullptr,
        "Text pool returned an identity already present in the canonical registry");
    allocation.identity = std::make_unique<NativeGuiTextIdentityReference>(
        *allocation.prefix, owner, *deletion);
    registration.bind(registration.context, allocation.raw_slot, *allocation.identity);
    allocation.registered = true;
    require(registration.find(registration.context, allocation.raw_slot) == allocation.identity.get(),
        "Text identity registration did not publish the same canonical companion");
}
NativeGuiTextIdentityReference& GuiTextRuntimeFactory::actual_identity(GuiWidgetOwner& owner) {
    const auto found = allocations_.find(&owner.layout());
    require(found != allocations_.end() && !found->second.completed_flags0 &&
        found->second.identity && found->second.registered &&
        &found->second.identity->canonical_owner() == &owner &&
        &owner.runtime() == &services_.buffers.widgets,
        "Text actual identity requires its same live factory allocation and canonical owner");
    const auto& registration = services_.buffers.geometry.registration();
    require(registration.find(registration.context, found->second.raw_slot) == found->second.identity.get(),
        "Text actual identity lost its original canonical registry binding");
    return *found->second.identity;
}
GuiWidgetOwner& GuiTextRuntimeFactory::canonical_owner(void* raw) {
    // Check the allocation before dereferencing a companion's owner: flags0
    // may retain storage/companion after the C++ widget owner was retired.
    for (auto& entry : allocations_) {
        auto& allocation = entry.second;
        if (allocation.raw_slot != raw) continue;
        require(!allocation.completed_flags0 && allocation.identity && allocation.registered,
            "completed or unbound Text storage has no live canonical body");
        auto& owner = allocation.identity->canonical_owner();
        require(&actual_identity(owner) == allocation.identity.get(),
            "Text material body lookup must use its original factory identity");
        return owner;
    }
    throw std::invalid_argument("raw identity is not this factory's canonical Text allocation");
}
void GuiTextRuntimeFactory::bind_retained_text_00b18a40(NativeMaterialStorage& material,
    GuiWidgetOwner& owner, NativeRenderActualOwners& actual) {
    require(&actual == &services_.buffers.geometry.actual_owners(),
        "cursor material and Text must share the original actual-owner domain");
    auto& identity = actual_identity(owner);
    bind_retained_native_gui_text_parameter_owner_00b18a40(material, identity, actual);
}
const std::exception_ptr& GuiTextRuntimeFactory::construction_failure(GuiLayoutWidget& layout) const {
    const auto found = allocations_.find(&layout);
    require(found != allocations_.end(), "Text construction has no retained allocation record");
    return found->second.failure;
}
std::unique_ptr<GuiTextRuntimeCopyOperation> GuiTextRuntimeFactory::begin_copy_00aa1380(
    GuiWidgetOwner& source, std::unique_ptr<GuiLayoutWidget>& destination,
    const GuiWidgetBaseCopyPreimage& preimage, GuiTextRuntimeCopyServices services) {
    require_services(services_);
    source.require_no_active_owned_operation();
    auto* implementation = dynamic_cast<GuiTextRuntimeImplementation*>(&source.implementation());
    require(implementation && &implementation->factory_ == this &&
        source.text_lifetime() == implementation->lifetime_.get() &&
        !implementation->has_pending_operation() && !implementation->source_copy_borrowed_ &&
        &services_.buffers.widgets.owner(source.layout()) == &source &&
        source.layout().type == GuiWidgetType::Text && source.layout().transform.type_id == 3,
        "copied Text factory requires its same live source implementation and allocation domain");
    const auto allocation = allocations_.find(&source.layout());
    require(allocation != allocations_.end() && !allocation->second.completed_flags0,
        "copied Text source must have its live original pool allocation transport");
    require(actual_identity(source).reference_count.load(std::memory_order_relaxed) > 0,
        "copied Text source requires its live actual native count");
    require(destination && destination.get() != &source.layout() &&
        !destination->before_destroy && !destination->parent && !destination->transform.parent &&
        destination->children.empty() && destination->transform.children.empty() &&
        !allocations_.count(destination.get()),
        "copied Text factory requires a fresh distinct destination layout");
    require(&services.cursor.buffers == &services_.buffers &&
        &services.cursor.parameter_owner == this &&
        &services.cursor.layouts == &services_.properties.submit.content.nonempty.layouts &&
        &services.cursor.one_00d7a24c == &services_.constructor.one_00d7a24c,
        "copied Text factory must use its same cursor/content/live constant bindings");
    return std::unique_ptr<GuiTextRuntimeCopyOperation>(new GuiTextRuntimeCopyOperation(
        *this, source, *implementation, destination, preimage, services));
}

GuiTextRuntimeCopyOperation::GuiTextRuntimeCopyOperation(GuiTextRuntimeFactory& factory,
    GuiWidgetOwner& source, GuiTextRuntimeImplementation& implementation,
    std::unique_ptr<GuiLayoutWidget>& destination, const GuiWidgetBaseCopyPreimage& preimage,
    GuiTextRuntimeCopyServices services)
    : factory_(factory), source_(source), source_implementation_(implementation),
      preimage_(preimage), services_(services),
      source_borrow_(std::make_unique<GuiWidgetCopySourceBorrow>(source)) {
    // No native calls before this stable frame exists. A failed source-borrow
    // admission leaves the caller's sole destination ownership untouched.
    source_implementation_.source_copy_borrowed_ = true;
    destination_ = std::move(destination);
}
GuiTextRuntimeCopyOperation::~GuiTextRuntimeCopyOperation() noexcept {
    if (phase_ != GuiTextRuntimeCopyPhase::ready && phase_ != GuiTextRuntimeCopyPhase::complete &&
        phase_ != GuiTextRuntimeCopyPhase::null_allocation) std::terminate();
    release_source_borrow();
    // Complete owns either no layout (transferred) or the same admitted layout,
    // whose existing before_destroy performs its ordinary canonical retirement.
}
void GuiTextRuntimeCopyOperation::release_source_borrow() noexcept {
    if (!source_borrow_) return; // Source may already have retired after success.
    source_implementation_.source_copy_borrowed_ = false;
    source_borrow_.reset();
}
GuiTextRuntimeImplementation& GuiTextRuntimeCopyOperation::implementation() {
    require(destination_ && copied_implementation_ && copied_implementation_->lifetime_,
        "copied Text implementation is not yet constructed or its layout was transferred");
    return *copied_implementation_;
}
GuiTextRuntimeContentContinuation* GuiTextRuntimeCopyOperation::pending_content() noexcept {
    return destination_ && copied_implementation_ ? copied_implementation_->pending_content() : nullptr;
}
GuiTextCursorAcquired& GuiTextRuntimeCopyOperation::cursor_acquired() {
    auto& copied = implementation();
    require(copied.copy_.has_value(), "copied Text has no admitted derived cursor frame");
    return copied.copy_->cursor_acquired();
}
GuiTextRuntimeCopyPhase GuiTextRuntimeCopyOperation::run_00aa1380() {
    require(phase_ == GuiTextRuntimeCopyPhase::ready && destination_ && source_borrow_,
        "copied Text factory constructor must run exactly once");
    phase_ = GuiTextRuntimeCopyPhase::running;
    try {
        // AA139E precedes ABB2E5's base copy. Only the proven raw8-byte prefix
        // is constructed; remaining Text body stays with its canonical owner.
        raw_slot_ = allocate_gui_text_raw_slot_00ab79e0(factory_.services_.pool);
        if (!raw_slot_) {
            phase_ = GuiTextRuntimeCopyPhase::null_allocation;
            release_source_borrow(); // Native AA13F2 returns zero, no Text created.
            return phase_;
        }
        require(factory_.allocations_.try_emplace(destination_.get()).second,
            "copied Text allocation callback occupied its destination transport");
        auto& allocation = factory_.allocations_.at(destination_.get());
        allocation.raw_slot = raw_slot_;
        allocation.prefix = initialize_native_gui_widget_identity_00aa9520_fragment(raw_slot_);
        auto& owner = factory_.services_.buffers.widgets.construct_base_copy_00aa9520(
            *destination_, source_, preimage_, services_.base, base_acquired_, source_borrow_.get());
        auto copied = std::unique_ptr<GuiTextRuntimeImplementation>(new GuiTextRuntimeImplementation(
            factory_, owner, GuiTextRuntimeImplementation::CopiedAdmission{}));
        copied_implementation_ = copied.get();
        implementation_ = std::move(copied); // Preserve shell before copied lifetime effects.
        // Publish the retained shell before allocating/copying derived fields.
        // Its active-operation query rejects retirement even if lifetime
        // allocation or copied-string construction throws before association.
        factory_.services_.buffers.widgets.begin_base_copy_type_admission(owner, *copied_implementation_);
        factory_.bind_identity(owner);
        copied_implementation_->initialize_copy(source_implementation_.lifetime(), services_.cursor);
        copied_implementation_->copy_operation_ = this;
        if (copied_implementation_->run_copy() == GuiTextCopyPhase::pending_content) {
            phase_ = GuiTextRuntimeCopyPhase::pending_content;
            return phase_;
        }
        finish_admission();
        return phase_;
    } catch (...) {
        failure_ = std::current_exception();
        const auto found = factory_.allocations_.find(destination_.get());
        if (found != factory_.allocations_.end()) found->second.failure = failure_;
        phase_ = GuiTextRuntimeCopyPhase::failed;
        throw; // All published ownership and source borrow remain on this frame.
    }
}
void GuiTextRuntimeCopyOperation::finish_admission() {
    auto& copied = implementation();
    require(copied.copy_ && copied.copy_->phase() == GuiTextCopyPhase::complete &&
        !copied.lifetime().has_incomplete_copy() && !copied.copy_dispatch_active_,
        "copied Text admission requires completed native content and an inactive constructor call");
    factory_.services_.buffers.widgets.finish_base_copy_type_admission(copied.owner_, implementation_);
    copied.copy_runtime_admitted_ = true;
    copied.copy_operation_ = nullptr;
    phase_ = GuiTextRuntimeCopyPhase::complete;
    release_source_borrow(); // Holding a completed result must not pin the template.
}
GuiTextRuntimeCopyPhase GuiTextRuntimeCopyOperation::resume_after_glyph_child() {
    require(phase_ == GuiTextRuntimeCopyPhase::pending_content,
        "copied Text factory resume requires its retained constructor content");
    auto& copied = implementation();
    require(copied.copy_.has_value(), "copied Text factory lost its constructor frame");
    try {
        {
            CopyDispatch dispatch(copied.copy_dispatch_active_);
            if (copied.copy_->resume_after_child() == GuiTextCopyPhase::pending_content)
                return phase_;
        }
        finish_admission();
        return phase_;
    } catch (...) {
        failure_ = std::current_exception();
        const auto found = factory_.allocations_.find(destination_.get());
        if (found != factory_.allocations_.end()) found->second.failure = failure_;
        phase_ = GuiTextRuntimeCopyPhase::failed;
        throw;
    }
}
std::unique_ptr<GuiLayoutWidget> GuiTextRuntimeCopyOperation::take_completed_layout() {
    require(phase_ == GuiTextRuntimeCopyPhase::complete ||
        phase_ == GuiTextRuntimeCopyPhase::null_allocation,
        "an unfinished copied Text cannot be transferred as a successful widget");
    if (phase_ == GuiTextRuntimeCopyPhase::null_allocation) return nullptr;
    require(destination_ != nullptr, "copied Text layout ownership was already transferred");
    copied_implementation_ = nullptr;
    raw_slot_ = nullptr; // The transported layout now owns its allocation lifetime.
    return std::move(destination_);
}
std::unique_ptr<GuiLayoutWidget> GuiTextRuntimeFactory::construct_unbound_glyph_child() {
    auto child = std::make_unique<GuiLayoutWidget>();
    child->type = GuiWidgetType::Text;
    try {
        auto& owner = services_.buffers.widgets.construct_unbound_text_00ab9650(*child);
        auto* implementation = dynamic_cast<GuiTextRuntimeImplementation*>(&owner.implementation());
        require(implementation && &implementation->factory_ == this &&
            owner.text_lifetime() != nullptr && !owner.node_binding(),
            "Text glyph prerequisite must retain the constructed primary-null companion");
        return child;
    } catch (...) {
        const auto found = allocations_.find(child.get());
        if (found != allocations_.end()) {
            found->second.failure = std::current_exception();
            // This function owns the wrapper until successful return. Retain
            // that SAME wrapper with its failed native frame rather than let
            // its C++ destructor retire the still-constructing canonical owner.
            found->second.failed_default_layout = std::move(child);
        }
        throw;
    }
}
void GuiTextRuntimeFactory::implementation_destroyed(GuiLayoutWidget& layout, bool flags0) noexcept {
    const auto found = allocations_.find(&layout);
    if (found == allocations_.end() || found->second.completed_flags0 ||
        !found->second.identity || !found->second.registered ||
        found->second.default_construction || found->second.failure) std::terminate();
    // Explicit scalar deletion already performed the exact base end hook.
    // Host-tree retirement completes its bounded base effects before removing
    // the implementation, so that route finalizes the same prefix here.
    if (found->second.prefix->native_vtable_00 != 0x00ceb130u)
        finish_native_gui_widget_identity_destruction_00aa9730_fragment(*found->second.prefix);
    if (flags0) {
        found->second.completed_flags0 = true;
        return;
    }
    return_completed_allocation(layout);
}
void GuiTextRuntimeFactory::return_completed_allocation(GuiLayoutWidget& layout) noexcept {
    const auto found = allocations_.find(&layout);
    if (found == allocations_.end() || !found->second.identity ||
        !found->second.registered) std::terminate();
    auto* const raw = found->second.raw_slot;
    auto* const identity = found->second.identity.get();
    const auto& registration = services_.buffers.geometry.registration();
    // Keep the SAME companion/lookup through native releases and pool return.
    // unbind's contract neither reads returned storage nor changes its count.
    services_.pool.return_raw_slot_00ab75a0(raw);
    registration.unbind(registration.context, raw, *identity);
    allocations_.erase(found); // Companion destructor does not release/count-gate.
}
void GuiTextRuntimeFactory::release_completed_storage(GuiLayoutWidget& layout) {
    const auto found = allocations_.find(&layout);
    require(found != allocations_.end() && found->second.completed_flags0 && !layout.before_destroy,
        "Text storage release requires the retained completed flags0 wrapper");
    return_completed_allocation(layout);
}
GuiTextRuntimeImplementation::GuiTextRuntimeImplementation(GuiTextRuntimeFactory& factory,
    GuiWidgetOwner& owner, DefaultAdmission) : factory_(factory), owner_(owner), default_admission_(true) {}
void GuiTextRuntimeImplementation::initialize_default() {
    require(default_admission_ && !default_completed_ && !lifetime_,
        "default Text may initialize its one retained lifetime exactly once");
    auto& s = factory_.services_;
    publish_native_gui_text_identity_00ab9650_fragment(native_identity().storage());
    lifetime_ = std::make_unique<GuiTextLifetime>(GuiTextDeferredDefaultAdmission{},
        owner_, s.buffers, s.children, s.constructor);
    // The lifetime, its tracked AB8530 frame, prefix and shell are retained
    // before any auxiliary model/mesh/section/material creator can be acquired.
    lifetime_->complete_default_construction_00ab9650();
    default_completed_ = true;
}
GuiTextRuntimeImplementation::GuiTextRuntimeImplementation(GuiTextRuntimeFactory& factory,
    GuiWidgetOwner& owner, CopiedAdmission) : factory_(factory), owner_(owner), copied_admission_(true) {}
void GuiTextRuntimeImplementation::initialize_copy(const GuiTextLifetime& source,
    GuiTextCursorServices& cursor) {
    require(copied_admission_ && !lifetime_ && !copy_ && !copy_services_,
        "copied Text shell may admit its sole lifetime only once");
    auto& services = factory_.services_;
    publish_native_gui_text_identity_00abb2c0_fragment(native_identity().storage());
    lifetime_ = std::make_unique<GuiTextLifetime>(GuiTextAfterBaseCopy00aa9520{},
        owner_, services.buffers, services.children, source);
    copy_services_.emplace(GuiTextCopyServices{cursor, services.properties.submit});
    copy_.emplace(*lifetime_, *copy_services_);
}
GuiTextCopyPhase GuiTextRuntimeImplementation::run_copy() {
    require(copy_ && copy_->phase() == GuiTextCopyPhase::admitted,
        "copied Text implementation requires its original admitted constructor frame");
    CopyDispatch dispatch(copy_dispatch_active_);
    return copy_->run_derived_00abb2c0();
}
GuiTextRuntimeImplementation::~GuiTextRuntimeImplementation() noexcept {
    if (has_pending_operation()) std::terminate();
    const bool flags0 = lifetime_->scalar_deletion_phase() == GuiTextScalarDeletionPhase::complete &&
        !(lifetime_->scalar_deletion_flags() & 1u);
    // Lifetime teardown precedes physical storage return. Failed/partial scalar
    // teardown terminates in GuiTextLifetime, never silently freeing the slot.
    auto& layout = owner_.layout();
    copy_.reset(); // Completed frame must not outlive its sole lifetime.
    copy_services_.reset();
    lifetime_.reset();
    factory_.implementation_destroyed(layout, flags0);
}
NativeGuiTextIdentityReference& GuiTextRuntimeImplementation::native_identity() {
    return factory_.actual_identity(owner_);
}
void GuiTextRuntimeImplementation::require_owner(GuiWidgetOwner& owner) const {
    require(&owner == &owner_ && owner.text_lifetime() == lifetime_.get(),
        "Text virtual dispatch requires its same canonical implementation/lifetime");
}
bool GuiTextRuntimeImplementation::has_pending_operation() const noexcept {
    return (default_admission_ && !default_completed_) || properties_ || submission_ || clip_.has_value() ||
        (lifetime_ && lifetime_->has_incomplete_native_resources()) ||
        (copied_admission_ && (!lifetime_ || !copy_ ||
            copy_->phase() != GuiTextCopyPhase::complete || lifetime_->has_incomplete_copy()));
}
bool GuiTextRuntimeImplementation::has_active_operation() const noexcept {
    return has_pending_operation() || copy_dispatch_active_;
}
void GuiTextRuntimeImplementation::require_idle() const {
    if (has_pending_operation() || source_copy_borrowed_ ||
        (copied_admission_ && !copy_runtime_admitted_))
        throw GuiTextRuntimePending("Text still retains an unfinished content or child70 operation");
}
void GuiTextRuntimeImplementation::require_constructor_read_or_idle() const {
    if (copy_dispatch_active_) {
        require(copied_admission_ && copy_ && lifetime_ &&
            owner_.text_lifetime() == lifetime_.get(),
            "constructor bounds require the same actively copied Text lifetime");
        return;
    }
    require_idle();
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
    require_owner(owner); require_idle();
    set_gui_text_active60_00ab87d0(*lifetime_, active, factory_.services_.dispatch);
}
bool GuiTextRuntimeImplementation::is_visible38(GuiWidgetOwner& owner) {
    require_owner(owner); return owner.base_is_visible38_00a9e0d0();
}
void GuiTextRuntimeImplementation::visibility_changed3c(GuiWidgetOwner& owner, bool visible) {
    require_owner(owner); require_idle(); owner.base_visibility_changed3c_00a9e100(visible);
}
void GuiTextRuntimeImplementation::before_scalar_deletion4(GuiWidgetOwner& owner) {
    require_owner(owner); require_idle();
}
void GuiTextRuntimeImplementation::before_scene_release(GuiWidgetOwner& owner) {
    require_owner(owner); require_idle();
    owner.require_timed_entry_ownership();
    begin_native_gui_text_identity_destruction_00ab8250_fragment(native_identity().storage());
    lifetime_->destroy_derived_00ab8250_fragment();
    begin_native_gui_widget_identity_destruction_00aa9730_fragment(native_identity().storage());
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
    if (copy_) {
        if (auto* pending = copy_->pending_content()) return pending->content.get();
    }
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
    if (copied_admission_ && !copy_runtime_admitted_) {
        require(copy_operation_ && copy_ && copy_->phase() == GuiTextCopyPhase::pending_content,
            "copied Text resume requires its same pending factory caller");
        if (copy_operation_->resume_after_glyph_child() == GuiTextRuntimeCopyPhase::pending_content)
            throw GuiTextRuntimePending("Text copy awaits another actual glyph-child/content continuation");
    } else if (properties_) {
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
void GuiTextRuntimeImplementation::set_size58(GuiWidgetOwner& owner, const GuiWidgetSize& size) {
    if (&owner != &owner_)
        throw std::logic_error("Text current58 requires the same canonical owner");
    resize58_00abbf30(size);
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
float GuiTextRuntimeImplementation::normalized_height_00ab6bd0() {
    require_constructor_read_or_idle();
    return normalized_height(*lifetime_, factory_.services_.properties.font_names.fonts,
        factory_.services_.bounds.height_divisor_00cef1b8);
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
void GuiTextRuntimeImplementation::set_alpha4c(GuiWidgetOwner& owner, float alpha) {
    require_owner(owner);
    set_alpha4c_00ab6ad0(alpha);
}
void GuiTextRuntimeImplementation::align_bounds64_00ab6d70(float& left, float& top,
    float& right, float& bottom) {
    require_constructor_read_or_idle();
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
