#pragma once
#include "bsp/native_scene_typed_lifetime.hpp"
#include "bsp/system_lighting_owners.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <intrin.h>
#include <new>
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
    SceneWord lighting[32];
};
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::atomic<std::int32_t>) == 4);
static_assert(alignof(std::atomic<std::int32_t>) == 4);
static_assert(std::is_trivially_copyable_v<std::atomic<std::int32_t>>);
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
static_assert(offsetof(ActualAmbient98h, lighting) == 0x18);
static_assert(std::is_standard_layout_v<ActualResource3Ch>);
static_assert(std::is_standard_layout_v<ActualAmbient98h>);
static_assert(std::is_trivially_copyable_v<ActualResource3Ch>);
static_assert(std::is_trivially_copyable_v<ActualAmbient98h>);
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
template<class T> T* begin_scene_storage(void* owner) noexcept {
    // No evaluation of an arbitrary count/pointer preimage as a C++ value.
    // The SDK atomic default constructor may transiently write its base.
    unsigned char preimage[sizeof(T)];
    auto* const source = static_cast<const volatile unsigned char*>(owner);
    _ReadWriteBarrier();
    for (std::size_t i = 0; i != sizeof(T); ++i) preimage[i] = source[i];
    _ReadWriteBarrier();
    auto* result = ::new (owner) T;
    auto* const destination = static_cast<volatile unsigned char*>(owner);
    _ReadWriteBarrier();
    for (std::size_t i = 0; i != sizeof(T); ++i) destination[i] = preimage[i];
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
