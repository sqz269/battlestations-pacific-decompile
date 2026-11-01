#pragma once
#include "bsp/native_material_owner.hpp"
#include <intrin.h>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Material member storage requires the qualified installed MSVC Win32 representation.
#endif

namespace bsp::detail {
using MaterialAtomic = std::atomic<std::int32_t>;
using MaterialAtomicStorage = std::remove_reference_t<decltype(std::declval<MaterialAtomic&>()._Storage)>;
using MaterialAtomicValue = std::remove_reference_t<decltype(std::declval<MaterialAtomic&>()._Storage._Value)>;
// Same installed-SDK route as the existing resource/ambient storage proof.
// Only the live integral subobject is restored; neither atomic nor carrier is
// treated as a trivially-copyable complete object. These are real compile
// gates, not a claim that Material traits have already been compiled.
static_assert(!std::is_trivially_copyable_v<MaterialAtomic>);
static_assert(std::is_standard_layout_v<MaterialAtomic>);
static_assert(std::is_standard_layout_v<MaterialAtomicStorage>);
static_assert(std::is_trivially_copyable_v<MaterialAtomicStorage>);
static_assert(std::is_same_v<MaterialAtomicValue, std::int32_t>);
static_assert(std::is_same_v<std::make_unsigned_t<MaterialAtomicValue>, std::uint32_t>);
static_assert(std::is_trivially_copyable_v<MaterialAtomicValue>);
static_assert(std::has_unique_object_representations_v<MaterialAtomicValue>);
static_assert(sizeof(MaterialAtomic) == 4 && alignof(MaterialAtomic) == 4);
static_assert(sizeof(MaterialAtomicStorage) == 4 && alignof(MaterialAtomicStorage) == 4);
static_assert(sizeof(MaterialAtomicValue) == 4 && alignof(MaterialAtomicValue) == 4);
static_assert(offsetof(MaterialAtomic, _Storage) == 0);
static_assert(offsetof(MaterialAtomicStorage, _Value) == 0);
static_assert(std::numeric_limits<MaterialAtomicValue>::digits == 31);
static_assert((std::numeric_limits<MaterialAtomicValue>::min)() == (-2147483647 - 1));
static_assert((std::numeric_limits<MaterialAtomicValue>::max)() == 2147483647);
static_assert(std::numeric_limits<unsigned char>::digits == 8);
static_assert(sizeof(void*) == 4 && alignof(void*) == 4);
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);
static_assert(sizeof(std::int16_t) == 2 && sizeof(std::int32_t) == 4);
static_assert(sizeof(std::byte) == 1 && alignof(std::byte) == 1);
static_assert(std::is_standard_layout_v<NativeMaterialStorage>);
static_assert(!std::is_trivially_copyable_v<NativeMaterialStorage>);
static_assert(std::is_nothrow_default_constructible_v<NativeMaterialStorage>);
static_assert(std::is_trivially_destructible_v<NativeMaterialStorage>);
static_assert(sizeof(NativeMaterialStorage) == 0x110 && alignof(NativeMaterialStorage) == 4);

// Every real member extent is checked, including the explicit preserved bytes.
// The next-offset checks prove there is no omitted inter-member/tail padding;
// array sizes prove no additional wrapper state beyond their element backing.
#define BSP_MATERIAL_MEMBER(member, offset, bytes) \
    static_assert(offsetof(NativeMaterialStorage, member) == offset); \
    static_assert(sizeof(decltype(NativeMaterialStorage::member)) == bytes); \
    static_assert(std::is_trivially_copyable_v<decltype(NativeMaterialStorage::member)>)
BSP_MATERIAL_MEMBER(vtable_00, 0x00, 4);
// The actual atomic's integral payload covers this entire next4B interval.
static_assert(offsetof(NativeMaterialStorage, references_04) == 4);
BSP_MATERIAL_MEMBER(word_08, 0x08, 4);
BSP_MATERIAL_MEMBER(source_owner_0c, 0x0c, 4);
BSP_MATERIAL_MEMBER(textures_10, 0x10, 9 * 4);
BSP_MATERIAL_MEMBER(texture_count_34, 0x34, 2);
BSP_MATERIAL_MEMBER(preserved_36, 0x36, 2);
BSP_MATERIAL_MEMBER(lighting_38, 0x38, 17 * 4);
BSP_MATERIAL_MEMBER(effect_7c, 0x7c, 4);
BSP_MATERIAL_MEMBER(parameters_80, 0x80, 32 * 4);
BSP_MATERIAL_MEMBER(parameter_count_100, 0x100, 4);
BSP_MATERIAL_MEMBER(word_104, 0x104, 4);
BSP_MATERIAL_MEMBER(word_108, 0x108, 4);
BSP_MATERIAL_MEMBER(lighting_enabled_10c, 0x10c, 1);
BSP_MATERIAL_MEMBER(retain_source_10d, 0x10d, 1);
BSP_MATERIAL_MEMBER(preserved_10e, 0x10e, 2);
#undef BSP_MATERIAL_MEMBER
#define BSP_MATERIAL_ADJACENT(left, right) \
    static_assert(offsetof(NativeMaterialStorage, right) == \
        offsetof(NativeMaterialStorage, left) + sizeof(decltype(NativeMaterialStorage::left)))
BSP_MATERIAL_ADJACENT(vtable_00, references_04);
BSP_MATERIAL_ADJACENT(references_04, word_08);
BSP_MATERIAL_ADJACENT(word_08, source_owner_0c);
BSP_MATERIAL_ADJACENT(source_owner_0c, textures_10);
BSP_MATERIAL_ADJACENT(textures_10, texture_count_34);
BSP_MATERIAL_ADJACENT(texture_count_34, preserved_36);
BSP_MATERIAL_ADJACENT(preserved_36, lighting_38);
BSP_MATERIAL_ADJACENT(lighting_38, effect_7c);
BSP_MATERIAL_ADJACENT(effect_7c, parameters_80);
BSP_MATERIAL_ADJACENT(parameters_80, parameter_count_100);
BSP_MATERIAL_ADJACENT(parameter_count_100, word_104);
BSP_MATERIAL_ADJACENT(word_104, word_108);
BSP_MATERIAL_ADJACENT(word_108, lighting_enabled_10c);
BSP_MATERIAL_ADJACENT(lighting_enabled_10c, retain_source_10d);
BSP_MATERIAL_ADJACENT(retain_source_10d, preserved_10e);
#undef BSP_MATERIAL_ADJACENT
static_assert(sizeof(NativeMaterialStorage) == offsetof(NativeMaterialStorage, preserved_10e) + 2);

template<class T> inline void restore_material_subobject(T& destination,
    const unsigned char* preimage) noexcept {
    static_assert(std::is_trivially_copyable_v<T>);
    // Address the born member without evaluating an arbitrary pointer/count/FP
    // preimage. Byte restoration adds no FP conversion, atomic operation or
    // count1 initialization; its physical byte writes are Source birth work.
    auto* const bytes = reinterpret_cast<volatile unsigned char*>(&destination);
    for (std::size_t i = 0; i != sizeof(T); ++i) bytes[i] = preimage[i];
}
inline NativeMaterialStorage* begin_material_storage(void* slot) noexcept {
    // Caller retains the existing fresh aligned110h storage/exclusive-birth
    // contract. +110 is the separately live pool ID and is never touched.
    unsigned char preimage[sizeof(NativeMaterialStorage)];
    auto* const source = static_cast<const volatile unsigned char*>(slot);
    _ReadWriteBarrier();
    for (std::size_t i = 0; i != sizeof(preimage); ++i) preimage[i] = source[i];
    _ReadWriteBarrier();
    auto* const material = ::new (slot) NativeMaterialStorage;
    _ReadWriteBarrier();
    restore_material_subobject(material->vtable_00, preimage);
    restore_material_subobject(material->references_04._Storage._Value, preimage + 4);
    restore_material_subobject(material->word_08, preimage + 0x08);
    restore_material_subobject(material->source_owner_0c, preimage + 0x0c);
    restore_material_subobject(material->textures_10, preimage + 0x10);
    restore_material_subobject(material->texture_count_34, preimage + 0x34);
    restore_material_subobject(material->preserved_36, preimage + 0x36);
    restore_material_subobject(material->lighting_38, preimage + 0x38);
    restore_material_subobject(material->effect_7c, preimage + 0x7c);
    restore_material_subobject(material->parameters_80, preimage + 0x80);
    restore_material_subobject(material->parameter_count_100, preimage + 0x100);
    restore_material_subobject(material->word_104, preimage + 0x104);
    restore_material_subobject(material->word_108, preimage + 0x108);
    restore_material_subobject(material->lighting_enabled_10c, preimage + 0x10c);
    restore_material_subobject(material->retain_source_10d, preimage + 0x10d);
    restore_material_subobject(material->preserved_10e, preimage + 0x10e);
    _ReadWriteBarrier();
    return material;
}
} // namespace bsp::detail
