#include "bsp/native_particle_resource_loader_raw.hpp"

#include "bsp/native_particle_resource_loading.hpp"
#include "bsp/native_particle_resource_parser_raw.hpp"
#include "bsp/native_particle_text_loader.hpp"
#include "bsp/native_pooled_text.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <exception>
#include <optional>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle resource loading requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& word(const void* base, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(
        static_cast<unsigned char*>(const_cast<void*>(base)) + offset);
}
void* pointer(const void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
} // namespace

struct NativeParticleResourceLoaderRawAcquired::Impl {
    // Native ESP+8 name and ESP+10 text. Keep all raw bytes before children.
    alignas(4) unsigned char copied_name[8];
    alignas(4) unsigned char text[0x1c];
    std::optional<NativeVfsNameResolutionAcquired> resolution;
    std::optional<NativeParticleResourceParserRawAcquired> parser;
    NativeParticleResourceLoaderRawPhase phase = NativeParticleResourceLoaderRawPhase::fresh;
    Word site = 0, failure = 0;
    std::int32_t state = -1, failed_state = -1;
    void* allocation = nullptr;
    bool constructed = false;
    std::int32_t parser_builder_kind;
    explicit Impl(std::int32_t kind) : parser_builder_kind(kind) {}
    // Preserve native uninitialized text+8 until AF5850 writes it.

    void cleanup(NativeStringRawPoolContext& strings) noexcept {
        try {
            // DC7560: 2 ->1 free allocation; 1 ->0 copied name; 0 ->-1 text.
            if (state == 2) { state = 1; singleton_lifetime_free(allocation); }
            if (state == 1) { state = 0; destroy_native_string_header_0041dd20(copied_name, strings); }
            if (state == 0) { state = -1; destroy_native_particle_text_buffer_00af5620(text, strings); }
        } catch (...) { std::terminate(); }
    }
};

NativeParticleResourceLoaderRawAcquired::NativeParticleResourceLoaderRawAcquired(std::int32_t kind)
    : impl_(std::make_unique<Impl>(kind)) {}
NativeParticleResourceLoaderRawAcquired::~NativeParticleResourceLoaderRawAcquired() = default;
NativeParticleResourceLoaderRawPhase NativeParticleResourceLoaderRawAcquired::phase() const noexcept { return impl_->phase; }
Word NativeParticleResourceLoaderRawAcquired::failure_site() const noexcept { return impl_->failure; }
std::int32_t NativeParticleResourceLoaderRawAcquired::native_state_at_failure() const noexcept { return impl_->failed_state; }
void* NativeParticleResourceLoaderRawAcquired::constructed_resource() const noexcept {
    return impl_->constructed ? impl_->allocation : nullptr;
}
const NativeVfsNameResolutionAcquired* NativeParticleResourceLoaderRawAcquired::resolution_invocation() const noexcept {
    return impl_->resolution ? &*impl_->resolution : nullptr;
}
const NativeParticleResourceParserRawAcquired* NativeParticleResourceLoaderRawAcquired::parser_invocation() const noexcept {
    return impl_->parser ? &*impl_->parser : nullptr;
}

void* create_native_particle_resource_0086ba60(void* ignored_cache, const void* name,
    Word ignored_second, NativeParticleResourceLoaderRawContext& context,
    NativeParticleResourceLoaderRawAcquired& acquired) {
    (void)ignored_cache;
    (void)ignored_second;
    auto& f = *acquired.impl_;
    if (f.phase != NativeParticleResourceLoaderRawPhase::fresh)
        throw std::logic_error("Particle resource loader invocation cannot replay");
    f.phase = NativeParticleResourceLoaderRawPhase::running;
    auto& strings = context.strings;
    try {
        f.site = 0x0086ba7e;
        initialize_native_text_buffer_00af5600(f.text);
        f.state = 0;
        word(f.copied_name) = 0;
        word(f.copied_name, 4) = 0;
        if (f.copied_name != name) {
            f.site = 0x0086baa6;
            resize_native_string_header_0041dd40(f.copied_name, strings, word(name), true);
            if (word(name) != 0) {
                const Word count = word(f.copied_name);
                const void* const input = pointer(name, 4);
                void* const output = pointer(f.copied_name, 4);
                f.site = 0x0086babd;
                std::memmove(output, input, count);
            }
        }
        f.state = 1;
        f.site = 0x0086bace;
        lowercase_native_string_header_004bcc00(f.copied_name);
        f.site = 0x0086bad8;
        f.allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x90, 0x90});
        f.state = 2;
        if (f.allocation) {
            f.site = 0x0086baf6;
            construct_native_particle_resource_00af45d0(f.allocation, f.copied_name, strings);
            word(f.allocation) = 0x00d0d418u;
            f.constructed = true;
        }
        const char* const name_bytes = static_cast<const char*>(pointer(f.copied_name, 4));
        f.state = 1;
        f.resolution.emplace();
        f.site = 0x0086bb1c;
        (void)load_native_particle_text_buffer_00af5850(f.text,
            name_bytes ? name_bytes : context.empty_name_00f8766c,
            context.actual_vfs_publication_0109ceec, context.vfs,
            context.name_resolution, *f.resolution, strings);
        f.parser.emplace(f.parser_builder_kind);
        f.site = 0x0086bb28;
        (void)parse_native_particle_resource_00af4ba0(f.allocation, f.text, context.parser, *f.parser);

        void* const data = pointer(f.copied_name, 4);
        f.state = 0;
        if (data) {
            const Word size = word(f.copied_name) + 1u;
            f.site = 0x0086bb44;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
            f.site = 0x0086bb4b;
            return_native_string_pool_00bd1510(pool, data, size, strings.actual_small_returns_disabled_01090aa4);
        }
        f.state = -1;
        f.site = 0x0086bb5c;
        destroy_native_particle_text_buffer_00af5620(f.text, strings);
        f.phase = NativeParticleResourceLoaderRawPhase::complete;
        return f.allocation;
    } catch (...) {
        f.failure = f.site;
        f.failed_state = f.state;
        f.phase = NativeParticleResourceLoaderRawPhase::failed;
        f.cleanup(strings);
        throw;
    }
}
} // namespace bsp
