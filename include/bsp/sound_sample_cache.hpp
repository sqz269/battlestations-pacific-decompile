#pragma once
#include "bsp/sound_system_owner.hpp"

namespace bsp {
struct SoundSampleCacheHost {
    virtual ~SoundSampleCacheHost() = default;
    // Required constructor frontier. Actual7Ch allocation, stack native name;
    // return the constructor's EAX. No fabricated sample or FMOD resource.
    virtual void* construct_sample_00a84d70(void*, const NativeString&) = 0;
    virtual void* unknown_cache_create_slot_04(SoundAuxiliaryTreeOwner&, NativeString&) = 0;
    virtual void unknown_cache_erase_slot_08(SoundAuxiliaryTreeOwner&,
        SoundAuxiliaryTreeOwner::Tree::iterator) = 0;
};
struct SoundSampleCacheContext {
    NativeStringStorage& strings;
    SoundSampleCacheHost& host;
    std::uint32_t& reset_word_00f8bbe4;
    // Live writable image storage, not immutable empty strings.
    const char* null_pattern_00e17bf0;
    const char* null_longer_data_00f8bbec;
};
//0043E9A0: ECX candidate, stack pattern/start DWORD, AL boolean, RET8.
bool native_string_matches_at_0043e9a0(const NativeString& candidate,
    const NativeString& pattern, std::uint32_t start, const char* null_pattern_00e17bf0) noexcept;
//00A85440: native ECX unused, stack name, EAX sample, RET4. Actual7Ch
// allocation invokes required00A84D70 constructor. No payload reconstruction.
void* create_sound_sample_00a85440(const NativeString&, SoundSampleCacheHost&);
//00A83FD0: ECX cache, stack out/name, EAX out, RET8. Linear inorder scan;
// equal lengths use CRT casefold, differing lengths allow case-sensitive
// path-component suffixes in either direction. Hits retain. Misses create
// through current slot4, uniquely insert a weak key/value, and return the fresh
// pointer even on a reentrant duplicate. Input and output may change in calls.
void** acquire_sound_sample_00a83fd0(SoundAuxiliaryTreeOwner&, void*& out,
    const NativeString&, SoundSampleCacheContext&);
//00A83E80: ECX cache, stack iterator address, RET4. Unlink node, destroy its
// pooled key, free node, decrement current native count only if nonzero.
// No sample release.
void erase_sound_sample_iterator_00a83e80(SoundAuxiliaryTreeOwner&,
    SoundAuxiliaryTreeOwner::Tree::iterator, NativeStringStorage&);
//00A82C00: repeatedly erase CURRENT begin through CURRENT slot8; then write
// DWORD00F8BBE4=0. Header/head remain live; payload pointers are weak.
void clear_sound_sample_cache_00a82c00(SoundAuxiliaryTreeOwner&, SoundSampleCacheContext&);
// Host std::map supplies native STL mechanics. Valid live tree/iterators and
// nonnull samples on hits are preconditions; no native layout/SEH/ABI claim.
} // namespace bsp
