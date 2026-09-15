#pragma once
#include "bsp/native_game_resource_named_groups.hpp"
#include "bsp/native_input_keyboard_iterators.hpp"
#include "bsp/plane_pose_commit.hpp"

namespace bsp {
struct NativeDamageableClassBindingContext {
    const SingletonLifetimeCallbacks& invalid_parameters;
    const NativePoseOrthonormalizationAccess& pose;
    const volatile std::uint32_t& one_00d7a24c;
    const char* const volatile* explosion_categories_00e08138;
};

// All storage is borrowed. Original 00879AD0 uses ECX class and RET. It writes
// crew frames in the class+60 tree, explosion positions in class+18's 30h rows,
// fake-explosion positions in class+28's counted point array, then copies each
// current explosion position into its secondary position. No class copy/retain.
void bind_native_damageable_class_model_00879ad0(
    void* actual_class, const NativeDamageableClassBindingContext&);

// Original ECX point item, stack output string, EAX output, RET4.
NativeLegacySboStringStorage& copy_native_point_group_category_00879a70(
    const void* item, NativeLegacySboStringStorage& output);
// Original ECX class, stack category/index, EAX first matching row, RET8.
void* find_native_damageable_effect_row_00876da0(const void* actual_class,
    std::uint32_t category, std::uint32_t index, const SingletonLifetimeCallbacks&);
// Existing generic checked-tree successor, specialized to nil byte +55h.
// Original ECX two-word owner/node iterator, no stack arguments, RET.
void advance_native_class_frame_008772b0(NativeKeyboardTreeIterator*,
    const SingletonLifetimeCallbacks&);
// Original ECX {data,count,capacity} header, stack signed capacity, RET4.
// Clamp to >=1, grow only, preserve count, copy via ordered x87 pairs, free old
// storage before publishing replacement. Ordinary canonical allocation boundary.
void reserve_native_class_point_array_0074d190(void* header, std::int32_t capacity);
} // namespace bsp
