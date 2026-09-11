#include "bsp/sound_resource_cache.hpp"
#include "bsp/native_pooled_resource_path.hpp"

#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {

class ScopedPath {
public:
    explicit ScopedPath(NativeStringStorage& storage) noexcept : storage_(storage) {}
    ScopedPath(NativeString&& string, NativeStringStorage& storage) noexcept
        : value(std::move(string)), storage_(storage) {}
    ~ScopedPath() { value.release_to(storage_); }
    NativeString value;
private:
    NativeStringStorage& storage_;
};

std::string copy_path(const NativeString& path) {
    return path.length() ? std::string(path.data(), path.length()) : std::string();
}

bool same_name(const NativeString& left, const NativeString& right) {
    if (!left.length()) return !right.length();
    if (!right.length()) return false;
    return ::_stricmp(left.data(), right.data()) == 0;
}

bool alias_matches(const std::string& alias, const NativeString& path) {
    // The counted-length guard is present in both alias searches, but absent
    // from the direct comparison between requested and resolved names.
    return alias.size() == path.length()
        && (alias.empty() || ::_stricmp(alias.c_str(), path.data()) == 0);
}

} // namespace

void append_sound_resource_cache_record_00a84530(SoundResourceOwner& owner,
    const SoundResourceCacheRecord& record) {
    if (owner.records_04.size() == static_cast<std::uint32_t>(owner.capacity_0c)) {
        // ADD EAX,EAX followed by signed JG 40h. Express wrap without signed UB.
        const auto doubled = static_cast<std::uint32_t>(owner.capacity_0c) * 2u;
        const auto requested = doubled <= static_cast<std::uint32_t>(INT32_MAX)
            && doubled > 64u ? doubled : 64u;
        if (requested > static_cast<std::uint32_t>(owner.capacity_0c)) {
            owner.records_04.reserve(requested);
            owner.capacity_0c = static_cast<std::int32_t>(requested);
        }
    }
    // Overflowed/full native storage would be an out-of-bounds placement-new.
    // This host container cannot model that memory corruption as valid behavior.
    if (owner.capacity_0c < 0
        || owner.records_04.size() >= static_cast<std::uint32_t>(owner.capacity_0c)) {
        throw std::length_error("Sound cache has no representable native record capacity");
    }
    owner.records_04.push_back(record);
}

void clear_sound_resource_cache_record_storage_00a842e0(
    SoundResourceCacheRecord& record) noexcept {
    record.aliases_0c.clear();
    std::string().swap(record.name_00);
}

SoundResourceCacheRecord& copy_sound_resource_cache_record_00a84680(
    SoundResourceCacheRecord& destination, const SoundResourceCacheRecord& source) {
    if (&destination != &source) {
        destination.name_00 = source.name_00;
        destination.aliases_0c.clear();
        destination.aliases_0c.insert(destination.aliases_0c.end(),
            source.aliases_0c.begin(), source.aliases_0c.end());
    }
    destination.metadata_14 = source.metadata_14;
    destination.resource_28 = source.resource_28;
    return destination;
}

void remove_sound_resource_cache_entry_00a85560(SoundResourceOwner& owner,
    const NativeString& path, SoundResourceCacheHost& host, NativeStringStorage& storage) {
    ScopedPath requested(storage);
    requested.value.copy_from_00be0a30_fragment(storage, path);
    {
        // The original copy constructor owns failure cleanup until it returns.
        // Do not arm a second destructor before that point.
        NativeString normalized;
        copy_construct_native_resource_path_header_00bee780(&normalized,
            &requested.value, storage);
        ScopedPath discarded(std::move(normalized), storage);
    }
    SoundResourceCacheRecord* matched = nullptr;
    for (auto& record : owner.records_04) {
        for (const auto& alias : record.aliases_0c) {
            if (alias_matches(alias, requested.value)) {
                matched = &record;
                break;
            }
        }
        if (matched) break;
    }
    // Both native diagnostic calls target the proven RET body 004254B0.
    if (!matched) return;
    if (!matched->resource_28) {
        throw std::logic_error("Sound cache removal requires a nonnull matched resource");
    }
    const auto size = host.resource_size_vslot_0c(*matched->resource_28);
    owner.total_resource_size_10 -= size;
    if (matched != &owner.records_04.back()) {
        copy_sound_resource_cache_record_00a84680(*matched, owner.records_04.back());
    }
    clear_sound_resource_cache_record_storage_00a842e0(owner.records_04.back());
    owner.records_04.pop_back();
}

SoundOwnedResource* load_sound_resource_00a84740(SoundResourceOwner& owner,
    const NativeString& path, SoundResourceLoadOptions& options, bool clone,
    bool load_if_missing, SoundResourceCacheHost& host, NativeStringStorage& storage) {
    host.pump_load_events_00beccd0();
    ScopedPath requested(storage);
    requested.value.copy_from_00be0a30_fragment(storage, path);
    normalize_native_resource_path_header_00bee690(&requested.value, storage);

    SoundOwnedResource* existing = nullptr;
    for (const auto& record : owner.records_04) {
        for (const auto& alias : record.aliases_0c) {
            if (alias_matches(alias, requested.value)) {
                existing = record.resource_28;
                break;
            }
        }
        if (existing) break;
    }

    ScopedPath canonical(storage);
    if (existing) return host.retain_00a854c0(owner, *existing);

    {
        ScopedPath resolved(host.resolve_00a82ea0(owner, requested.value, options,
            storage), storage);
        canonical.value.copy_from_00be0a30_fragment(storage, resolved.value);
    }
    normalize_native_resource_path_header_00bee690(&canonical.value, storage);

    if (!same_name(canonical.value, requested.value)) {
        for (auto& record : owner.records_04) {
            // Native checked-iterator failure on an empty list is an invalid
            // record precondition. Never manufacture an alias to recover it.
            if (record.aliases_0c.empty()) {
                throw std::logic_error("Sound cache record has no canonical alias");
            }
            if (alias_matches(record.aliases_0c.front(), canonical.value)) {
                record.aliases_0c.push_back(copy_path(requested.value));
                existing = record.resource_28;
                break;
            }
        }
        if (existing) return host.retain_00a854c0(owner, *existing);
    }

    if (!load_if_missing) return nullptr;

    auto* created = host.create_00a835b0(owner, canonical.value, options);
    SoundResourceCacheRecord record;
    record.name_00 = copy_path(canonical.value);
    record.aliases_0c.push_back(copy_path(canonical.value));
    record.metadata_14 = host.query_metadata_00bdd340(canonical.value);
    if (!same_name(requested.value, canonical.value)) {
        record.aliases_0c.push_back(copy_path(requested.value));
    }
    record.resource_28 = created;
    append_sound_resource_cache_record_00a84530(owner, record);
    if (created) {
        const auto size = host.resource_size_vslot_0c(*created);
        // Reload accounting after the callback; DWORD addition wraps.
        owner.total_resource_size_10 += size;
    }
    auto* result = clone && created ? host.retain_00a854c0(owner, *created) : created;
    clear_sound_resource_cache_record_storage_00a842e0(record);
    return result;
}

} // namespace bsp
