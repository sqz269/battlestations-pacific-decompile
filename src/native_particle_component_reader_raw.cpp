#include "bsp/native_particle_component_reader_raw.hpp"

#include "bsp/native_effect_component_reader.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_particle_component_lifetime.hpp"
#include "bsp/native_particle_resource_acquisition_raw.hpp"
#include "bsp/native_particle_resource_cache.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstring>
#include <exception>
#include <stdexcept>
#include <vector>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle component reading requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& word(void* base, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<unsigned char*>(base) + offset);
}
void* pointer(void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
} // namespace

struct NativeParticleComponentReaderRawAcquired::Impl {
    alignas(4) unsigned char name[8];
    NativeLuaObjectStorage particle;
    NativeLuaObjectStorage key;
    NativeLuaObjectStorage value; // Same native local as UnderWater.
    // These own source invocation frames only; component+28 is the game vector.
    std::vector<std::unique_ptr<NativeParticleResourceAcquisitionRawAcquired>> children;
    NativeParticleComponentReaderRawPhase phase = NativeParticleComponentReaderRawPhase::fresh;
    Word site = 0, failure = 0;
    std::int32_t state = -1, failed_state = -1;
    std::int32_t parser_builder_kind;
    explicit Impl(std::int32_t kind) : parser_builder_kind(kind) {}

    void cleanup(NativeStringRawPoolContext& strings) noexcept {
        try {
            if (state == 5) { state = 0; destroy_native_string_header_0041dd20(name, strings); }
            if (state == 4) { state = 3; destroy_native_string_header_0041dd20(name, strings); }
            if (state == 3) { state = 2; destroy_native_lua_object_00b67700(value); }
            if (state == 2) { state = 0; destroy_native_lua_object_00b67700(key); }
            if (state == 1) { state = 0; destroy_native_lua_object_00b67700(value); }
            if (state == 0) { state = -1; destroy_native_lua_object_00b67700(particle); }
        } catch (...) { std::terminate(); }
    }

    void* acquire(NativeParticleComponentReaderRawContext& context, bool table) {
        site = table ? 0x00871e1e : 0x00871ef1;
        void* const owner = get_native_particle_resource_cache_owner_00871bd0(context.cache);
        children.push_back(std::make_unique<NativeParticleResourceAcquisitionRawAcquired>(parser_builder_kind));
        site = table ? 0x00871e31 : 0x00871f04;
        return acquire_native_particle_resource_00870dd0(
            static_cast<unsigned char*>(owner) + 4, name, 0, 0, 1,
            context.acquisition, *children.back());
    }

    void return_name(void* captured_data, Word captured_length,
        NativeStringRawPoolContext& strings, bool table) {
        if (!captured_data) return;
        site = table ? 0x00871e48 : 0x00871f23;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
        site = table ? 0x00871e4f : 0x00871f2a;
        return_native_string_pool_00bd1510(pool, captured_data, captured_length + 1u,
            strings.actual_small_returns_disabled_01090aa4);
    }

    void append(void* header, void* resource, std::uint8_t underwater, bool table) {
        site = table ? 0x00871e5b : 0x00871f36;
        set_native_particle_resource_underwater_00af3e10(resource, underwater);
        const Word capacity = word(header, 8);
        if (word(header, 4) == capacity) {
            Word growth = capacity + capacity;
            if (static_cast<std::int32_t>(growth) <= 1) growth = 1;
            site = table ? 0x00871e77 : 0x00871f55;
            reserve_native_particle_component_resources_0086a4d0(header,
                static_cast<std::int32_t>(growth));
        }
        const Word count = word(header, 4);
        const Word data = word(header);
        const Word destination = data + count * 4u;
        if (destination) word(reinterpret_cast<void*>(destination)) = reinterpret_cast<Word>(resource);
        word(header, 4) = word(header, 4) + 1u;
    }
};

NativeParticleComponentReaderRawAcquired::NativeParticleComponentReaderRawAcquired(std::int32_t kind)
    : impl_(std::make_unique<Impl>(kind)) {}
NativeParticleComponentReaderRawAcquired::~NativeParticleComponentReaderRawAcquired() = default;
NativeParticleComponentReaderRawPhase NativeParticleComponentReaderRawAcquired::phase() const noexcept { return impl_->phase; }
Word NativeParticleComponentReaderRawAcquired::failure_site() const noexcept { return impl_->failure; }
std::int32_t NativeParticleComponentReaderRawAcquired::native_state_at_failure() const noexcept { return impl_->failed_state; }
std::size_t NativeParticleComponentReaderRawAcquired::acquisition_count() const noexcept { return impl_->children.size(); }
const NativeParticleResourceAcquisitionRawAcquired*
NativeParticleComponentReaderRawAcquired::acquisition_invocation(std::size_t index) const noexcept {
    return index < impl_->children.size() ? impl_->children[index].get() : nullptr;
}

void read_native_particle_component_00871d00(void* component, NativeLuaObjectStorage& definition,
    NativeParticleComponentReaderRawContext& context, NativeParticleComponentReaderRawAcquired& acquired) {
    auto& f = *acquired.impl_;
    if (f.phase != NativeParticleComponentReaderRawPhase::fresh)
        throw std::logic_error("Particle component reader invocation cannot replay");
    f.phase = NativeParticleComponentReaderRawPhase::running;
    auto& strings = context.acquisition.strings;
    try {
        f.site = 0x00871d22;
        read_native_effect_component_base_00868bf0(component, definition, context.actual_crt_sse2_conversion);
        f.site = 0x00871d33;
        native_lua_get_by_name_protected(definition, &f.particle, "Particle");
        f.state = 0;
        f.site = 0x00871d4c;
        auto* const under = native_lua_get_by_name_protected(definition, &f.value, "UnderWater");
        f.state = 1;
        f.site = 0x00871d58;
        const std::uint8_t underwater = native_lua_is_boolean_00b66000(*under) ? 1 : 0;
        f.state = 0;
        f.site = 0x00871d6a;
        destroy_native_lua_object_00b67700(f.value);
        f.site = 0x00871d73;
        if (native_lua_is_table_00b661b0(f.particle)) {
            f.site = 0x00871d84;
            construct_native_lua_object_00b65f50(&f.key);
            f.state = 2;
            f.site = 0x00871d92;
            construct_native_lua_object_00b65f50(&f.value);
            f.state = 3;
            f.site = 0x00871daa;
            native_lua_iterate_first_protected(f.particle, f.key, f.value);
            f.site = 0x00871db8;
            if (!native_lua_is_unbound_00b66420(f.key)) {
                void* const header = static_cast<unsigned char*>(component) + 0x28;
                do {
                    f.site = 0x00871dd4;
                    const char* const text = native_lua_string_protected(f.value);
                    word(f.name) = 0;
                    word(f.name, 4) = 0;
                    const Word length = static_cast<Word>(std::strlen(text));
                    f.site = 0x00871dfa;
                    resize_native_string_header_0041dd40(f.name, strings, length, true);
                    void* const data = pointer(f.name, 4);
                    const Word current_length = word(f.name);
                    if (data) {
                        f.site = 0x00871e11;
                        std::memmove(data, text, current_length + 1u);
                    }
                    f.state = 4;
                    void* const resource = f.acquire(context, true);
                    f.state = 3;
                    f.return_name(data, current_length, strings, true);
                    f.append(header, resource, underwater, true);
                    f.site = 0x00871e9c;
                    native_lua_iterate_next_protected(f.particle, f.key, f.value);
                    f.site = 0x00871eaa;
                } while (!native_lua_is_unbound_00b66420(f.key));
            }
            f.state = 2;
            f.site = 0x00871ec1;
            destroy_native_lua_object_00b67700(f.value);
            f.state = 0;
            f.site = 0x00871ecf;
            destroy_native_lua_object_00b67700(f.key);
        } else {
            f.site = 0x00871edd;
            const char* const text = native_lua_string_protected(f.particle);
            f.site = 0x00871ee7;
            construct_native_string_header_0041e870(f.name, strings, text);
            f.state = 5;
            void* const resource = f.acquire(context, false);
            void* const data = pointer(f.name, 4);
            f.state = 0;
            if (data) f.return_name(data, word(f.name), strings, false);
            f.append(static_cast<unsigned char*>(component) + 0x28, resource, underwater, false);
        }
        f.state = -1;
        f.site = 0x00871f78;
        destroy_native_lua_object_00b67700(f.particle);
        f.phase = NativeParticleComponentReaderRawPhase::complete;
    } catch (...) {
        f.failure = f.site;
        f.failed_state = f.state;
        f.phase = NativeParticleComponentReaderRawPhase::failed;
        f.cleanup(strings);
        throw;
    }
}
} // namespace bsp
