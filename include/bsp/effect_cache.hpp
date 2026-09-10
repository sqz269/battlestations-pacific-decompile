#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bsp {
using EffectOwner = std::shared_ptr<const void>;
// The exact mutable-name/existence contract of00bdf4c0, including mutation on
// failure. Supply the actual VFS lookup, not a file-open or suffix guess.
using EffectExistingNameResolver = std::function<bool(std::string&)>;

// Registry virtual+4,00b2e940: rewrite EVERY .mshd substring to .shfx and resolve
// that copy first. If absent, resolve the original copy and return it even when
// absent. A true return means supported input, not that the resource exists.
// Host keys are NUL-free ASCII with length<=INT32_MAX. Output survives guards.
bool resolve_effect_cache_key_00b2e940_fragment(const std::string& request,
    const EffectExistingNameResolver&, std::string& output, std::string& error);

struct EffectCacheCallbacks {
    EffectExistingNameResolver resolve_existing_name;
    // Return an OWNED actual effect. A new entry adopts it; the caller obtains
    // another owner. Loading/compilation and00b2ebb0 error.shfx recursion belong
    // here. No pending entry or recursion sentinel is installed before this call.
    std::function<EffectOwner(const std::string& canonical_name, std::string& error)> load_effect;
    // Native effect virtual+Ch is queried on insertion and again at removal.
    // Must not throw or reenter the cache. Returns DWORD accounting units.
    std::function<std::uint32_t(const void*)> effect_size;
};

struct EffectCacheEntry {
    // Native entry+0 string, with canonical key also first in aliases (+8 list).
    std::string canonical_name;
    std::vector<std::string> aliases;
    EffectOwner effect; // Native entry+28h; exactly one retained owner per entry.
};

// Owning projection of the renderer00b318b0's fixed0/1/1 call to00b31090.
// New interface, not native ABI. Serialized calls; resolve/load may acquire
// recursively. Platform message pump/renderer guard/VFS metadata remain external.
class EffectCache {
public:
    explicit EffectCache(EffectCacheCallbacks);
    ~EffectCache();
    EffectCache(const EffectCache&) = delete;
    EffectCache& operator=(const EffectCache&) = delete;
    EffectCache(EffectCache&&) = delete;
    EffectCache& operator=(EffectCache&&) = delete;

    // Normalizes via00bee690 only; does NOT collapse slash/dot components.
    // Searches all aliases, then canonical first-alias identity after resolution.
    // Null load is an explicit host failure (native inserts unsafe null entries).
    // Existing entries are unchanged by null failure; recursive loads may have
    // inserted entries. Allocation/callback exceptions propagate.
    EffectOwner acquire_00b318b0_fragment(const std::string&, std::string& error);
    // Reverse release, recomputing each effect's size before its owner release.
    // External caller owners survive this operation. Size callback must not throw.
    void clear_00b31750_fragment() noexcept;
    std::size_t size() const noexcept { return entries_.size(); }
    std::uint32_t accounted_size() const noexcept { return accounted_size_; }
    const EffectCacheEntry* entry(std::size_t index) const noexcept;

private:
    EffectCacheCallbacks callbacks_;
    std::vector<EffectCacheEntry> entries_;
    std::uint32_t accounted_size_{};
};
}
