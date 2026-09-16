#include "bsp/native_particle_resource_acquisition_raw.hpp"
#include "bsp/native_particle_resource_loader_raw.hpp"
#include "bsp/native_particle_resource_cache.hpp"
#include "bsp/native_particle_resource_records.hpp"
#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_pooled_resource_path.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/resource_load_events.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <optional>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle cache acquisition requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
using Context = NativeParticleResourceAcquisitionRawContext;
using Acquired = NativeParticleResourceAcquisitionRawAcquired;
using Phase = NativeParticleResourceAcquisitionRawPhase;
static_assert(sizeof(void*) == 4);
void* ptr(U value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, U offset) noexcept { return ptr(reinterpret_cast<U>(p) + offset); }
volatile U& word(const void* p, U offset = 0) noexcept {
    return *static_cast<volatile U*>(at(p, offset));
}
void* pointer(const void* p, U offset = 0) noexcept { return ptr(word(p, offset)); }
// Output has already been initialized/armed according to its native caller.
void copy_header(void* output, const void* source, Context& c, U& site, U resize_site, U copy_site) {
    if (output == source) return;
    site = resize_site;
    resize_native_string_header_0041dd40(output, c.strings, word(source), true);
    if (word(source) != 0) {
        const U count = word(output);
        const void* const input = pointer(source, 4);
        void* const data = pointer(output, 4);
        site = copy_site;
        std::memmove(data, input, count);
    }
}
bool names_equal(const void* left, const void* right, bool length_gate) {
    const U a = word(left), b = word(right);
    if (length_gate && a != b) return false;
    if (a == 0) return b == 0;
    if (b == 0) return false;
    const char* const r = static_cast<const char*>(pointer(right, 4));
    const char* const l = static_cast<const char*>(pointer(left, 4));
    return _stricmp(l, r) == 0;
}
void append_alias(void* owner, const void* name, Context& c, U& site, U allocate_site, U grow_site) {
    void* const sentinel = pointer(owner, 4);
    auto* const previous = static_cast<NativeRenderResourceAliasNode*>(pointer(sentinel, 4));
    site = allocate_site;
    auto* const node = allocate_native_render_alias_node_004ce6f0(
        static_cast<NativeRenderResourceAliasNode*>(sentinel), previous, name, c.strings);
    site = grow_site;
    grow_native_alias_list_count_004ce780(owner, 1);
    word(sentinel, 4) = reinterpret_cast<U>(node);
    void* const current_previous = pointer(node, 4);
    word(current_previous) = reinterpret_cast<U>(node);
}
} // namespace

struct Acquired::Impl {
    explicit Impl(std::int32_t kind) : parser_kind(kind) {}
    // Native ESP+10..6B. Keep raw locals alive BEFORE the optional loader,
    // including name/date/record bytes retained by interrupted providers.
    U locals[23];
    std::optional<NativeParticleResourceLoaderRawAcquired> loader;
    std::int32_t parser_kind;
    Phase phase = Phase::fresh;
    U site = 0, failure = 0, profile = 0, target = 0;
    int state = -1, failed_state = -1;
    void* platform = nullptr;
    void* loaded = nullptr;
    U* slot(unsigned i) noexcept { return locals + i; }
    U dispatch(void* cache, U offset, U call_site, Context& c) {
        site = call_site;
        profile = word(cache);
        const void* view = nullptr;
        if (profile == 0x00d0db40) view = c.actual_cache_profile_00d0db40;
        else if (profile == 0x00d0daf0) view = c.actual_base_profile_00d0daf0;
        target = 0;
        if (!view) throw std::invalid_argument("Particle cache acquisition requires the current native profile binding");
        target = word(view, offset);
        return target;
    }
    void invalid(Context& c, U call_site) {
        site = call_site;
        if (!c.crt.invalid_parameter)
            throw std::invalid_argument("Particle cache iterator requires the actual CRT invalid-parameter policy");
        c.crt.invalid_parameter(c.crt.context);
    }
    void return_header(U* header, int previous, Context& c, U getter_site, U return_site) {
        void* const captured = pointer(header, 4);
        state = previous;
        if (captured) {
            const U size = word(header) + 1u;
            site = getter_site;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                c.strings.actual_published_01090aa8, c.strings.actual_manager_publication_01090aa0);
            site = return_site;
            return_native_string_pool_00bd1510(pool, captured, size,
                c.strings.actual_small_returns_disabled_01090aa4);
        }
    }
    void unwind(Context& c) noexcept {
        static constexpr int previous[]{-1, 0, 1, 1, 1};
        static constexpr unsigned slots[]{5, 3, 1, 12, 12};
        try {
            while (state >= 0) {
                const int old = state;
                state = previous[old];
                if (old == 4) destroy_native_particle_resource_record_0086fcf0(slot(12), c.strings);
                else destroy_native_string_header_0041dd20(slot(slots[old]), c.strings);
            }
        } catch (...) { std::terminate(); }
    }
};
Acquired::NativeParticleResourceAcquisitionRawAcquired(std::int32_t kind)
    : impl_(std::make_unique<Impl>(kind)) {}
Acquired::~NativeParticleResourceAcquisitionRawAcquired() = default;
Phase Acquired::phase() const noexcept { return impl_->phase; }
U Acquired::failure_site() const noexcept { return impl_->failure; }
std::int32_t Acquired::native_state_at_failure() const noexcept { return impl_->failed_state; }
U Acquired::captured_profile() const noexcept { return impl_->profile; }
U Acquired::captured_target() const noexcept { return impl_->target; }
void* Acquired::loaded_resource() const noexcept { return impl_->loaded; }
void* Acquired::captured_platform() const noexcept { return impl_->platform; }
const NativeParticleResourceLoaderRawAcquired* Acquired::loader_invocation() const noexcept {
    return impl_->loader ? &*impl_->loader : nullptr;
}

void* acquire_native_particle_resource_00870dd0(void* cache, const void* name,
    U forwarded, U retain_new, U allow_load, Context& c, Acquired& acquired) {
    auto& f = *acquired.impl_;
    if (f.phase != Phase::fresh) throw std::logic_error("Particle cache acquisition cannot replay");
    f.phase = Phase::running;
    auto* const input = f.slot(5);
    auto* const key = f.slot(3);
    auto* const temporary = f.slot(1);
    auto* const record = f.slot(12);
    auto retain = [&](void* owner, void* resource, U site) {
        if (f.dispatch(owner, 0xc, site, c) != 0x00871400)
            throw std::invalid_argument("Particle cache current retain slot is not a proved source target");
        return retain_native_particle_resource_00871400(resource);
    };
    auto finish = [&](void* result) {
        f.return_header(key, 0, c, 0x008712c0, 0x008712c7);
        f.return_header(input, -1, c, 0x008712e7, 0x008712ee);
        f.phase = Phase::complete;
        return result;
    };
    try {
        *f.slot(0) = reinterpret_cast<U>(cache);
        f.platform = c.actual_platform_publication_0109cf04;
        f.site = 0x00870df8;
        if (f.platform != c.bound_platform_identity)
            throw std::invalid_argument("Particle load events require the captured platform owner's source binding");
        pump_resource_load_events_00beccd0(c.load_events);
        *temporary = 0;
        input[0] = 0; input[1] = 0;
        copy_header(input, name, c, f.site, 0x00870e20, 0x00870e37);
        f.state = 0; f.site = 0x00870e47;
        normalize_native_resource_path_header_00bee690(input, c.strings);
        const U initial_count = word(cache, 8);
        U row = word(cache, 4);
        const U end = row + initial_count * 0x2cu;
        for (; row != end; row += 0x2cu) {
            void* const sentinel_field = ptr(row + 0xcu);
            void* const sentinel = pointer(sentinel_field);
            void* node = pointer(sentinel);
            while (node != sentinel) {
                if (node == pointer(sentinel_field)) f.invalid(c, 0x00870e85);
                f.site = 0x00870eb3;
                if (names_equal(at(node, 8), input, true)) {
                    *temporary = word(ptr(row), 0x28);
                    break;
                }
                if (node == pointer(sentinel_field)) f.invalid(c, 0x00870ec9);
                node = pointer(node);
            }
            if (*temporary != 0) break;
        }
        key[0] = 0; key[1] = 0; f.state = 1;
        if (*temporary) return finish(retain(cache, ptr(*temporary), 0x00871176));

        if (f.dispatch(cache, 4, 0x00870f29, c) != 0x0086ba10)
            throw std::invalid_argument("Particle cache current key slot is not a proved source target");
        void* const returned_key = construct_native_particle_resource_cache_key_0086ba10(
            cache, temporary, input, ptr(forwarded), c.strings);
        f.state = 2;
        copy_header(key, returned_key, c, f.site, 0x00870f43, 0x00870f5a);
        f.return_header(temporary, 1, c, 0x00870f7a, 0x00870f81);
        f.site = 0x00870f8a;
        normalize_native_resource_path_header_00bee690(key, c.strings);
        f.site = 0x00870faf;
        if (!names_equal(key, input, false)) {
            const U count = word(cache, 8);
            row = word(cache, 4);
            const U resolved_end = row + count * 0x2cu;
            for (; row != resolved_end; row += 0x2cu) {
                void* const sentinel = pointer(ptr(row), 0xc);
                void* const first = pointer(sentinel);
                if (first == sentinel) f.invalid(c, 0x00870fd8);
                f.site = 0x00871006;
                if (names_equal(at(first, 8), key, true)) {
                    append_alias(ptr(row + 8), input, c, f.site, 0x0087113d, 0x00871148);
                    *temporary = word(ptr(row), 0x28);
                    if (*temporary) return finish(retain(cache, ptr(*temporary), 0x00871176));
                    break;
                }
            }
        }
        if ((allow_load & 0xffu) == 0) return finish(nullptr);

        if (f.dispatch(cache, 8, 0x0087104d, c) != 0x0086ba60)
            throw std::invalid_argument("Particle cache current loader slot is not a proved source target");
        f.loader.emplace(f.parser_kind);
        f.loaded = create_native_particle_resource_0086ba60(cache, key, forwarded, c.loader, *f.loader);
        record[0] = 0; record[1] = 0; f.state = 3;
        f.site = 0x00871062;
        record[3] = reinterpret_cast<U>(allocate_native_render_alias_sentinel_004c3020());
        record[4] = 0;
        for (unsigned i = 9; i >= 5; --i) record[i] = 0;
        f.state = 4;
        copy_header(record, key, c, f.site, 0x00871093, 0x008710ad);
        append_alias(at(record, 8), key, c, f.site, 0x008710ca, 0x008710d7);
        void* const manager = c.actual_vfs_publication_0109ceec;
        f.site = 0x008710f3;
        const void* const date = query_native_vfs_file_date_00bdd340(manager, f.slot(7), key, c.dates, c.strings);
        const bool input_nonempty = word(input) != 0;
        for (U offset = 0; offset != 20; offset += 4) word(record, 0x14 + offset) = word(date, offset);
        bool same;
        if (!input_nonempty) same = word(key) == 0;
        else if (word(key) == 0) same = false;
        else {
            const char* const key_data = static_cast<const char*>(pointer(key, 4));
            const char* const input_data = static_cast<const char*>(pointer(input, 4));
            f.site = 0x00871190;
            same = _stricmp(input_data, key_data) == 0;
        }
        if (!same) append_alias(at(record, 8), input, c, f.site, 0x008711b1, 0x008711be);
        void* const current_cache = ptr(*f.slot(0));
        word(record, 0x28) = reinterpret_cast<U>(f.loaded);
        f.site = 0x008711da;
        append_native_particle_resource_record_00870ac0(at(current_cache, 4), record, c.strings);
        void* result = f.loaded;
        const bool retained = (retain_new & 0xffu) != 0 && result != nullptr;
        if (retained) result = retain(current_cache, result, 0x008711f5);
        f.state = 1;
        f.site = retained ? 0x00871202 : 0x0087123f;
        destroy_native_particle_resource_record_0086fcf0(record, c.strings);
        f.return_header(key, 0, c, retained ? 0x0087121f : 0x0087125c,
            retained ? 0x00871226 : 0x00871263);
        f.return_header(input, -1, c, retained ? 0x008712e7 : 0x00871283,
            retained ? 0x008712ee : 0x0087128a);
        f.phase = Phase::complete;
        return result;
    } catch (...) {
        f.failure = f.site;
        f.failed_state = f.state;
        f.phase = Phase::failed;
        f.unwind(c);
        throw;
    }
}
} // namespace bsp
