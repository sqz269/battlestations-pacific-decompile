#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_render_resource_record_array.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/native_vfs_open_route.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_memory_stream.hpp"
#include "bsp/native_physical_stream_open.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/resource_load_events.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <d3dx9tex.h>
#include <cstring>
#include <exception>
#include <list>
#include <new>
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture loading requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);
static_assert(offsetof(NativeRenderResourceRecord, sentinel_0c) == 0xc);
static_assert(offsetof(NativeRenderResourceRecord, resource_28) == 0x28);
struct Name { std::uint32_t length; char* data; };
static_assert(sizeof(Name) == 8);
template<class T> volatile T& field(void* p, std::size_t offset) {
    return *reinterpret_cast<volatile T*>(static_cast<unsigned char*>(p) + offset);
}
template<class T> const volatile T& field(const void* p, std::size_t offset) {
    return *reinterpret_cast<const volatile T*>(static_cast<const unsigned char*>(p) + offset);
}
void* plus(void* p, std::uint32_t offset) {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::int32_t signed_bits(std::uint32_t v) {
    std::int32_t result;
    std::memcpy(&result, &v, 4);
    return result;
}
void copy_name(void* destination, const void* source, ActualNativeStringPoolStorage& strings,
    std::uint32_t* active_site = nullptr, std::uint32_t copy_site = 0) {
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings,
        field<std::uint32_t>(source, 0), true);
    if (field<std::uint32_t>(source, 0) != 0) {
        const auto count = field<std::uint32_t>(destination, 0);
        const auto* input = field<char*>(source, 4);
        auto* output = field<char*>(destination, 4);
        if (active_site) *active_site = copy_site;
        if (count != 0) std::memmove(output, input, count); // BF7680 overlap-safe body.
    }
}
struct NameCleanup {
    void* name;
    ActualNativeStringPoolStorage& strings;
    bool armed = true;
    ~NameCleanup() { if (armed) destroy_native_string_header_0041dd20(name, strings); }
};
struct PersistentNameCleanup {
    NativeTextureCacheName& name;
    ActualNativeStringPoolStorage& strings;
    bool armed{true};
    ~PersistentNameCleanup() { if (armed) name.destroy_current(strings); }
};
bool string_equal(const void* a, const void* b, bool require_length) {
    const auto left = field<std::uint32_t>(a, 0);
    const auto right = field<std::uint32_t>(b, 0);
    if (require_length && left != right) return false;
    if (left == 0) return right == 0;
    if (right == 0) return false;
    return _stricmp(field<char*>(a, 4), field<char*>(b, 4)) == 0;
}
void append_alias(void* list, const void* name, NativeTextureCacheContext& c) {
    auto* const sentinel = field<NativeRenderResourceAliasNode*>(list, 4);
    auto* const previous = field<NativeRenderResourceAliasNode*>(sentinel, 4);
    auto* const node = allocate_native_render_alias_node_004ce6f0(
        sentinel, previous, name, c.strings);
    grow_native_alias_list_count_004ce780(list, 1);
    // Native count failure leaks the allocated but unlinked node.
    field<NativeRenderResourceAliasNode*>(sentinel, 4) = node;
    field<NativeRenderResourceAliasNode*>(field<NativeRenderResourceAliasNode*>(node, 4), 0) = node;
}
void require_slot(void* registry, NativeTextureCacheContext& c,
    unsigned slot, std::uint32_t target) {
    if (field<std::uint32_t>(registry, 0) != 0x00d5f088u ||
        c.registry_vtable_00d5f088 == nullptr ||
        c.registry_vtable_00d5f088[slot] != target)
        throw std::invalid_argument("unimplemented current texture registry slot");
}
void require_domains(NativeTextureCacheContext& c) {
    if (&c.strings != &c.textures.strings || &c.strings != &c.dates.physical.strings ||
        c.textures.cache != &c || &c.textures.current_vfs_0109ceec !=
        &c.dates.physical.manager_0109ceec || &c.synchronization_0108d6dc !=
        &c.textures.synchronization_0108d6dc)
        throw std::invalid_argument("texture cache requires the same actual string/VFS domains");
}
std::uint32_t accounted_size(void* resource, NativeTextureCacheContext& c) {
    const auto profile = field<std::uint32_t>(resource, 0);
    const volatile std::uint32_t* table = nullptr;
    if (profile == 0x00d61948u) table = c.accounting_tables.texture_2d_00d61948;
    if (profile == 0x00d61870u) table = c.accounting_tables.texture_cube_00d61870;
    if (profile == 0x00d618b0u) table = c.accounting_tables.texture_volume_00d618b0;
    const auto expected = profile == 0x00d61948u ? 0x00b3ce30u : 0x00a82250u;
    if (table == nullptr || table[3] != expected)
        throw std::invalid_argument("unimplemented current texture size slot");
    return expected == 0x00b3ce30u ? native_resource_accounted_size_00b3ce30(resource) :
        native_resource_zero_accounted_size_00a82250();
}
void* acquire_result(void* texture, NativeTextureCacheAcquired* acquired) noexcept {
    void* const result = acquire_native_cached_texture_00b31d80(texture);
    if (acquired != nullptr) {
        acquired->result = result;
        acquired->caller_acquired = true;
    }
    return result;
}
struct OperationScope {
    NativeTextureCacheAcquired* acquired;
    explicit OperationScope(NativeTextureCacheAcquired* a) : acquired(a) {
        if (a != nullptr) {
            a->phase = NativeTextureCacheAcquired::Phase::running;
            a->native_site = 0x00b30b68;
        }
    }
    ~OperationScope() {
        if (acquired != nullptr &&
            acquired->phase == NativeTextureCacheAcquired::Phase::running)
            acquired->phase = NativeTextureCacheAcquired::Phase::failed;
    }
    void* finish(void* result) noexcept {
        if (acquired != nullptr) {
            acquired->result = result;
            acquired->phase = NativeTextureCacheAcquired::Phase::complete;
        }
        return result;
    }
};
struct GuardScope {
    NativeTextureCacheContext& context;
    NativeRendererOptionalGuardStorage saved; // Original skipped-entry preimage.
    bool initialized;
    bool armed{true};
    explicit GuardScope(NativeTextureCacheContext& c) : context(c),
        initialized(c.synchronization_0108d6dc.mode_00 != 0) {
        if (initialized) {
            saved.renderer_04 = c.textures.current_renderer_00f8d394;
            saved.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
                saved.renderer_04, c.synchronization_0108d6dc);
        }
    }
    ~GuardScope() noexcept {
        if (armed) {
            if (context.synchronization_0108d6dc.mode_00 != 0 && !initialized)
                std::terminate(); // Native would consume an unwritten guard.
            destroy_native_renderer_optional_guard_00b21110(saved,
                context.synchronization_0108d6dc);
        }
    }
    void finish() {
        armed = false; // B319B0 state -1 precedes the normal leave call.
        if (context.synchronization_0108d6dc.mode_00 != 0) {
            if (!initialized)
                throw std::invalid_argument("texture guard enabled after skipped entry");
            // Only low byte was written; B33B00 ignores the entire argument.
            leave_native_renderer_optional_guard_00b33b00(saved.renderer_04,
                saved.entered_00, context.synchronization_0108d6dc);
        }
    }
};
} // namespace

bool NativeTextureNameResolutionCall::invoke(void* header,
    void* const volatile& current_manager, void* factory_context,
    MakeNativeTextureNameResolutionOperation make) {
    if (operation || factory_started || started || failed)
        throw std::logic_error("texture name resolution child cannot replay");
    factory_started = true;
    try {
        if (!make)
            throw std::invalid_argument("texture resolution requires an owned actual BDF4C0 operation");
        operation = make(factory_context);
        if (!operation)
            throw std::invalid_argument("texture resolution factory returned no owned operation");
        // Publish child first; capture the native global only at the call site.
        manager = current_manager;
        name = header;
        started = true;
        result = operation->resolve(manager, name);
        returned = true;
        return result;
    } catch (...) {
        failed = true;
        throw;
    }
}
void NativeTextureCacheName::initialize() {
    if (initialized)
        throw std::logic_error("texture cache name header cannot replay");
    initialized = true;
    field<std::uint32_t>(&value, 0) = 0;
    field<char*>(&value, 4) = nullptr;
}
void NativeTextureCacheName::destroy_current(ActualNativeStringPoolStorage& strings) noexcept {
    if (!cleanup_armed) return;
    // Same current-data/current-length schedule as actual41DD20. Preserve the
    // header; released_data/size are diagnostics, not another owning header.
    destroy_captured(field<char*>(&value, 4), strings);
}
void NativeTextureCacheName::destroy_captured(char* captured,
    ActualNativeStringPoolStorage& strings) noexcept {
    if (!cleanup_armed) return;
    cleanup_armed = false;
    released = true;
    released_data = captured;
    if (captured) {
        released_size = field<std::uint32_t>(&value, 0) + 1u;
        strings.release(captured, released_size);
    }
}

NativeTextureLoadAcquired::~NativeTextureLoadAcquired() {
    if (phase != Phase::not_started && phase != Phase::complete) std::terminate();
}
NativeTextureCacheAcquired::NativeTextureCacheAcquired() = default;
NativeTextureCacheAcquired::~NativeTextureCacheAcquired() {
    if (phase != Phase::not_started && phase != Phase::complete) std::terminate();
}

struct NativeTextureLoadOwners::Impl {
    struct Entry final : RenderCommandReference {
        Impl& domain;
        void* raw;
        bool registered{};
        Entry(Impl& d, void* p) : RenderCommandReference(
            *reinterpret_cast<std::atomic<std::int32_t>*>(plus(p, 4))), domain(d), raw(p) {}
        void release_zero_references() noexcept override {
            auto& d = domain;
            const auto& r = d.geometry.registration();
            if (field<std::uint32_t>(raw, 0) != 0x00d61948u ||
                d.profile[0] != 0x00bd30e0u || d.profile[1] != 0x00b3f590u)
                std::terminate();
            try { delete_native_texture_2d_00b3f590(raw, 1, d.context); }
            catch (...) { std::terminate(); }
            if (registered) r.unbind(r.context, raw, *this);
            for (auto i = d.entries.begin(); i != d.entries.end(); ++i) {
                if (i->get() == this) { d.entries.erase(i); return; }
            }
            std::terminate();
        }
    };
    GuiNativeGeometryOwners& geometry;
    NativeTexture2DOwnerContext& context;
    const volatile std::uint32_t* profile;
    std::list<std::unique_ptr<Entry>> entries;
};
NativeTextureLoadOwners::NativeTextureLoadOwners(GuiNativeGeometryOwners& geometry,
    NativeTexture2DOwnerContext& context, const volatile std::uint32_t* profile)
    : impl_(std::make_unique<Impl>(Impl{geometry, context, profile, {}})) {
    const auto& r = geometry.registration();
    if (!r.find || !r.bind || !r.unbind || !profile ||
        profile[0] != 0x00bd30e0u || profile[1] != 0x00b3f590u)
        throw std::invalid_argument("texture companions require the actual canonical registration/terminal");
}
NativeTextureLoadOwners::~NativeTextureLoadOwners() {
    if (!impl_->entries.empty()) std::terminate();
}
NativeTexture2DOwnerContext& NativeTextureLoadOwners::texture_context() noexcept {
    return impl_->context;
}
void NativeTextureLoadOwners::register_completed_creator(NativeTextureLoadAcquired& a) {
    auto& s = *impl_;
    const auto& r = s.geometry.registration();
    if (!a.creator || !a.constructor_complete || a.owner_record || a.companion || a.registered ||
        field<std::uint32_t>(a.creator, 0) != 0x00d61948u || r.find(r.context, a.creator))
        throw std::invalid_argument("new texture creator requires exactly one canonical identity");
    auto entry = std::make_unique<Impl::Entry>(s, a.creator);
    s.entries.push_back(std::move(entry));
    auto& e = *s.entries.back();
    a.owner_record = &e;
    a.companion = &e;
    r.bind(r.context, a.creator, e); // Transactional metadata only, no retain.
    e.registered = true;
    a.registered = true;
}

void append_native_texture_record_00b30130(void* header,
    const NativeRenderResourceRecord& source, NativeTextureCacheContext& c) {
    const auto capacity = field<std::uint32_t>(header, 8);
    if (field<std::uint32_t>(header, 4) == capacity) {
        const auto doubled = capacity + capacity;
        reserve_native_render_resource_record_array_00b2ff00(header,
            signed_bits(doubled) > 64 ? doubled : 64u, c.strings, c.validation);
    }
    auto* const p = plus(field<void*>(header, 0), field<std::uint32_t>(header, 4) * 0x2cu);
    if (p != nullptr) {
        auto* const destination = ::new (p) NativeRenderResourceRecord;
        try { copy_construct_native_render_resource_record_00b2fc60(
            *destination, source, c.strings, c.validation); }
        catch (...) {
            // CBDA1C only computes a pointer for no-op placement delete401130.
            (void)plus(field<void*>(header, 0), field<std::uint32_t>(header, 4) * 0x2cu);
            throw;
        }
    }
    field<std::uint32_t>(header, 4) = field<std::uint32_t>(header, 4) + 1u;
}
void* acquire_native_cached_texture_00b31d80(void* texture) noexcept {
    InterlockedIncrement(&field<LONG>(texture, 4));
    return texture;
}
namespace {
struct CacheHelperScope {
    NativeTextureCacheAcquired& acquired;
    bool owns_phase;
    explicit CacheHelperScope(NativeTextureCacheAcquired& a) : acquired(a),
        owns_phase(a.phase == NativeTextureCacheAcquired::Phase::not_started) {
        if (!owns_phase && a.phase != NativeTextureCacheAcquired::Phase::running)
            throw std::logic_error("texture cache helper cannot replay a completed/failed frame");
        if (owns_phase) a.phase = NativeTextureCacheAcquired::Phase::running;
    }
    ~CacheHelperScope() {
        if (owns_phase && acquired.phase == NativeTextureCacheAcquired::Phase::running)
            acquired.phase = NativeTextureCacheAcquired::Phase::failed;
    }
    void finish() noexcept {
        if (owns_phase) acquired.phase = NativeTextureCacheAcquired::Phase::complete;
    }
};
}
void initialize_native_texture_fallback_00b31bd0(void* registry,
    NativeTextureCacheContext& c, NativeTextureCacheAcquired& a) {
    if (a.fallback || a.fallback_initialized || a.fallback_name ||
        a.fallback_resolution.operation || a.fallback_resolution.factory_started)
        throw std::logic_error("texture fallback initialization cannot replay");
    CacheHelperScope helper{a};
    // Allocate the host continuation before any native initialization effect.
    a.fallback = std::make_unique<NativeTextureCacheAcquired>();
    auto* const name = plus(registry, 0x14);
    a.fallback_name = name;
    a.native_site = 0x00b31bdd;
    resize_native_string_header_0041dd40(name, c.strings, 9, false);
    if (field<char*>(name, 4))
        std::memcpy(field<char*>(name, 4), "error.tga", field<std::uint32_t>(name, 0));
    field<std::uint8_t>(registry, 0x1c) = 1;
    a.fallback_initialized = true;
    a.native_site = 0x00b31c05;
    (void)a.fallback_resolution.invoke(name, c.textures.current_vfs_0109ceec,
        c.textures.resolution_context, c.textures.make_resolution_operation);
    a.native_site = 0x00b31c13;
    auto* const result = load_native_cached_texture_00b30b40(
        registry, name, 0, 1, 1, c, a.fallback.get());
    field<void*>(registry, 0x20) = result;
    helper.finish();
}
void* resolve_native_texture_cache_name_00b31c20(void* registry, void* output,
    const void* name, NativeTextureCacheContext& c, NativeTextureCacheAcquired* a) {
    if (!a || output != &a->names.resolver_output.value)
        throw std::invalid_argument("texture resolution requires its persistent acquired output header");
    if (a->resolver_started || a->names.resolver_output.initialized ||
        a->resolution.operation || a->resolution.factory_started)
        throw std::logic_error("texture cache resolver cannot replay its frame");
    CacheHelperScope helper{*a};
    a->resolver_started = true;
    if (field<std::uint8_t>(registry, 0x1c) == 0) {
        initialize_native_texture_fallback_00b31bd0(registry, c, *a);
    }
    auto& retained_output = a->names.resolver_output;
    retained_output.initialize();
    copy_name(output, name, c.strings);
    retained_output.cleanup_armed = true; // B31C8F: AFTER completed initial copy.
    PersistentNameCleanup cleanup{retained_output, c.strings};
    a->native_site = 0x00b31c97;
    if (!a->resolution.invoke(output, c.textures.current_vfs_0109ceec,
        c.textures.resolution_context, c.textures.make_resolution_operation)) {
        // Native4254B0 is RET. Preserve its diagnostic operand read, no logger.
        auto* const original = field<char*>(name, 4);
        const char* const diagnostic = original ? original : c.textures.empty_string_0108d5a4;
        (void)diagnostic;
        copy_name(output, plus(registry, 0x14), c.strings);
    }
    cleanup.armed = false;
    helper.finish();
    return output;
}

namespace {
std::uintptr_t current_table(const void* owner) {
    return field<std::uint32_t>(owner, 0);
}
std::uint32_t table_slot(std::uintptr_t table, unsigned offset) {
    return *reinterpret_cast<const volatile std::uint32_t*>(table + offset);
}
void require_loading_domains(NativeTextureLoadingContext& c) {
    auto& owner = c.owners.texture_context();
    auto& notification = owner.renderer_notification;
    if (!c.cache || &c.cache->textures != &c || &c.strings != &c.opens.physical.strings ||
        &c.current_vfs_0109ceec != &c.opens.physical.manager_0109ceec ||
        c.opens.native_bindings != &c.streams || &c.conversion.memory_owners != &owner.retained_memory ||
        notification.actual_native_string_pool != &c.strings ||
        &notification.actual_string_storage != &c.strings ||
        owner.surfaces.actual_string_pool_00419cc0.actual_storage() != &c.strings ||
        &notification.actual_renderer_00f8d394 != &c.current_renderer_00f8d394 ||
        reinterpret_cast<const volatile void*>(&owner.surfaces.actual_renderer_00f8d394) !=
        reinterpret_cast<const volatile void*>(&c.current_renderer_00f8d394) ||
        &notification.synchronization != &c.synchronization_0108d6dc ||
        !c.image_info_00c2dfec || !c.create_texture_00c2dfe6)
        throw std::invalid_argument("texture loading requires the same actual strings/VFS/renderer/owner domains");
    require_domains(*c.cache);
}
void release_stream(void*& stream, bool& started, NativeTextureLoadingContext& c) {
    auto* const captured = stream;
    started = true;
    if (InterlockedDecrement(&field<LONG>(captured, 4)) == 0)
        c.streams.zero_reference(current_table(captured), captured);
    stream = nullptr;
}
void delete_invalid_stream(void*& stream, bool& started, NativeTextureLoadingContext& c) {
    auto* const captured = stream;
    const auto table = current_table(captured);
    const auto target = table_slot(table, 4);
    if (table == 0x00d691b0u && target == 0x00bf5090u) {
        started = true;
        delete_native_physical_stream_00bf5090(captured, 1);
    } else if (table == 0x00d642c0u && target == 0x00bb8f90u) {
        started = true;
        delete_native_memory_stream_00bb8f90(captured, 1, c.conversion.memory_owners);
    } else if (table == 0x00d68db0u && target == 0x00bf1240u) {
        started = true;
        delete_native_adopted_substream_00bf1240(captured, 1, c.streams);
    } else throw std::invalid_argument("unimplemented current invalid-stream deleting slot");
    stream = nullptr; // Native direct deleting+4, NOT a reference decrement.
}
std::uint32_t current_memory_size(void* memory, NativeTextureLoadingContext& c) {
    return static_cast<std::uint32_t>(c.streams.length(current_table(memory), memory));
}
bool native_substring(const void* header, const char* needle) {
    const auto* const input = field<char*>(header, 4);
    if (!input) return false;
    const auto* const found = std::strstr(input, needle);
    if (!found) return false;
    // B2C492/B2C4B3 reload original.data AFTER the CRT call.
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(found) -
        reinterpret_cast<std::uintptr_t>(field<char*>(header, 4))) != 0xffffffffu;
}
std::uint32_t signed_minimum_one(std::uint32_t bits) {
    return signed_bits(bits) > 1 ? bits : 1u;
}
}

void* load_native_texture_2d_00b2c2d0(const void* original, std::uint32_t callback,
    NativeTextureLoadingContext& c, NativeTextureLoadAcquired& a) {
    if (a.phase != NativeTextureLoadAcquired::Phase::not_started)
        throw std::logic_error("texture loader cannot replay a retained operation");
    require_loading_domains(c);
    a.phase = NativeTextureLoadAcquired::Phase::running;
    try {
        a.native_site = 0x00b2c30f;
        copy_name(&a.name, original, c.strings, &a.native_site, 0x00b2c327);
        a.name_retained = true;
        a.native_site = 0x00b2c345;
        auto* const manager = c.current_vfs_0109ceec;
        a.source = c.streams.open_manager_entry(table_slot(current_table(manager), 4), manager, &a.name, 2);
        a.native_site = 0x00b2c350;
        if (!c.streams.is_open(current_table(a.source), a.source)) {
            a.native_site = 0x00b2c35f;
            delete_invalid_stream(a.source, a.source_release_started, c);
            destroy_native_string_header_0041dd20(&a.name, c.strings);
            a.name_retained = false;
            a.phase = NativeTextureLoadAcquired::Phase::complete;
            return nullptr;
        }
        a.native_site = 0x00b2c39f;
        a.memory = convert_native_stored_stream_00bef750(a.source, c.conversion);
        a.native_site = 0x00b2c3ae;
        release_stream(a.source, a.source_release_started, c);
        a.captured_device = field<IDirect3DDevice9*>(c.current_renderer_00f8d394, 0x1a10);
        D3DXIMAGE_INFO info; // Required provider writes actual image output.
        const auto bytes = current_memory_size(a.memory, c);
        auto* const data = native_memory_stream_data_00bef610(a.memory, nullptr);
        a.native_site = 0x00b2c3ef;
        a.last_hresult = c.image_info_00c2dfec(data, bytes, &info);
        // Native ignores HRESULT and may consume uninitialized stack bytes.
        // That input has no defined C++ translation; retain the actual source.
        if (FAILED(a.last_hresult))
            throw std::invalid_argument("unwritten D3DX image-info output is outside the source domain");
        a.resource_type = static_cast<std::uint32_t>(info.ResourceType);
        if (info.ResourceType != D3DRTYPE_TEXTURE)
            throw std::invalid_argument("B2C2D0 cube/volume named-owner arms require B3CED0/B3CFA0");
        std::uint32_t width = 0xffffffffu, height = 0xffffffffu;
        auto saved_width = info.Width;
        auto saved_height = info.Height;
        auto mips = info.MipLevels;
        GuardScope guard{*c.cache};
        if (mips > 1u && field<std::uint32_t>(c.current_renderer_00f8d394, 0x1d84) != 0) {
            NativeString detail;
            detail.assign_0041e870(c.strings, "detail.dds");
            NameCleanup detail_cleanup{&detail, c.strings};
            bool reduce = !string_equal(original, &detail, false);
            if (reduce) {
                reduce = !native_substring(original, "noseart") &&
                    !native_substring(original, "interface/textures/gui/units");
            }
            detail_cleanup.armed = false;
            destroy_native_string_header_0041dd20(&detail, c.strings);
            if (reduce) {
                const auto quality = field<std::uint32_t>(c.current_renderer_00f8d394, 0x1d84);
                width = saved_width = signed_minimum_one(saved_width >> (quality & 31u));
                height = saved_height = signed_minimum_one(saved_height >> (quality & 31u));
                mips = signed_minimum_one(mips - quality);
            }
        }
        const auto format = info.Format == D3DFMT_R8G8B8 ? D3DFMT_A8R8G8B8 : D3DFMT_UNKNOWN;
        const auto create = [&]() {
            const auto size = current_memory_size(a.memory, c);
            auto* const memory_data = native_memory_stream_data_00bef610(a.memory, nullptr);
            return c.create_texture_00c2dfe6(a.captured_device, memory_data, size,
                width, height, mips, 0, format, D3DPOOL_MANAGED, 0x70004u,
                0xffffffffu, 0, nullptr, nullptr, &a.texture);
        };
        a.native_site = 0x00b2c565;
        a.last_hresult = create();
        if (!a.texture && a.last_hresult != 0 &&
            static_cast<std::uint32_t>(a.last_hresult) != 0x8876017cu &&
            static_cast<std::uint32_t>(a.last_hresult) != 0x8007000eu) {
            a.native_site = 0x00b2c595;
            if (!c.retry_device_00b29670)
                throw std::invalid_argument("texture retry requires actual B29670 device recovery");
            c.retry_device_00b29670(c.callback_context, c.current_renderer_00f8d394);
            a.native_site = 0x00b2c5cf;
            a.last_hresult = create(); // SAME captured device, fresh stream reads.
        }
        if (a.texture) {
            a.native_site = 0x00b2c5e0;
            a.raw_slot = allocate_d3d9_texture2d_slot_00b3f2b0();
            if (!a.raw_slot)
                throw std::invalid_argument("null texture slot reaches native null+4C store");
            a.native_site = 0x00b2c60d;
            a.creator = construct_native_texture_2d_00b3f930(a.raw_slot, original,
                a.texture, saved_width, saved_height, 0, c.owners.texture_context());
            a.constructor_complete = true; // Publish before any later call.
            a.native_site = 0x00b2c624;
            assign_native_retained_memory_slot_00b23640(plus(a.creator, 0x4c),
                &a.memory, c.conversion.memory_owners);
            a.source_assigned = true;
            field<std::uint32_t>(a.creator, 0x3c) = mips;
            c.owners.register_completed_creator(a);
            if (callback != 0) {
                a.native_site = 0x00b2c636;
                if (!c.post_load_callback)
                    throw std::invalid_argument("nonzero texture loader word requires its concrete ECX callback");
                a.callback_started = true;
                c.post_load_callback(c.callback_context, callback, a.creator);
            }
            a.native_site = 0x00b2c643;
            const auto size = current_memory_size(a.memory, c);
            field<std::uint32_t>(a.creator, 0x24) = size;
        }
        a.native_site = 0x00b2c82e;
        guard.finish();
        a.native_site = 0x00b2c837;
        release_stream(a.memory, a.memory_release_started, c);
        destroy_native_string_header_0041dd20(&a.name, c.strings);
        a.name_retained = false;
        a.phase = NativeTextureLoadAcquired::Phase::complete;
        return a.texture ? a.creator : nullptr;
    } catch (...) {
        a.phase = NativeTextureLoadAcquired::Phase::failed;
        throw;
    }
}

void* load_native_cached_texture_00b30b40(void* registry, const void* name,
    std::uint32_t loader_word, std::uint8_t acquire_new, std::uint8_t allow_load,
    NativeTextureCacheContext& c, NativeTextureCacheAcquired* acquired) {
    require_domains(c);
    if (acquired != nullptr && acquired->phase != NativeTextureCacheAcquired::Phase::not_started)
        throw std::logic_error("texture cache operation cannot replay");
    OperationScope operation{acquired};
    pump_resource_load_events_00beccd0(*c.platform_0109cf04);
    // Scratch is admitted only for the first-search hot path. Any path that
    // exports a header to persistent resolution requires the caller's storage.
    NativeTextureCacheNames scratch;
    auto& names = acquired ? acquired->names : scratch;
    auto& normalized = names.normalized.value;
    names.normalized.initialize();
    copy_name(&normalized, name, c.strings);
    names.normalized.cleanup_armed = true;
    PersistentNameCleanup normalized_cleanup{names.normalized, c.strings};
    normalize_native_resource_path_header_00bee690(&normalized, c.strings);
    void* cached = nullptr;
    const auto count = field<std::uint32_t>(registry, 8);
    auto* row = field<void*>(registry, 4);
    auto* const end = plus(row, count * 0x2cu);
    while (row != end) {
        auto* const captured_sentinel = field<NativeRenderResourceAliasNode*>(row, 0xc);
        auto* node = field<NativeRenderResourceAliasNode*>(captured_sentinel, 0);
        while (node != captured_sentinel) {
            if (node == field<NativeRenderResourceAliasNode*>(row, 0xc))
                c.validation.invalid_parameter(c.validation.context);
            if (string_equal(plus(node, 8), &normalized, true)) {
                cached = field<void*>(row, 0x28);
                break;
            }
            if (node == field<NativeRenderResourceAliasNode*>(row, 0xc))
                c.validation.invalid_parameter(c.validation.context);
            node = field<NativeRenderResourceAliasNode*>(node, 0);
        }
        if (cached != nullptr) break;
        row = plus(row, 0x2c);
    }
    auto& resolved = names.resolved.value;
    names.resolved.initialize();
    names.resolved.cleanup_armed = true;
    PersistentNameCleanup resolved_cleanup{names.resolved, c.strings};
    if (cached != nullptr) {
        require_slot(registry, c, 3, 0x00b31d80);
        if (acquired != nullptr) acquired->native_site = 0x00b30ee6;
        return operation.finish(acquire_result(cached, acquired));
    }
    if (!acquired)
        throw std::invalid_argument("texture name resolution requires retained acquired output");
    auto& output = names.resolver_output.value;
    require_slot(registry, c, 1, 0x00b31c20);
    auto* const returned = resolve_native_texture_cache_name_00b31c20(
        registry, &output, &normalized, c, acquired);
    {
        PersistentNameCleanup output_cleanup{names.resolver_output, c.strings};
        copy_name(&resolved, returned, c.strings);
    }
    normalize_native_resource_path_header_00bee690(&resolved, c.strings);
    if (!string_equal(&resolved, &normalized, false)) {
        const auto resolved_count = field<std::uint32_t>(registry, 8);
        row = field<void*>(registry, 4);
        auto* const resolved_end = plus(row, resolved_count * 0x2cu);
        while (row != resolved_end) {
            auto* const sentinel = field<NativeRenderResourceAliasNode*>(row, 0xc);
            auto* const first = field<NativeRenderResourceAliasNode*>(sentinel, 0);
            if (first == sentinel) c.validation.invalid_parameter(c.validation.context);
            // Only the first alias participates in this SECOND search.
            if (string_equal(plus(first, 8), &resolved, true)) {
                append_alias(plus(row, 8), &normalized, c);
                cached = field<void*>(row, 0x28);
                break;
            }
            row = plus(row, 0x2c);
        }
        if (cached != nullptr) {
            require_slot(registry, c, 3, 0x00b31d80);
            if (acquired != nullptr) acquired->native_site = 0x00b30ee6;
            return operation.finish(acquire_result(cached, acquired));
        }
    }
    if (allow_load == 0) return operation.finish(nullptr);
    require_slot(registry, c, 2, 0x00b2c2d0);
    if (acquired == nullptr)
        throw std::invalid_argument("cold texture load requires retained acquired output");
    acquired->native_site = 0x00b30dbd;
    void* const loaded = load_native_texture_2d_00b2c2d0(
        &resolved, loader_word, c.textures, acquired->loader);
    acquired->loaded = loaded;
    acquired->native_site = 0x00b30dd2;
    auto& record = acquired->pending_record; // Native leaves +08/+28 unset here.
    acquired->record_initialized = true;
    record.name_length_00 = 0;
    record.name_data_04 = nullptr;
    {
        NameCleanup name_cleanup{&record, c.strings};
        record.sentinel_0c = allocate_native_render_alias_sentinel_004c3020();
        record.alias_count_10 = 0;
        for (unsigned i = 5; i != 0; --i) record.payload_14_24[i - 1] = 0;
        name_cleanup.armed = false;
    }
    acquired->record_cleanup_armed = true;
    try {
        copy_name(&record, &resolved, c.strings);
        append_alias(plus(&record, 8), &resolved, c);
        std::uint32_t date[5];
        acquired->native_site = 0x00b30e63;
        auto* const returned_date = query_native_vfs_file_date_00bdd340(
            c.dates.physical.manager_0109ceec, date, &resolved, c.dates);
        for (unsigned i = 0; i != 5; ++i)
            record.payload_14_24[i] = field<std::uint32_t>(returned_date, i * 4u);
        if (!string_equal(&normalized, &resolved, false)) append_alias(plus(&record, 8), &normalized, c);
        record.resource_28 = loaded;
        acquired->native_site = 0x00b30f4a;
        append_native_texture_record_00b30130(plus(registry, 4), record, c);
        acquired->cache_published = true;
        if (loaded != nullptr) {
            acquired->native_site = 0x00b30f5a;
            const auto size = accounted_size(loaded, c);
            field<std::uint32_t>(registry, 0x10) = field<std::uint32_t>(registry, 0x10) + size;
        }
        void* result = loaded;
        if (acquire_new != 0 && loaded != nullptr) {
            require_slot(registry, c, 3, 0x00b31d80);
            acquired->native_site = 0x00b30f75;
            result = acquire_result(loaded, acquired);
        }
        acquired->record_cleanup_armed = false; // Native state1 before B2F990.
        destroy_native_render_resource_record_00b2f990(record, c.strings);
        acquired->record_destroyed = true;
        return operation.finish(result);
    } catch (...) {
        if (acquired->record_cleanup_armed) {
            acquired->record_cleanup_armed = false;
            destroy_native_render_resource_record_00b2f990(record, c.strings);
            acquired->record_destroyed = true;
        }
        throw;
    }
}

void* load_native_renderer_texture_00b319b0(void* renderer,
    const void* name, std::uint32_t post_load_word, NativeTextureCacheContext& c,
    NativeTextureCacheAcquired* acquired) {
    require_domains(c);
    if (acquired && (acquired->phase != NativeTextureCacheAcquired::Phase::not_started ||
        acquired->wrapper_started))
        throw std::logic_error("texture renderer operation cannot replay");
    if (acquired) acquired->wrapper_started = true;
    try {
    GuardScope guard{c};
    NativeTextureCacheName scratch;
    auto& retained_lower = acquired ? acquired->names.wrapper : scratch;
    retained_lower.initialize();
    auto& lower = retained_lower.value;
    copy_name(&lower, name, c.strings);
    char* const captured_data = field<char*>(&lower, 4);
    retained_lower.cleanup_armed = true;
    PersistentNameCleanup cleanup{retained_lower, c.strings};
    lowercase_native_string_header_004bcc00(&lower);
    auto* const result = load_native_cached_texture_00b30b40(
        plus(renderer, 0x1a74), &lower, post_load_word, 0, 1, c, acquired);
    cleanup.armed = false;
    retained_lower.destroy_captured(captured_data, c.strings);
    guard.finish();
    return result;
    } catch (...) {
        if (acquired != nullptr)
            acquired->phase = NativeTextureCacheAcquired::Phase::failed;
        throw;
    }
}
} // namespace bsp
