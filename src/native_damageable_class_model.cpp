#include "bsp/native_damageable_class_model.hpp"
#include "bsp/native_game_resource_factory.hpp"
#include "bsp/native_resource_manager_lifetime.hpp"
#include "bsp/native_resource_load_cache.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*) == 4, "Actual class model headers require Win32");
namespace {
using Word = std::uint32_t;
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
template<class T> T read(const void* p, Word offset = 0) noexcept {
    T value;std::memcpy(&value, reinterpret_cast<const void*>(reinterpret_cast<Word>(p) + offset), sizeof value);return value;
}
template<class T> void write(void* p, Word offset, T value) noexcept {
    std::memcpy(at(p, offset), &value, sizeof value);
}
} // namespace
struct NativeDamageableClassModelAcquired::Impl {
    explicit Impl(NativeDamageableClassModelContext& c)
        : context(c), raw(c.resources.manager.strings), strings(raw.actual_published_01090aa8,
            raw.actual_small_returns_disabled_01090aa4, raw.actual_manager_publication_01090aa0) {}
    NativeDamageableClassModelContext& context;
    NativeStringRawPoolContext& raw;
    ActualNativeStringPoolStorage strings;
    NativeVfsNameResolutionAcquired resolution;
    NativeResourceLoadCacheAcquired loading;
    NativeString suffix, extension, prefix, joined, candidate, load_name;
    NativeDamageableClassModelPhase phase{NativeDamageableClassModelPhase::fresh};
    int state{-1};
    void unwind() noexcept {
        static constexpr int previous[]{-1,0,1,2,3,2,1,0,-1,-1};
        NativeString* const headers[]{&suffix,&extension,&prefix,&joined,&candidate,
            &candidate,&candidate,&candidate,&candidate,&load_name};
        try {
            while (state >= 0) {
                auto* const header = headers[state];state = previous[state];
                destroy_native_string_header_0041dd20(header, raw);
            }
        } catch (...) { std::terminate(); }
    }
};
NativeDamageableClassModelAcquired::NativeDamageableClassModelAcquired(NativeDamageableClassModelContext& c)
    : impl_(std::make_unique<Impl>(c)) {}
NativeDamageableClassModelAcquired::~NativeDamageableClassModelAcquired() = default;
NativeDamageableClassModelPhase NativeDamageableClassModelAcquired::phase() const noexcept { return impl_->phase; }

void* load_native_game_resource_007188a0(const void* name, NativeGameResourceLoadContext& c,
    NativeResourceLoadCacheAcquired& acquired) {
    void* const factory = get_native_game_resource_factory_007175d0(c.factory);
    void* const manager = get_native_resource_manager_004c1400(c.manager);
    return load_and_cache_native_resource_00b80720(manager, name, factory, c.cache, acquired);
}
void load_native_damageable_class_model_00879590(void* actual, Word enemy,
    NativeDamageableClassModelContext& context, NativeDamageableClassModelAcquired& acquired) {
    auto& f = *acquired.impl_;
    if (&f.context != &context || f.phase != NativeDamageableClassModelPhase::fresh)
        throw std::logic_error("class model loading requires a fresh invocation in the same context");
    f.phase = NativeDamageableClassModelPhase::running;
    try {
        void* const name = at(actual, 0x38);
        if (read<Word>(name) != 0 && read<void*>(actual, 0x50) == nullptr) {
            if ((enemy & 0xffu) != 0) {
                const auto* const data = read<const char*>(name, 4);
                if (data) {
                    const auto* const dot = std::strstr(data, ".");
                    if (dot) {
                        // Reload header data after strstr, as8795F0 does.
                        const Word position = reinterpret_cast<Word>(dot) - read<Word>(name, 4);
                        if (static_cast<std::int32_t>(position) >= 0) {
                            construct_native_string_cstring_0041e870(&f.suffix, "_enemy", f.raw);f.state = 0;
                            construct_native_string_substring_00469840(name, &f.extension, position, 0x7fffffffu, f.strings);f.state = 1;
                            construct_native_string_substring_00469840(name, &f.prefix, 0, position, f.strings);f.state = 2;
                            concatenate_native_string_headers_004261a0(&f.prefix, &f.joined, &f.suffix, f.strings);f.state = 3;
                            concatenate_native_string_headers_004261a0(&f.joined, &f.candidate, &f.extension, f.strings);
                            f.state = 5;destroy_native_string_header_0041dd20(&f.joined, f.raw);
                            f.state = 6;destroy_native_string_header_0041dd20(&f.prefix, f.raw);
                            f.state = 7;destroy_native_string_header_0041dd20(&f.extension, f.raw);
                            f.state = 8;destroy_native_string_header_0041dd20(&f.suffix, f.raw);
                            auto& cache = context.resources.cache;
                            if (resolve_native_vfs_existing_name_00bdf4c0(cache.actual_vfs_0109ceec,
                                &f.candidate, cache.names, f.resolution))
                                assign_native_string_header_00425f40(name, &f.candidate, f.strings);
                            f.state = -1;destroy_native_string_header_0041dd20(&f.candidate, f.raw);
                        }
                    }
                }
            }
            const auto* const current_data = read<const char*>(name, 4);
            construct_native_string_cstring_0041e870(&f.load_name, current_data ? current_data : "", f.raw);
            f.state = 9;
            void* const resource = load_native_game_resource_007188a0(&f.load_name, context.resources, f.loading);
            write(actual, 0x50, resource);
            f.state = -1;destroy_native_string_header_0041dd20(&f.load_name, f.raw);
        }
        f.phase = NativeDamageableClassModelPhase::complete;
    } catch (...) {
        f.unwind();f.phase = NativeDamageableClassModelPhase::failed;throw;
    }
}
void activate_native_damageable_class_model_00879aa0(void* actual, Word enemy,
    NativeDamageableClassModelContext& context, NativeDamageableClassModelAcquired& acquired) {
    load_native_damageable_class_model_00879590(actual, enemy, context, acquired);
    if (read<void*>(actual, 0x50)) {
        const Word target = context.classes.class_slot20(read<Word>(actual));
        context.classes.bind_model(target, actual);
    }
    write<std::uint8_t>(actual, 0x44, 1);
}
void ensure_native_damageable_class_model_0043ebd0(void* actual, Word enemy,
    NativeDamageableClassModelContext& context, NativeDamageableClassModelAcquired& acquired) {
    if (read<std::uint8_t>(actual, 0x44) == 0)
        activate_native_damageable_class_model_00879aa0(actual, enemy, context, acquired);
}
} // namespace bsp
