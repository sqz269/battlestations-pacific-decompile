#include "bsp/sound_sample_cache.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
struct ScopedName {
    NativeStringStorage& strings;
    NativeString value;
    ~ScopedName() { destroy_native_string_header_0041dd20(&value, strings); }
};
// Miss temporaries have two different captured cleanup operands in assembly.
struct PairName {
    NativeStringStorage& strings;
    NativeString value;
    char* captured_data{};
    ~PairName() { if (captured_data) strings.release(captured_data, value.length() + 1u); }
};
struct InputCopy {
    NativeStringStorage& strings;
    NativeString value;
    std::uint32_t captured_length{};
    ~InputCopy() { if (value.data()) strings.release(value.data(), captured_length + 1u); }
};
bool matches(const NativeString& stored, const NativeString& requested, SoundSampleCacheContext& context) {
    if (stored.length() == requested.length()
        && !native_string_not_equal_case_insensitive_00449af0(stored, requested)) return true;
    if (stored.length() == requested.length()) return false;
    const NativeString& longer = stored.length() > requested.length() ? stored : requested;
    const NativeString& shorter = stored.length() > requested.length() ? requested : stored;
    const auto offset = longer.length() - shorter.length();
    const char* const longer_data = longer.data() ? longer.data() : context.null_longer_data_00f8bbec;
    const char separator = longer_data[offset - 1u];
    return (separator == '/' || separator == '\\')
        && native_string_matches_at_0043e9a0(longer, shorter, offset, context.null_pattern_00e17bf0);
}
void insert_weak(SoundAuxiliaryTreeOwner& owner, const NativeString& name,
    void* sample, NativeStringStorage& strings) {
    auto& tree = *owner.tree_04;
    const auto position = tree.lower_bound(name);
    if (position != tree.end() && !tree.key_comp()(name, position->first)) return;
    //00A83C90 checks the native count only after unique-insertion lookup.
    if (owner.count_0c > 0x15555553u) throw std::length_error("map/set<T> too long");
    ScopedName key{strings, {}};
    key.value.copy_from_00be0a30_fragment(strings, name);
    tree.emplace_hint(position, std::move(key.value), SoundAuxiliaryTreeValue{sample});
    ++owner.count_0c;
}
} // namespace
bool native_string_matches_at_0043e9a0(const NativeString& candidate,
    const NativeString& pattern, std::uint32_t start, const char* null_pattern_00e17bf0) noexcept {
    const char* wanted = pattern.data() ? pattern.data() : null_pattern_00e17bf0;
    if (!candidate.data() || !wanted || start > candidate.length()) return false;
    const char* value = candidate.data() + start;
    while (*value && *wanted && *value == *wanted) { ++value; ++wanted; }
    return *wanted == '\0';
}
void* create_sound_sample_00a85440(const NativeString& name, SoundSampleCacheHost& host) {
    void* const allocated = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x7c, 0x7c});
    if (!allocated) return nullptr;
    try { return host.construct_sample_00a84d70(allocated, name); }
    catch (...) { singleton_lifetime_free(allocated); throw; }
}
void** acquire_sound_sample_00a83fd0(SoundAuxiliaryTreeOwner& owner, void*& out,
    const NativeString& name, SoundSampleCacheContext& context) {
    ScopedName creator_name{context.strings, {}};
    creator_name.value.copy_from_00be0a30_fragment(context.strings, name);
    // The scan uses the current original argument; the first copy is only
    // the argument to the creator and stays alive through output publication.
    for (auto it = owner.tree_04->begin(); it != owner.tree_04->end(); ++it) {
        if (!matches(it->first, name, context)) continue;
        void* const captured = it->second.sample_14;
        InterlockedIncrement(reinterpret_cast<volatile LONG*>(static_cast<unsigned char*>(captured) + 4));
        out = it->second.sample_14;
        return &out;
    }
    void* const fresh = owner.native_vtable_00 == 0x00d5b460
        ? create_sound_sample_00a85440(creator_name.value, context.host)
        : context.host.unknown_cache_create_slot_04(owner, creator_name.value);
    {
        InputCopy input{context.strings, {}, 0};
        input.value.copy_from_00be0a30_fragment(context.strings, name);
        input.captured_length = input.value.length();
        PairName pair{context.strings, {}, nullptr};
        pair.value.resize_0041dd40(context.strings, input.captured_length, true);
        pair.captured_data = pair.value.data();
        if (input.captured_length != 0)
            std::memcpy(pair.captured_data, input.value.data(), pair.value.length());
        insert_weak(owner, pair.value, fresh, context.strings);
    }
    out = fresh; // Discard duplicate-insertion status; never retain the map value here.
    return &out;
}
void erase_sound_sample_iterator_00a83e80(SoundAuxiliaryTreeOwner& owner,
    SoundAuxiliaryTreeOwner::Tree::iterator position, NativeStringStorage& strings) {
    auto& tree = *owner.tree_04;
    if (position == tree.end()) throw std::out_of_range("invalid map/set<T> iterator");
    {
        auto node = tree.extract(position); // Topology changes before pooled-key release.
        destroy_native_string_header_0041dd20(&node.key(), strings);
    }
    if (owner.count_0c != 0) --owner.count_0c; // After key/node destruction, preserving a callback's zero.
}
void clear_sound_sample_cache_00a82c00(SoundAuxiliaryTreeOwner& owner,
    SoundSampleCacheContext& context) {
    while (!owner.tree_04->empty()) {
        const auto first = owner.tree_04->begin();
        if (owner.native_vtable_00 == 0x00d5b460)
            erase_sound_sample_iterator_00a83e80(owner, first, context.strings);
        else context.host.unknown_cache_erase_slot_08(owner, first);
    }
    context.reset_word_00f8bbe4 = 0;
}
} // namespace bsp
