#include "bsp/gui_text_native_layout.hpp"
#include "bsp/native_renderer_binding_getters.hpp"
#include <atomic>
#include <exception>
#include <list>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool ok, const char* why) { if (!ok) throw std::logic_error(why); }
std::uint32_t word(const void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const std::byte*>(p) + offset);
}
std::atomic<std::int32_t>& count(void* p) {
    require(p != nullptr, "hardware layout companion requires actual storage");
    return *reinterpret_cast<std::atomic<std::int32_t>*>(static_cast<std::byte*>(p) + 4);
}
bool profile_matches(void* raw, const volatile std::uint32_t* profile) noexcept {
    return word(raw) == 0x00d62af4u && profile &&
        profile[0] == 0x00bd30e0u && profile[1] == 0x00b60770u;
}
}
GuiTextNativeLayoutReference::GuiTextNativeLayoutReference(void* raw,
    NativeHardwareLayoutOwnerContext& context, const volatile std::uint32_t* profile,
    GuiTextNativeLayoutDisposal disposal)
    : RenderCommandReference(count(raw)), storage_(raw), context_(context),
      profile_(profile), disposal_(disposal) {
    require(disposal.retire && reference_count.load(std::memory_order_relaxed) > 0 &&
        profile_matches(raw, profile), "hardware layout requires its live actual profile and retirement");
}
GuiTextNativeLayoutReference::~GuiTextNativeLayoutReference() {
    if (phase_ != Phase::retired) std::terminate();
}
bool GuiTextNativeLayoutReference::matches_context(const NativeHardwareLayoutOwnerContext& c,
    const volatile std::uint32_t* p) const noexcept { return &c == &context_ && p == profile_; }
void GuiTextNativeLayoutReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound || !profile_matches(storage_, profile_) ||
        reference_count.load(std::memory_order_relaxed) != 0) std::terminate();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    (void)delete_native_hardware_layout_00b60770(storage_, 1, context_);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this); // May delete this; no later member access.
}

bool GuiTextNativeLayoutReference::matches_raw_context(
    const NativeHardwareLayoutOwnerContext& context,
    const volatile std::uint32_t* profile) const {
    return phase_ == Phase::bound && &context_ == &context && profile_ == profile;
}
void* GuiTextNativeLayoutReference::delete_raw_scalar_00b60770(std::uint32_t flags) {
    if (phase_ != Phase::bound) {
        phase_ = Phase::failed;
        throw std::logic_error("raw hardware layout scalar cannot replay or reenter");
    }
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    try {
        void* const result = delete_native_hardware_layout_00b60770(storage_, flags, context_);
        if (phase_ != Phase::destroying)
            throw std::logic_error("raw hardware layout scalar invalidated while active");
        phase_ = Phase::retired;
        disposal.retire(disposal.context, *this); // No later companion/raw-slot access.
        return result;
    } catch (...) {
        phase_ = Phase::failed;
        throw;
    }
}
struct GuiTextNativeLayoutServices::Impl {
    struct Entry {
        Impl* domain;
        void* raw;
        bool registered{};
        std::unique_ptr<GuiTextNativeLayoutReference> reference;
    };
    GuiNativeGeometryOwners& geometry;
    NativeLogicalVertexOwnerContext& vertices;
    NativeVertexDeclarationLoadingContext& declarations;
    NativeHardwareLayoutConstructContext& construct;
    const volatile std::uint32_t* profile;
    std::list<Entry> entries;
    GuiTextNativeLayoutAcquired acquisition;

    void require_domain() {
        const auto& registration = geometry.registration();
        auto& h = construct.actual_owner;
        require(registration.bind && registration.unbind && registration.find &&
            &registration.owners == &vertices.actual_owners &&
            h.canonical_declaration_owners == &registration.owners &&
            static_cast<const volatile void*>(&h.actual_renderer_00f8d394) ==
                static_cast<const volatile void*>(&vertices.actual_renderer_00f8d394) &&
            h.actual_renderer_profile_00d5f0a8 == vertices.actual_renderer_profile_00d5f0a8 &&
            h.actual_type_sizes_00d61cc0 == declarations.type_sizes_00d61cc0 &&
            h.actual_type_sizes_00d61cc0 == vertices.actual_type_sizes_00d61cc0 &&
            h.actual_declaration_pool_0108fd38 == declarations.pool_0108fd38 &&
            h.actual_declaration_profile_00d61d1c == declarations.declaration_vtable_00d61d1c &&
            &construct.actual_string_storage == &declarations.strings,
            "Text hardware layout requires the same canonical renderer, declarations, strings and owners");
    }
    static void retire(void* opaque, GuiTextNativeLayoutReference& reference) noexcept {
        auto& e = *static_cast<Entry*>(opaque);
        auto& s = *e.domain;
        const auto& r = s.geometry.registration();
        if (e.registered) r.unbind(r.context, e.raw, reference);
        for (auto i = s.entries.begin(); i != s.entries.end(); ++i) {
            if (&*i == &e) { s.entries.erase(i); return; }
        }
        std::terminate();
    }
    void register_acquired(GuiTextNativeLayoutAcquired& a,
        const NativeHardwareLayoutFactoryObservation* observed = nullptr) {
        const auto& r = geometry.registration();
        auto* const existing = r.find(r.context, a.creator);
        if (observed) {
            require(observed->source_complete() && observed->actual_context() == &construct &&
                observed->source_result() == a.creator &&
                observed->actual_tree() == construct.actual_owner.actual_tree_0108d530 &&
                observed->actual_pool() == construct.actual_owner.actual_hardware_layout_pool_0108fe9c,
                "raw layout admission requires its genuine actual factory invocation");
            if (observed->source_cache_hit())
                require(existing != nullptr, "raw cached layout requires its existing same-generation companion");
            else
                require(observed->source_constructor_returned() && observed->source_pre_insertion() &&
                    observed->allocated_slot() == a.creator && existing == nullptr,
                    "raw new layout requires actual constructor return and no stale canonical generation");
        }
        if (existing) {
            auto* layout = dynamic_cast<GuiTextNativeLayoutReference*>(existing);
            require(layout && layout->storage() == a.creator &&
                layout->matches_context(construct.actual_owner, profile) &&
                (!observed || layout->matches_raw_context(construct.actual_owner, profile)),
                "cached hardware layout must reuse its existing same-domain companion");
            a.companion = layout;
            a.registered = true;
            a.reused = true;
            return;
        }
        entries.push_back({this, a.creator, false, {}});
        auto& e = entries.back();
        e.reference = std::make_unique<GuiTextNativeLayoutReference>(a.creator,
            construct.actual_owner, profile, GuiTextNativeLayoutDisposal{&e, retire});
        a.companion = e.reference.get();
        r.bind(r.context, e.raw, *e.reference); // Transactional. Failure retains creator/companion.
        e.registered = true;
        a.registered = true;
    }
};
GuiTextNativeLayoutServices::GuiTextNativeLayoutServices(GuiNativeGeometryOwners& g,
    NativeLogicalVertexOwnerContext& v, NativeVertexDeclarationLoadingContext& d,
    NativeHardwareLayoutConstructContext& c, const volatile std::uint32_t* p)
    : impl_(std::make_unique<Impl>(Impl{g, v, d, c, p, {}, {}})) {
    impl_->require_domain();
}
GuiTextNativeLayoutServices::~GuiTextNativeLayoutServices() {
    if (!impl_->entries.empty() || (impl_->acquisition.phase != GuiTextNativeLayoutPhase::empty &&
        impl_->acquisition.phase != GuiTextNativeLayoutPhase::transferred)) std::terminate();
}
const GuiTextNativeLayoutAcquired& GuiTextNativeLayoutServices::acquired() const noexcept {
    return impl_->acquisition;
}
bool GuiTextNativeLayoutServices::has_live_companions() const noexcept {
    return !impl_->entries.empty();
}
void* GuiTextNativeLayoutServices::current_stream_descriptor_virtual24(void* stream) {
    auto& s = *impl_;
    s.require_domain();
    const auto* profile = s.vertices.actual_logical_profile_00d61d6c;
    require(stream && word(stream) == 0x00d61d6cu && profile && profile[9] == 0x00b48ce0u,
        "Text layout requires actual D61D6C current24 B48CE0");
    auto* reference = dynamic_cast<NativeLogicalVertexReference*>(
        &s.vertices.actual_owners.resolve_actual(stream));
    require(reference && reference->storage() == stream && &reference->reference_count == &count(stream),
        "Text layout descriptor requires the canonical actual logical vertex reference");
    return native_logical_vertex_stream_get_declaration_00b48ce0(stream);
}
void* GuiTextNativeLayoutServices::current_renderer_layout_virtual40(const NativeMeshSectionLayoutKey& key) {
    auto& s = *impl_;
    s.require_domain();
    auto& a = s.acquisition;
    require(a.phase == GuiTextNativeLayoutPhase::empty || a.phase == GuiTextNativeLayoutPhase::transferred,
        "Text hardware layout cannot replay an interrupted acquisition");
    auto& h = s.construct.actual_owner;
    auto* renderer = h.actual_renderer_00f8d394;
    require(renderer && word(renderer) == 0x00d5f0a8u && h.actual_renderer_profile_00d5f0a8 &&
        h.actual_renderer_profile_00d5f0a8[0x40 / 4] == 0x00b2f710u,
        "Text layout requires actual D5F0A8 current40 B2F710");
    a = {};
    a.phase = GuiTextNativeLayoutPhase::factory;
    void* result = get_or_create_native_hardware_layout_00b2f710(&key, s.construct, &a.creator);
    if (result) {
        require(result == a.creator, "hardware layout factory publication differs from return");
        a.phase = GuiTextNativeLayoutPhase::registration;
        s.register_acquired(a);
    }
    // B865A0 immediately writes the return to section+50. Null follows the
    // original null-allocation result; no empty successful companion is made.
    a.creator = nullptr;
    a.phase = GuiTextNativeLayoutPhase::transferred;
    return result;
}
GuiTextNativeRawLayoutInvocation::GuiTextNativeRawLayoutInvocation(
    GuiTextNativeLayoutServices& service, const void* actual_key,
    NativeMeshSectionLayoutReferenceOperation const volatile& increment,
    NativeMeshSectionLayoutReferenceOperation const volatile& decrement)
    : service_(service), actual_key_home_(actual_key),
      context_{*this, *this, service.impl_->construct,
          service.impl_->construct.actual_owner.actual_renderer_00f8d394,
          increment, decrement, this} {
    service.impl_->require_domain(); // Source address/service metadata only.
    require(service.impl_->profile && service.impl_->vertices.actual_logical_profile_00d61d6c &&
        service.impl_->construct.actual_owner.actual_renderer_profile_00d5f0a8,
        "raw layout preparation requires its genuine existing table views");
}
void GuiTextNativeRawLayoutInvocation::begin_effect() {
    if (active_ || failed_ || factory_started_) {
        failed_ = true;
        throw std::logic_error("raw layout Source invocation cannot replay or reenter");
    }
    active_ = true;
}
const volatile std::uint32_t* GuiTextNativeRawLayoutInvocation::current_table(
    void*, std::uint32_t profile) {
    auto& s = *service_.impl_;
    if (profile == 0x00d61d6cu) return s.vertices.actual_logical_profile_00d61d6c;
    if (profile == 0x00d62af4u) return s.profile;
    if (profile == 0x00d5f0a8u) return s.construct.actual_owner.actual_renderer_profile_00d5f0a8;
    failed_ = true;
    throw std::logic_error("OTHER raw layout table is outside the admitted Source families");
}
void GuiTextNativeRawLayoutInvocation::delete_vslot04(void* actual,
    std::uint32_t profile, std::uint32_t flags) {
    begin_effect();
    try {
        auto& s = *service_.impl_;
        const auto& r = s.geometry.registration();
        std::uint32_t target;
        if (profile == 0x00d61d6cu) target = s.vertices.actual_logical_profile_00d61d6c[1];
        else if (profile == 0x00d62af4u) target = s.profile[1];
        else throw std::logic_error("OTHER raw scalar profile remains unadmitted");
        auto* const base = r.find(r.context, actual);
        if (profile == 0x00d61d6cu) {
            auto* reference = dynamic_cast<NativeLogicalVertexReference*>(base);
            require(target == 0x00b4bf10u && reference && reference->storage() == actual &&
                reference->matches_raw_context(s.vertices),
                "raw logical scalar requires its captured target and same live companion/context");
            (void)reference->delete_raw_scalar_00b4bf10(flags);
        } else if (profile == 0x00d62af4u) {
            auto* reference = dynamic_cast<GuiTextNativeLayoutReference*>(base);
            require(target == 0x00b60770u && reference && reference->storage() == actual &&
                reference->matches_raw_context(s.construct.actual_owner, s.profile),
                "raw hardware scalar requires its captured target and same live companion/context");
            (void)reference->delete_raw_scalar_00b60770(flags);
        } else {
            throw std::logic_error("OTHER raw scalar profile remains unadmitted");
        }
        require(!failed_, "raw scalar Source invocation invalidated while active");
        active_ = false;
    } catch (...) {
        failed_ = true; active_ = false;
        if (!failure_) failure_ = std::current_exception();
        throw;
    }
}
void GuiTextNativeRawLayoutInvocation::other_reference_current00(void* actual,
    std::uint32_t profile, std::uint32_t target) {
    if ((profile != 0x00d61d6cu && profile != 0x00d62af4u) || target != 0x00bd30e0u) {
        failed_ = true;
        throw std::logic_error("OTHER current00 target remains unadmitted by the real raw provider");
    }
    invoke_native_ref_counted_delete_00bd30e0(actual, *this);
}
void* GuiTextNativeRawLayoutInvocation::other_stream_descriptor_current24(void* stream,
    std::uint32_t profile, std::uint32_t target) {
    begin_effect();
    try {
        auto& s = *service_.impl_;
        const auto& r = s.geometry.registration();
        auto* reference = dynamic_cast<NativeLogicalVertexReference*>(r.find(r.context, stream));
        require(profile == 0x00d61d6cu && target == 0x00b48ce0u && reference &&
            reference->storage() == stream && reference->matches_raw_context(s.vertices),
            "OTHER stream descriptor target remains unadmitted by the real raw provider");
        void* const result = native_logical_vertex_stream_get_declaration_00b48ce0(stream);
        require(!failed_, "raw descriptor Source invocation invalidated while active");
        active_ = false;
        return result;
    } catch (...) {
        failed_ = true; active_ = false;
        if (!failure_) failure_ = std::current_exception();
        throw;
    }
}
void* GuiTextNativeRawLayoutInvocation::other_renderer_layout_current40(void* renderer,
    std::uint32_t profile, std::uint32_t target, const void* key, void** acquired) {
    if (target != 0x00b2f710u) {
        failed_ = true;
        throw std::logic_error("OTHER renderer layout target remains unadmitted by the real raw provider");
    }
    return create_layout_00b2f710(renderer, profile, key, service_.impl_->construct, acquired);
}
void* GuiTextNativeRawLayoutInvocation::create_layout_00b2f710(void*,
    std::uint32_t profile, const void* key,
    NativeHardwareLayoutConstructContext& actual_construct, void** acquired) {
    begin_effect();
    factory_started_ = true; // A factory never replays, including swallowed denial.
    try {
        auto& s = *service_.impl_;
        require(profile == 0x00d5f0a8u && &actual_construct == &s.construct &&
            key == actual_key_home_, "raw layout factory requires its actual key/context/captured family");
        acquisition_.phase = GuiTextNativeLayoutPhase::factory;
        void* const result = get_or_create_native_hardware_layout_00b2f710(
            key, actual_construct, acquired, observation_);
        acquisition_.creator = result; // Source return identity, no HOME rescan.
        require(!failed_ && observation_.source_complete(),
            "raw factory Source invocation invalidated while active");
        if (result) {
            acquisition_.phase = GuiTextNativeLayoutPhase::registration;
            s.register_acquired(acquisition_, &observation_);
        }
        require(!failed_, "raw layout Source admission invalidated while active");
        acquisition_.phase = GuiTextNativeLayoutPhase::transferred;
        factory_complete_ = true; // Factory/metadata return only, NOT section publication.
        active_ = false;
        return result;
    } catch (...) {
        failed_ = true; active_ = false;
        if (!failure_) failure_ = std::current_exception();
        throw;
    }
}

} // namespace bsp
