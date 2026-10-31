#pragma once
#include "bsp/native_scene_typed_lifetime.hpp"
#include "bsp/system_lighting_owners.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <intrin.h>
#include <new>
#include <limits>
#include <utility>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene typed storage requires the qualified MSVC Win32 representation.
#endif

namespace bsp::detail {
using SceneWord = std::uint32_t;
// No DMIs, owning member, semantic host tail, extra count or Native initializer.
struct ActualResource3Ch {
    SceneWord profile;
    std::atomic<std::int32_t> count;
    SceneWord name[2];
    SceneWord ambient;
    SceneWord registry[10];
};
struct ActualAmbient98h {
    SceneWord profile;
    std::atomic<std::int32_t> count;
    SystemAmbientBacklinks backlinks;
    SceneWord scalar;
    SystemLightingWords4 ambient_18;
    SystemLightingWords4 ambient_mode3_28;
    std::array<SystemLightingWords4, 6> ambient_cube_38;
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::atomic<std::int32_t>) == 4);
static_assert(alignof(std::atomic<std::int32_t>) == 4);
// This installed atomic is NOT trivially copyable. The qualified route uses
// only its actual public integral subobject, never an atomic/class roundtrip.
using SceneAtomic = std::atomic<std::int32_t>;
using SceneAtomicStorage = std::remove_reference_t<decltype(std::declval<SceneAtomic&>()._Storage)>;
using SceneAtomicValue = std::remove_reference_t<decltype(std::declval<SceneAtomic&>()._Storage._Value)>;
static_assert(!std::is_trivially_copyable_v<SceneAtomic>);
static_assert(std::is_same_v<SceneAtomicValue, std::int32_t>);
static_assert(std::is_same_v<std::make_unsigned_t<SceneAtomicValue>, SceneWord>);
static_assert(std::numeric_limits<unsigned char>::digits == 8);
static_assert(sizeof(SceneWord) == 4 && std::numeric_limits<SceneWord>::digits == 32);
static_assert(std::is_standard_layout_v<SceneAtomic>);
static_assert(std::is_standard_layout_v<SceneAtomicStorage>);
static_assert(std::is_trivially_copyable_v<SceneAtomicStorage>);
static_assert(std::is_trivially_copyable_v<SceneAtomicValue>);
static_assert(sizeof(SceneAtomic) == 4 && sizeof(SceneAtomicStorage) == 4);
static_assert(sizeof(SceneAtomicValue) == 4);
static_assert(alignof(SceneAtomic) == 4 && alignof(SceneAtomicStorage) == 4);
static_assert(alignof(SceneAtomicValue) == 4);
static_assert(offsetof(SceneAtomic, _Storage) == 0);
static_assert(offsetof(SceneAtomicStorage, _Value) == 0);
static_assert(std::has_unique_object_representations_v<SceneAtomicValue>);
static_assert(std::numeric_limits<SceneAtomicValue>::digits == 31);
static_assert((std::numeric_limits<SceneAtomicValue>::min)() == (-2147483647 - 1));
static_assert((std::numeric_limits<SceneAtomicValue>::max)() == 2147483647);
// SDK active ABI: atomic -> integral facade -> 4B storage -> _Storage._Value.
// The live int32 member occupies EVERY atomic byte, with no additional state.
// Existing write(owner,4,1) addresses exactly this member. SceneWord is its
// corresponding unsigned type; positive1 has the same representation. Raw
// Interlocked consumers retain the existing qualified MSVC address contract.
static_assert(sizeof(ActualResource3Ch) == 0x3c && alignof(ActualResource3Ch) == 4);
static_assert(offsetof(ActualResource3Ch, count) == 4);
static_assert(offsetof(ActualResource3Ch, name) == 8);
static_assert(offsetof(ActualResource3Ch, ambient) == 0x10);
static_assert(offsetof(ActualResource3Ch, registry) == 0x14);
static_assert(sizeof(ActualAmbient98h) == 0x98 && alignof(ActualAmbient98h) == 4);
static_assert(offsetof(ActualAmbient98h, count) == 4);
static_assert(offsetof(ActualAmbient98h, backlinks) == 8);
static_assert(sizeof(SystemAmbientBacklinks) == 12);
static_assert(offsetof(SystemAmbientBacklinks, count_04) == 4);
static_assert(offsetof(SystemAmbientBacklinks, capacity_08) == 8);
static_assert(offsetof(ActualAmbient98h, scalar) == 0x14);
static_assert(offsetof(ActualAmbient98h, ambient_18) == 0x18);
static_assert(offsetof(ActualAmbient98h, ambient_mode3_28) == 0x28);
static_assert(offsetof(ActualAmbient98h, ambient_cube_38) == 0x38);
static_assert(sizeof(SystemLightingWords4) == 0x10 && alignof(SystemLightingWords4) == 4);
static_assert(sizeof(decltype(ActualAmbient98h::ambient_cube_38)) == 0x60);
static_assert(alignof(decltype(ActualAmbient98h::ambient_cube_38)) == 4);
static_assert(std::is_standard_layout_v<SystemLightingWords4>);
static_assert(std::is_standard_layout_v<decltype(ActualAmbient98h::ambient_cube_38)>);
static_assert(std::is_trivially_copyable_v<SystemLightingWords4>);
static_assert(std::is_trivially_copyable_v<decltype(ActualAmbient98h::ambient_cube_38)>);
static_assert(std::is_standard_layout_v<ActualResource3Ch>);
static_assert(std::is_standard_layout_v<ActualAmbient98h>);
static_assert(!std::is_trivially_copyable_v<ActualResource3Ch>);
static_assert(!std::is_trivially_copyable_v<ActualAmbient98h>);
// Literal complete byte coverage: no omitted padding or overlapping restores.
static_assert(offsetof(ActualResource3Ch, profile) == 0);
static_assert(offsetof(ActualResource3Ch, count) == sizeof(SceneWord));
static_assert(offsetof(ActualResource3Ch, name) == offsetof(ActualResource3Ch, count) + sizeof(SceneAtomicValue));
static_assert(offsetof(ActualResource3Ch, ambient) == offsetof(ActualResource3Ch, name) + sizeof(decltype(ActualResource3Ch::name)));
static_assert(offsetof(ActualResource3Ch, registry) == offsetof(ActualResource3Ch, ambient) + sizeof(SceneWord));
static_assert(sizeof(ActualResource3Ch) == offsetof(ActualResource3Ch, registry) + sizeof(decltype(ActualResource3Ch::registry)));
static_assert(offsetof(ActualAmbient98h, profile) == 0);
static_assert(offsetof(ActualAmbient98h, count) == sizeof(SceneWord));
static_assert(offsetof(ActualAmbient98h, backlinks) == offsetof(ActualAmbient98h, count) + sizeof(SceneAtomicValue));
static_assert(offsetof(SystemAmbientBacklinks, begin_00) == 0);
static_assert(offsetof(SystemAmbientBacklinks, count_04) == sizeof(decltype(SystemAmbientBacklinks::begin_00)));
static_assert(offsetof(SystemAmbientBacklinks, capacity_08) == offsetof(SystemAmbientBacklinks, count_04) + sizeof(std::int32_t));
static_assert(sizeof(SystemAmbientBacklinks) == offsetof(SystemAmbientBacklinks, capacity_08) + sizeof(std::int32_t));
static_assert(std::is_trivially_copyable_v<SystemAmbientBacklinks>);
static_assert(offsetof(ActualAmbient98h, scalar) == offsetof(ActualAmbient98h, backlinks) + sizeof(SystemAmbientBacklinks));
static_assert(offsetof(ActualAmbient98h, ambient_18) == offsetof(ActualAmbient98h, scalar) + sizeof(SceneWord));
static_assert(offsetof(ActualAmbient98h, ambient_mode3_28) == offsetof(ActualAmbient98h, ambient_18) + sizeof(SystemLightingWords4));
static_assert(offsetof(ActualAmbient98h, ambient_cube_38) == offsetof(ActualAmbient98h, ambient_mode3_28) + sizeof(SystemLightingWords4));
static_assert(sizeof(ActualAmbient98h) == offsetof(ActualAmbient98h, ambient_cube_38) + sizeof(decltype(ActualAmbient98h::ambient_cube_38)));
static_assert(offsetof(ActualResource3Ch, count) + offsetof(SceneAtomic, _Storage) + offsetof(SceneAtomicStorage, _Value) == 4);
static_assert(offsetof(ActualAmbient98h, count) + offsetof(SceneAtomic, _Storage) + offsetof(SceneAtomicStorage, _Value) == 4);
static_assert(std::is_nothrow_default_constructible_v<ActualResource3Ch>);
static_assert(std::is_nothrow_default_constructible_v<ActualAmbient98h>);
static_assert(std::is_trivially_destructible_v<ActualResource3Ch>);
static_assert(std::is_trivially_destructible_v<ActualAmbient98h>);

inline SceneWord scene_word(const void* p) noexcept {
    SceneWord value;
    _ReadWriteBarrier(); std::memcpy(&value, p, 4); _ReadWriteBarrier();
    return value;
}
inline void scene_word(void* p, SceneWord value) noexcept {
    _ReadWriteBarrier(); std::memcpy(p, &value, 4); _ReadWriteBarrier();
}
template<class T> void restore_actual_subobject(T& destination,
    const unsigned char* preimage) noexcept {
    static_assert(std::is_trivially_copyable_v<T>);
    // Bind/address the live scalar/array/member; do not read its typed value.
    auto* const bytes = reinterpret_cast<volatile unsigned char*>(&destination);
    for (std::size_t i = 0; i != sizeof(T); ++i) bytes[i] = preimage[i];
}
inline void restore_scene_members(ActualResource3Ch& owner,
    const unsigned char* preimage) noexcept {
    restore_actual_subobject(owner.profile, preimage);
    restore_actual_subobject(owner.count._Storage._Value, preimage + 4);
    restore_actual_subobject(owner.name, preimage + 8);
    restore_actual_subobject(owner.ambient, preimage + 0x10);
    restore_actual_subobject(owner.registry, preimage + 0x14);
}
inline void restore_scene_members(ActualAmbient98h& owner,
    const unsigned char* preimage) noexcept {
    restore_actual_subobject(owner.profile, preimage);
    restore_actual_subobject(owner.count._Storage._Value, preimage + 4);
    restore_actual_subobject(owner.backlinks, preimage + 8);
    restore_actual_subobject(owner.scalar, preimage + 0x14);
    restore_actual_subobject(owner.ambient_18, preimage + 0x18);
    restore_actual_subobject(owner.ambient_mode3_28, preimage + 0x28);
    restore_actual_subobject(owner.ambient_cube_38, preimage + 0x38);
}

template<class T> T* begin_scene_storage(void* owner) noexcept {
    // Save raw bytes only. Restore real trivial subobjects individually;
    // the atomic object and carrier are never copied as complete objects.
    // No evaluation of an arbitrary count/pointer preimage as a C++ value.
    // The SDK atomic default constructor may transiently write its base.
    unsigned char preimage[sizeof(T)];
    auto* const source = static_cast<const volatile unsigned char*>(owner);
    _ReadWriteBarrier();
    for (std::size_t i = 0; i != sizeof(T); ++i) preimage[i] = source[i];
    _ReadWriteBarrier();
    auto* result = ::new (owner) T;
    _ReadWriteBarrier();
    restore_scene_members(*result, preimage);
    _ReadWriteBarrier();
    return result;
}

struct NativeSceneTypedStorageAccess {
    static bool fresh(const NativeSceneResourceTypedLifetime& a) noexcept {
        return !a.actual_ && !a.born_ && !a.returned_ && !a.destroying_ && !a.ended_;
    }
    static bool fresh(const NativeAmbientTypedLifetime& a) noexcept {
        return !a.actual_ && !a.born_ && !a.returned_ && !a.destroying_ && !a.ended_;
    }
    static void require(void* p, NativeSceneResourceStorageContext& c,
        const NativeSceneResourceTypedLifetime& a) {
        if (a.actual_ != p || a.context_ != &c || !a.born_ || !a.returned_ ||
            a.destroying_ || a.ended_ || a.failed_)
            throw std::logic_error("resource requires its actual constructor-issued live extent");
    }
    static void require(void* p, NativeAmbientIdentityContext& c,
        const NativeAmbientTypedLifetime& a) {
        if (a.actual_ != p || a.context_ != &c || !a.born_ || !a.returned_ ||
            a.destroying_ || a.ended_ || a.failed_)
            throw std::logic_error("ambient requires its actual constructor-issued live extent");
    }
    static std::atomic<std::int32_t>& count(void* p, NativeSceneResourceStorageContext& c,
        const NativeSceneResourceTypedLifetime& a) {
        require(p, c, a);
        return std::launder(static_cast<ActualResource3Ch*>(p))->count;
    }
    static std::atomic<std::int32_t>& count(void* p, NativeAmbientIdentityContext& c,
        const NativeAmbientTypedLifetime& a) {
        require(p, c, a);
        return std::launder(static_cast<ActualAmbient98h*>(p))->count;
    }
    static void enter(void* p, NativeSceneResourceStorageContext& c,
        NativeSceneResourceTypedLifetime& a) { require(p, c, a); a.destroying_ = true; }
    static void enter(void* p, NativeAmbientTypedLifetime& a) {
        if (a.actual_ != p || !a.born_ || !a.returned_ || a.destroying_ || a.ended_ || a.failed_)
            throw std::logic_error("ambient destruction requires its actual live extent");
        a.destroying_ = true;
    }
    static void end(NativeSceneResourceTypedLifetime& a, bool failed) noexcept {
        if (!a.born_ || a.ended_) return;
        a.ended_ = true; a.failed_ = a.failed_ || failed;
        std::launder(static_cast<ActualResource3Ch*>(a.actual_))->~ActualResource3Ch();
    }
    static void end(NativeAmbientTypedLifetime& a, bool failed) noexcept {
        if (!a.born_ || a.ended_) return;
        a.ended_ = true; a.failed_ = a.failed_ || failed;
        std::launder(static_cast<ActualAmbient98h*>(a.actual_))->~ActualAmbient98h();
    }
};
} // namespace bsp::detail
