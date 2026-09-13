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
    void register_acquired() {
        auto& a = acquisition;
        const auto& r = geometry.registration();
        if (auto* existing = r.find(r.context, a.creator)) {
            auto* layout = dynamic_cast<GuiTextNativeLayoutReference*>(existing);
            require(layout && layout->storage() == a.creator &&
                layout->matches_context(construct.actual_owner, profile),
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
        s.register_acquired();
    }
    // B865A0 immediately writes the return to section+50. Null follows the
    // original null-allocation result; no empty successful companion is made.
    a.creator = nullptr;
    a.phase = GuiTextNativeLayoutPhase::transferred;
    return result;
}
} // namespace bsp
