#include "bsp/effect_cache.hpp"
#include "bsp/resource_path.hpp"
#include <algorithm>
#include <utility>

namespace bsp {
namespace {
bool supported_key(const std::string& name) {
    return name.size() <= INT32_MAX && std::all_of(name.begin(), name.end(),
        [](unsigned char c) { return c != 0 && c < 128; });
}
bool same_key(const std::string& left, const std::string& right) {
    if (left.size() != right.size()) return false;
    return std::equal(left.begin(), left.end(), right.begin(), [](char a, char b) {
        if (a >= 'A' && a <= 'Z') a = static_cast<char>(a + 32);
        if (b >= 'A' && b <= 'Z') b = static_cast<char>(b + 32);
        return a == b;
    });
}
}

bool resolve_effect_cache_key_00b2e940_fragment(const std::string& request,
    const EffectExistingNameResolver& resolve, std::string& output, std::string& error) {
    if (!resolve || !supported_key(request)) {
        error = "Effect cache requires an existing-name resolver and an ASCII resource key.";
        return false;
    }
    std::string changed;
    if (!shader_descriptor_name_00b2ebb0_fragment(request, changed)) return false;
    if (!resolve(changed)) {
        changed = request;
        resolve(changed); // Native deliberately ignores original-name failure.
    }
    if (!supported_key(changed)) {
        error = "Effect resolver returned an unsupported resource key.";
        return false;
    }
    output = std::move(changed);
    error.clear();
    return true;
}

EffectCache::EffectCache(EffectCacheCallbacks callbacks) : callbacks_(std::move(callbacks)) {}
EffectCache::~EffectCache() { clear_00b31750_fragment(); }

EffectOwner EffectCache::acquire_00b318b0_fragment(const std::string& requested, std::string& error) {
    if (!callbacks_.resolve_existing_name || !callbacks_.load_effect || !callbacks_.effect_size
        || !supported_key(requested)) {
        error = "Effect cache requires complete ownership callbacks and an ASCII resource key.";
        return {};
    }
    auto request = requested;
    normalize_resource_path_00bee690(request);
    for (const auto& cached : entries_) {
        if (std::any_of(cached.aliases.begin(), cached.aliases.end(),
            [&](const std::string& alias) { return same_key(alias, request); })) {
            error.clear();
            return cached.effect;
        }
    }

    std::string canonical;
    if (!resolve_effect_cache_key_00b2e940_fragment(request, callbacks_.resolve_existing_name,
        canonical, error)) return {};
    normalize_resource_path_00bee690(canonical);
    if (!same_key(canonical, request)) {
        for (auto& cached : entries_) {
            if (same_key(cached.aliases.front(), canonical)) {
                cached.aliases.push_back(request);
                error.clear();
                return cached.effect;
            }
        }
    }

    // load_effect may recursively append an error.shfx entry. Keep no references
    // to vector storage across the callback, and do not deduplicate by effect.
    auto owner = callbacks_.load_effect(canonical, error);
    if (!owner) {
        if (error.empty()) error = "Effect loader returned no owned effect.";
        return {};
    }
    EffectCacheEntry added{canonical, {canonical}, std::move(owner)};
    if (!same_key(request, canonical)) added.aliases.push_back(request);
    entries_.push_back(std::move(added));
    accounted_size_ += callbacks_.effect_size(entries_.back().effect.get());
    error.clear();
    return entries_.back().effect;
}

void EffectCache::clear_00b31750_fragment() noexcept {
    while (!entries_.empty()) {
        auto& cached = entries_.back();
        accounted_size_ -= callbacks_.effect_size(cached.effect.get());
        cached.effect.reset();
        entries_.pop_back();
    }
}

const EffectCacheEntry* EffectCache::entry(std::size_t index) const noexcept {
    return index < entries_.size() ? &entries_[index] : nullptr;
}
}
