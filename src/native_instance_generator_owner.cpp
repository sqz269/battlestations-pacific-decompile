#include "bsp/native_instance_generator_owner.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
#include <cstring>
#include <list>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native instance-generator ownership requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
using Phase = NativeInstanceGeneratorAcquired::Phase;
static_assert(sizeof(void*) == 4);
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* ptr(U p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p, U n = 0) noexcept { return ptr(bits(p) + n); }
U word(const void* p, U n = 0) noexcept { return *static_cast<const volatile U*>(at(p, n)); }
void put(void* p, U n, U v) noexcept { *static_cast<volatile U*>(at(p, n)) = v; }
std::atomic<std::int32_t>& count(void* p) noexcept { return *static_cast<std::atomic<std::int32_t>*>(at(p, 4)); }
NativeRenderActualOwners& owners(NativeInstanceGeneratorContext& c) { return c.graphics.streams.geometry.actual_owners(); }
ActualNativeStringPoolStorage& strings(NativeInstanceGeneratorContext& c) { return c.graphics.declarations.strings; }
void require(bool condition, const char* message) { if (!condition) throw std::logic_error(message); }
void current_renderer38(const void* r, NativeInstanceGeneratorContext& c) {
    const auto* p = c.graphics.streams.vertices.actual_renderer_profile_00d5f0a8;
    require(word(r) == 0x00d5f0a8u && p && p[0x38 / 4] == 0x00b317e0u,
        "Instance declaration requires current D5F0A8 renderer38 B317E0");
}
void release_field(void* p, U offset, NativeInstanceGeneratorContext& c) {
    if (void* captured = ptr(word(p, offset))) {
        release_native_render_actual_owner(owners(c), captured);
        put(p, offset, 0);
    }
}
void return_captured_name(void* data, U bytes, NativeInstanceGeneratorContext& c) noexcept {
    if (data) strings(c).release(static_cast<char*>(data), bytes);
}
void unwind_name(void* header, NativeInstanceGeneratorContext& c) noexcept {
    destroy_native_string_header_0041dd20(header, strings(c));
}
struct BaseCleanup {
    void* raw;
    ~BaseCleanup() { destroy_native_ref_counted_base_00bd30f0(raw); }
};
void* construct_derived(void* raw, void* section, void* ignored,
    NativeInstanceGeneratorContext& c, NativeInstanceGeneratorAcquired& a, bool building) {
    require(!a.derived_started, "Instance generator construction cannot replay");
    a.derived_started = true; a.phase = Phase::name;
    a.native_site = building ? 0x00b45107 : 0x00b45007;
    resize_native_string_header_0041dd40(&a.temporary_name, strings(c), building ? 41u : 17u, true);
    if (void* data = ptr(word(&a.temporary_name, 4)))
        std::memmove(data, building ? c.building_declaration_00d61c28 : c.generic_declaration_00d61c08,
            word(&a.temporary_name) + 1u);
    a.temporary_name_armed = true; // State0 only after resize/copy completed.
    try {
        a.native_site = building ? 0x00b45143 : 0x00b45043;
        construct_native_instance_generator_declarations_00b55b20(raw, section, ignored, &a.temporary_name, c, a);
    } catch (...) {
        a.temporary_name_armed = false;
        unwind_name(&a.temporary_name, c); a.temporary_name_returned = true;
        throw;
    }
    void* captured = ptr(word(&a.temporary_name, 4));
    a.temporary_name_armed = false; // State2: base only, not temporary again.
    a.native_site = building ? 0x00b45167 : 0x00b45067;
    return_captured_name(captured, word(&a.temporary_name) + 1u, c);
    a.temporary_name_returned = true;
    put(raw, 0, building ? 0x00d61c1cu : 0x00d61bfcu);
    a.derived_complete = true; a.phase = Phase::constructor_complete;
    return raw;
}
} // namespace

void* construct_native_instance_generator_declarations_00b55b20(void* raw,
    void* section, void*, const void* name, NativeInstanceGeneratorContext& c,
    NativeInstanceGeneratorAcquired& a) {
    require(!a.base_started, "Instance generator base construction cannot replay");
    a.base_started = true;
    put(raw, 0, 0x00ceb130); put(raw, 4, 1); put(raw, 0, 0x00d62190);
    put(raw, 8, 0); put(raw, 0xc, 0); put(raw, 0x10, 0); put(raw, 0x14, 0); put(raw, 0x18, 0);
    const void* renderer = c.graphics.streams.vertices.actual_renderer_00f8d394;
    current_renderer38(renderer, c);
    try {
        a.phase = Phase::declaration; a.native_site = 0x00b55b7d;
        a.declaration.reference = load_native_renderer_vertex_declaration_00b317e0(const_cast<void*>(renderer), name, c.graphics.declarations);
        a.captured_declaration = a.declaration.reference;
        if (a.declaration.reference)
            c.graphics.streams.geometry.register_native_declaration_reference(a.declaration, c.graphics.declarations.declarations);
        put(raw, 0x10, bits(a.declaration.reference)); a.declaration.reference = nullptr;
        a.phase = Phase::stream_descriptor; a.native_site = 0x00b55b92;
        NativeMeshSectionLayoutKey key; key.count = 0; // Unused pointer words remain uninitialized.
        void* descriptor = c.layouts.current_stream_descriptor_virtual24(ptr(word(section, 0x3c)));
        void* instance = ptr(word(raw, 0x10));
        key.descriptors[key.count] = descriptor; ++key.count;
        key.descriptors[key.count] = instance; ++key.count;
        a.phase = Phase::layout; a.native_site = 0x00b55bc3;
        a.captured_layout = c.layouts.current_renderer_layout_virtual40(key);
        put(raw, 0x14, bits(a.captured_layout));
    } catch (...) {
        // CC0338 then CC0330. Completed +10/+14 resources are not EH owners.
        unwind_name(at(raw, 8), c); destroy_native_ref_counted_base_00bd30f0(raw);
        throw;
    }
    a.base_complete = true;
    return raw;
}
void* construct_native_generic_instance_generator_00b44fd0(void* raw, void* section, void* ignored,
    NativeInstanceGeneratorContext& c, NativeInstanceGeneratorAcquired& a) { return construct_derived(raw, section, ignored, c, a, false); }
void* construct_native_building_instance_generator_00b450d0(void* raw, void* section, void* ignored,
    NativeInstanceGeneratorContext& c, NativeInstanceGeneratorAcquired& a) { return construct_derived(raw, section, ignored, c, a, true); }

void destroy_native_instance_generator_00b55be0(void* raw, NativeInstanceGeneratorContext& c) {
    put(raw, 0, 0x00d62190); BaseCleanup base{raw};
    try { release_field(raw, 0x18, c); release_field(raw, 0x10, c); release_field(raw, 0x14, c); }
    catch (...) { unwind_name(at(raw, 8), c); throw; }
    void* captured = ptr(word(raw, 0xc)); // Name cleanup disarmed before normal return.
    return_captured_name(captured, word(raw, 8) + 1u, c);
}
void* delete_native_instance_generator_00b55cb0(void* raw, U flags, NativeInstanceGeneratorContext& c) {
    destroy_native_instance_generator_00b55be0(raw, c); if (flags & 1u) singleton_lifetime_free(raw); return raw;
}
void* delete_native_generic_instance_generator_00b450a0(void* raw, U flags, NativeInstanceGeneratorContext& c) {
    put(raw, 0, 0x00d61bfc); destroy_native_instance_generator_00b55be0(raw, c);
    if (flags & 1u) singleton_lifetime_free(raw); return raw;
}
void* delete_native_building_instance_generator_00b451a0(void* raw, U flags, NativeInstanceGeneratorContext& c) {
    put(raw, 0, 0x00d61c1c); destroy_native_instance_generator_00b55be0(raw, c);
    if (flags & 1u) singleton_lifetime_free(raw); return raw;
}
void destroy_native_instance_generator_binding_00b41710(void* raw, NativeInstanceGeneratorContext& c) {
    put(raw, 0, 0x00d619f8); BaseCleanup base{raw}; release_field(raw, 0xc, c);
}
void* delete_native_instance_generator_binding_00b417c0(void* raw, U flags, NativeInstanceGeneratorContext& c) {
    destroy_native_instance_generator_binding_00b41710(raw, c); if (flags & 1u) singleton_lifetime_free(raw); return raw;
}
void set_native_instance_generator_binding_00b41780(void* raw, void* incoming, NativeRenderActualOwners& registry) {
    void* old = ptr(word(raw, 0xc)); if (old == incoming) return;
    put(raw, 0xc, bits(incoming)); if (incoming) count(incoming).fetch_add(1, std::memory_order_seq_cst);
    if (old) release_native_render_actual_owner(registry, old);
}

struct NativeInstanceGeneratorOwners::Impl {
    struct Entry final : RenderCommandReference {
        Impl& domain; void* raw; bool binding; bool registered{};
        Entry(Impl& d, void* p, bool b) : RenderCommandReference(count(p)), domain(d), raw(p), binding(b) {}
        void release_zero_references() noexcept override {
            auto& d = domain; void* p = raw; const bool b = binding;
            const auto profile = word(p); const auto* table = d.table(profile, b);
            if (!table || table[0] != 0x00bd30e0u) std::terminate();
            const auto terminal = table[1];
            if (b && terminal == 0x00b417c0u) delete_native_instance_generator_binding_00b417c0(p, 1, d.context);
            else if (!b && terminal == 0x00b55cb0u) delete_native_instance_generator_00b55cb0(p, 1, d.context);
            else if (!b && terminal == 0x00b450a0u) delete_native_generic_instance_generator_00b450a0(p, 1, d.context);
            else if (!b && terminal == 0x00b451a0u) delete_native_building_instance_generator_00b451a0(p, 1, d.context);
            else std::terminate();
            const auto& r = d.context.graphics.streams.geometry.registration();
            if (registered) r.unbind(r.context, p, *this);
            for (auto i = d.entries.begin(); i != d.entries.end(); ++i)
                if (i->get() == this) { d.entries.erase(i); return; }
            std::terminate();
        }
    };
    NativeInstanceGeneratorContext& context;
    std::list<std::unique_ptr<Entry>> entries;
    const volatile U* table(U profile, bool binding) {
        const auto& p = context.profiles;
        if (binding) return profile == 0x00d619f8u ? p.binding_00d619f8 : nullptr;
        switch (profile) {
        case 0x00d62190: return p.base_00d62190;
        case 0x00d61bfc: return p.generic_00d61bfc;
        case 0x00d61c1c: return p.building_00d61c1c;
        default: return nullptr;
        }
    }
    void register_owner(void* p, bool binding, bool& registered, RenderCommandReference*& companion) {
        const auto& r = context.graphics.streams.geometry.registration();
        require(p && !registered && !companion && r.find && r.bind && r.unbind && !r.find(r.context, p) && table(word(p), binding),
            "Instance generator requires one completed canonical creator identity");
        entries.push_back(std::make_unique<Entry>(*this, p, binding));
        auto& e = *entries.back();
        companion = &e;
        r.bind(r.context, p, e); e.registered = true; registered = true;
    }
};
NativeInstanceGeneratorOwners::NativeInstanceGeneratorOwners(NativeInstanceGeneratorContext& c)
    : impl_(std::make_unique<Impl>(Impl{c, {}})) {}
NativeInstanceGeneratorOwners::~NativeInstanceGeneratorOwners() { if (!impl_->entries.empty()) std::terminate(); }
NativeInstanceGeneratorContext& NativeInstanceGeneratorOwners::context() noexcept { return impl_->context; }
void NativeInstanceGeneratorOwners::register_generator(NativeInstanceGeneratorAcquired& a) {
    require(a.derived_complete, "Generator companion requires completed native construction");
    impl_->register_owner(a.generator, false, a.generator_registered, a.generator_companion);
}
void NativeInstanceGeneratorOwners::register_binding(NativeInstanceGeneratorAcquired& a) {
    impl_->register_owner(a.binding, true, a.binding_registered, a.binding_companion);
}

void attach_native_material_instance_generator_00b451d0(void* effect, void* section, void* ignored,
    NativeInstanceGeneratorOwners& domain, NativeInstanceGeneratorAcquired& a) {
    require(!a.attachment_started && !a.generator && !a.binding, "Instance attachment cannot replay");
    a.attachment_started = true; a.phase = Phase::selection;
    void* header = at(ptr(word(effect, 0xc4)), 0x28);
    if (!word(header)) { a.phase = Phase::complete; return; }
    a.native_site = 0x00b45206;
    const bool building = equal_native_string_header_00425850(header, "building");
    if (!building) {
        a.native_site = 0x00b45250;
        if (!equal_native_string_header_00425850(at(ptr(word(effect, 0xc4)), 0x28), "generic")) {
            a.phase = Phase::complete; return;
        }
    }
    auto& c = domain.context(); a.phase = Phase::allocation;
    a.native_site = building ? 0x00b45211 : 0x00b4525f;
    a.raw_generator = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x1c, 0x1c});
    try {
        if (a.raw_generator) a.generator = building
            ? construct_native_building_instance_generator_00b450d0(a.raw_generator, section, ignored, c, a)
            : construct_native_generic_instance_generator_00b44fd0(a.raw_generator, section, ignored, c, a);
    } catch (...) {
        singleton_lifetime_free(a.raw_generator); a.raw_generator_freed = true; a.raw_generator = nullptr;
        throw; // CBF460/46B: raw allocation only, no completed-object rollback.
    }
    if (!a.generator) { a.phase = Phase::complete; return; }
    a.phase = Phase::generator_registration; domain.register_generator(a);
    a.phase = Phase::binding_allocation; a.native_site = 0x00b4529d;
    a.binding = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x10, 0x10});
    if (a.binding) {
        put(a.binding, 0, 0x00ceb130); put(a.binding, 4, 1); put(a.binding, 0, 0x00d619f8); put(a.binding, 0xc, 0);
        put(a.binding, 8, c.binding_serial_0108fd30); c.binding_serial_0108fd30 = c.binding_serial_0108fd30 + 1u;
        a.phase = Phase::binding_registration; domain.register_binding(a);
    }
    a.phase = Phase::assignment; a.native_site = 0x00b452d3;
    set_native_instance_generator_binding_00b41780(a.binding, a.generator, owners(c));
    a.phase = Phase::generator_release; a.native_site = 0x00b452e2;
    void* generator = a.generator; a.generator = nullptr; release_native_render_actual_owner(owners(c), generator);
    a.phase = Phase::section_publication; a.native_site = 0x00b452f3;
    set_native_mesh_section_instance_generator_binding_00b417e0(*static_cast<NativeMeshSectionStorage*>(section), owners(c), a.binding);
    a.phase = Phase::binding_release; a.native_site = 0x00b45300;
    void* binding = a.binding; a.binding = nullptr;
    if (binding) release_native_render_actual_owner(owners(c), binding);
    a.phase = Phase::complete;
}
void finalize_native_mesh_section_generator_00b85610(void* section, void* ignored,
    NativeInstanceGeneratorOwners& owners, NativeInstanceGeneratorAcquired& acquired) {
    if (void* material = ptr(word(section, 0x20)))
        if (void* effect = ptr(word(material, 0x7c)))
            attach_native_material_instance_generator_00b451d0(effect, section, ignored, owners, acquired);
}
} // namespace bsp
