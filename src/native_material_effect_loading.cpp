#include "bsp/native_material_effect_loading.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_instance_collection.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <exception>
#include <list>
#include <new>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bsp {
namespace {
void require(bool ok, const char* reason) { if (!ok) throw std::logic_error(reason); }
// Native local headers have explicit FH3 ownership. No NativeString destructor
// changes storage. Fixed rewrite operands preserve the pointers captured in
// ESI/EBX before 004CAD40 and read their lengths only at the cleanup point.
struct Name {
    NativeString local_value;
    NativeString& value;
    NativeStringStorage& strings;
    bool armed{};
    bool captured{};
    char* saved_data{};
    explicit Name(NativeStringStorage& s) noexcept : value(local_value), strings(s) {}
    Name(NativeStringStorage& s, NativeString& stable) noexcept : value(stable), strings(s) {}
    ~Name() noexcept {
        // FH3 states2/3 call0041DD20 on the CURRENT header; only the
        // explicit normal path below uses the captured ESI/EBX buffer.
        if (armed) {
            armed = false;
            destroy_native_string_header_0041dd20(&value, strings);
        }
    }
    void destroy() noexcept {
        if (!armed) return;
        armed = false;
        if (captured) {
            if (saved_data) strings.release(saved_data, value.length() + 1u);
        } else destroy_native_string_header_0041dd20(&value, strings);
    }
    void copy(const void* source) {
        copy_native_string_header_00be0a30_fragment(&value, strings, source);
        armed = true; // Native arms only after the initial copy returns.
    }
    void literal(const char* text) {
        value.assign_0041e870(strings, text);
        armed = true;
    }
    void rewrite_operand(const char* text) {
        value.resize_0041dd40(strings, 5, true);
        saved_data = value.data();
        captured = true;
        if (saved_data) std::memcpy(saved_data, text, value.length() + 1u);
        armed = true;
    }
};
std::uint32_t current_word(const void* p) noexcept {
    return *static_cast<const volatile std::uint32_t*>(p);
}
ActualNativeStringPoolStorage& require_domain(NativeMaterialEffectLoadingContext& c) {
    auto* strings = dynamic_cast<ActualNativeStringPoolStorage*>(&c.construction.strings);
    require(strings && c.cache && &c.cache->effects == &c && &c.cache->strings == strings &&
        &c.owners.string_storage() == strings && &c.programs.strings == strings &&
        &c.programs.lifetime.strings == strings &&
        &c.programs.lifetime.retained_owners == &c.owners.actual_owners() &&
        &c.programs.current_renderer_00f8d394 == &c.construction.current_renderer_00f8d394 &&
        &c.programs.current_vfs_0109ceec == &c.current_vfs_0109ceec &&
        c.resolve_existing_name_00bdf4c0,
        "effect loading requires its same actual string/cache/owner/resolution domains");
    return *strings;
}
}

NativeMaterialEffectLoadAcquired::NativeMaterialEffectLoadAcquired() = default;
NativeMaterialEffectLoadAcquired::~NativeMaterialEffectLoadAcquired() {
    if (phase != NativeMaterialEffectLoadPhase::not_started &&
        phase != NativeMaterialEffectLoadPhase::complete) std::terminate();
}

struct NativeMaterialEffectLoadOwners::Impl {
    struct Entry {
        Impl* domain{};
        NativeMaterialEffectStorage* raw{};
        bool registered{};
        std::unique_ptr<NativeMaterialEffectProgramOperation> programs;
        std::unique_ptr<NativeMaterialEffectReference> reference;
    };
    GuiNativeGeometryOwners& geometry;
    NativeMaterialEffectDestructionAccess& access;
    const volatile std::uint32_t* profile;
    std::list<Entry> entries;
    static void retire(void* opaque, NativeMaterialEffectReference& reference) noexcept {
        auto& entry = *static_cast<Entry*>(opaque);
        auto& domain = *entry.domain;
        const auto& registration = domain.geometry.registration();
        if (entry.registered) registration.unbind(registration.context, entry.raw, reference);
        // Actual effect destruction has already invoked descriptor current0.
        // Its callable bindings cannot disappear before that invocation.
        if (entry.programs && entry.programs->has_live_bindings()) std::terminate();
        for (auto it = domain.entries.begin(); it != domain.entries.end(); ++it) {
            if (&*it == &entry) { domain.entries.erase(it); return; }
        }
        std::terminate();
    }
};
NativeMaterialEffectLoadOwners::NativeMaterialEffectLoadOwners(GuiNativeGeometryOwners& geometry,
    NativeMaterialEffectDestructionAccess& access, const volatile std::uint32_t* profile)
    : impl_(std::make_unique<Impl>(Impl{geometry, access, profile, {}})) {
    const auto& r = geometry.registration();
    require(&r.owners == &access.retained_owners && r.bind && r.unbind && r.find && profile &&
        profile[0] == 0x00bd30e0u && profile[1] == 0x00b422d0u,
        "effect companions require the same canonical registration and actual derived terminal");
}
NativeMaterialEffectLoadOwners::~NativeMaterialEffectLoadOwners() {
    if (!impl_->entries.empty()) std::terminate();
}
NativeRenderActualOwners& NativeMaterialEffectLoadOwners::actual_owners() noexcept {
    return impl_->access.retained_owners;
}
NativeStringStorage& NativeMaterialEffectLoadOwners::string_storage() noexcept {
    return impl_->access.strings;
}
void NativeMaterialEffectLoadOwners::register_completed_creator(NativeMaterialEffectLoadAcquired& a) {
    require(a.creator && a.constructor_complete && !a.companion && !a.owner_record && !a.registered &&
        a.programs && a.programs->complete() && a.programs->pass_slots_initialized(),
        "effect terminal admission requires its completed actual program populations");
    auto& s = *impl_;
    const auto& r = s.geometry.registration();
    require(r.find(r.context, a.creator) == nullptr && current_word(a.creator) == 0x00d61a00u,
        "new actual effect creator must have one canonical identity");
    s.entries.emplace_back();
    auto& e = s.entries.back();
    e.domain = &s;
    e.raw = static_cast<NativeMaterialEffectStorage*>(a.creator);
    a.owner_record = &e;
    // After this transfer the stable owner record retains program bindings even
    // if companion allocation or canonical metadata registration fails.
    e.programs = std::move(a.programs);
    e.reference = std::make_unique<NativeMaterialEffectReference>(e.raw->base,
        s.access, s.profile, NativeMaterialEffectCompanionDisposal{&e, Impl::retire});
    a.companion = e.reference.get();
    r.bind(r.context, a.creator, *e.reference); // Transactional; no AddRef.
    e.registered = true;
    a.registered = true;
}

bool resolve_native_material_effect_name(void* name, NativeMaterialEffectLoadingContext& c) {
    require(c.resolve_existing_name_00bdf4c0,
        "effect name resolution requires the actual mutable-manager BDF4C0 body");
    void* const manager = c.current_vfs_0109ceec;
    return c.resolve_existing_name_00bdf4c0(c.resolution_context, manager, name);
}
void set_native_material_effect_name_00b18f70(NativeMaterialEffectBaseStorage& effect,
    const void* name, NativeStringStorage& strings) {
    copy_native_string_header_00be0a30_fragment(&effect.name_b8, strings, name);
}

void* load_native_material_effect_00b2ebb0(const void* name, std::uint32_t ignored,
    NativeMaterialEffectLoadingContext& c, NativeMaterialEffectLoadAcquired& a) {
    (void)ignored;
    require(a.phase == NativeMaterialEffectLoadPhase::not_started && !a.creator &&
        !a.raw_slot && !a.programs && !a.fallback && !a.construction,
        "effect load requires a fresh retained caller frame");
    auto& strings = require_domain(c);
    a.phase = NativeMaterialEffectLoadPhase::names;
    try {
        Name original_copy(strings);
        original_copy.copy(name); // Native local+20 is copied and later destroyed.
        Name resolved(strings, a.resolved_name);
        resolved.copy(name);
        {
            Name replacement(strings); replacement.rewrite_operand(".shfx");
            Name search(strings); search.rewrite_operand(".mshd");
            a.native_site = 0x00b2ecce;
            replace_native_particle_string_substrings_004cad40(&resolved.value,
                &search.value, &replacement.value, 0x7fffffffu, strings);
            search.destroy();
            replacement.destroy();
        }
        a.phase = NativeMaterialEffectLoadPhase::resolution;
        a.native_site = 0x00b2ed1e;
        if (resolve_native_material_effect_name(&resolved.value, c)) {
            // Host continuation allocation precedes acquiring native storage;
            // its failure therefore cannot hide an already-created effect.
            a.programs = std::make_unique<NativeMaterialEffectProgramOperation>();
            a.construction = std::make_unique<NativeMaterialEffectConstructionFrame>();
            a.phase = NativeMaterialEffectLoadPhase::allocation;
            a.native_site = 0x00b2ed2c;
            a.raw_slot = ::operator new(sizeof(NativeMaterialEffectStorage));
            a.phase = NativeMaterialEffectLoadPhase::constructor;
            a.native_site = 0x00b2ed43;
            // Source construction can retain a live B319B0 child and its
            // argument storage. Preserve the raw allocation with that frame;
            // native FH3 state4 cleanup is not safe source-frame retirement.
            a.creator = initialize_native_material_effect_00b407a0(a.raw_slot,
                c.construction, *a.construction);
            a.constructor_complete = true;
            a.raw_slot = nullptr; // Completed creator is now the acquisition.
            a.phase = NativeMaterialEffectLoadPhase::programs;
            a.native_site = 0x00b2ed5a;
            auto& effect = *static_cast<NativeMaterialEffectStorage*>(a.creator);
            // A required child can stop with native acquisitions and argument
            // pointers retained in its continuation. Keep this exact8h header
            // and allocation alive on that host failure, rather than leaving
            // those pointers aimed at a destroyed stack local or pooled data.
            resolved.armed = false;
            a.resolved_name_retained = true;
            (void)load_native_material_effect_variants_00b46950(effect, &resolved.value,
                c.programs, *a.programs); // Original AL is deliberately ignored.
            a.resolved_name_retained = false;
            resolved.armed = true;
            a.phase = NativeMaterialEffectLoadPhase::name_assignment;
            a.native_site = 0x00b2ed62;
            set_native_material_effect_name_00b18f70(effect.base, name, strings);
            a.phase = NativeMaterialEffectLoadPhase::registration;
            c.owners.register_completed_creator(a);
        } else {
            {
                Name newline(strings); newline.literal("\n");
                Name prefixed(strings);
                a.native_site = 0x00b2ed8c;
                prefix_native_string_header_0043c130(&prefixed.value,
                    "cannot find shader in shaderfx : ", name, strings);
                prefixed.armed = true;
                Name message(strings);
                a.native_site = 0x00b2eda2;
                concatenate_native_string_headers_004261a0(&prefixed.value,
                    &message.value, &newline.value, strings);
                message.armed = true;
                prefixed.destroy();
                newline.destroy();
                a.native_site = 0x00b2edfd;
                OutputDebugStringA(message.value.data() ? message.value.data() : c.empty_string_0108d5a4);
            }
            Name fallback_name(strings); fallback_name.literal("error.shfx");
            a.fallback = std::make_unique<NativeMaterialEffectCacheAcquired>();
            void* renderer = c.construction.current_renderer_00f8d394;
            require(renderer && current_word(renderer) == 0x00d5f0a8u &&
                c.renderer_profile_00d5f0a8 && c.renderer_profile_00d5f0a8[0x48 / 4] == 0x00b318b0u,
                "effect fallback requires actual current renderer48 B318B0");
            a.phase = NativeMaterialEffectLoadPhase::fallback;
            a.native_site = 0x00b2ee4a;
            a.creator = load_native_renderer_material_effect_00b318b0(renderer,
                &fallback_name.value, *c.cache, a.fallback.get());
            // The fallback result is an owned cache acquisition, not necessarily
            // a fresh creator/count1. Do not register or construct another owner.
            if (a.creator) {
                a.companion = dynamic_cast<NativeMaterialEffectReference*>(
                    &c.owners.actual_owners().resolve_actual(a.creator));
                require(a.companion && &a.companion->storage() == a.creator,
                    "fallback must return the same actual canonical effect owner");
                a.registered = true;
            }
        }
        // Current resolved/original headers are destroyed in native reverse
        // order before the owned return. Pool release is nonthrowing.
        resolved.destroy();
        original_copy.destroy();
        a.phase = NativeMaterialEffectLoadPhase::complete;
        a.returned = true;
        return a.creator;
    } catch (...) {
        a.phase = NativeMaterialEffectLoadPhase::failed;
        throw;
    }
}
} // namespace bsp
