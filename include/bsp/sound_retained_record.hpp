#pragma once

#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>

namespace bsp {
// Actual14h source record, embedded at +5C in retained channel variants.
// Field names are hypotheses: +0 is a sample acquired by 00A83FD0;
// +4/+8 is its native name header; +C/+10 are x87-copied floats.
// No implicit ownership cleanup. Evidence: docs/SOUND_RETAINED_RECORD.md.
struct SoundRetainedSourceRecord {
    void* sample_00{};
    NativeString name_04;
    float field_0c{};
    float field_10{};
};
static_assert(sizeof(void*) == 4, "Retained source storage requires Win32");
static_assert(sizeof(SoundRetainedSourceRecord) == 0x14);
static_assert(offsetof(SoundRetainedSourceRecord, name_04) == 4);
static_assert(offsetof(SoundRetainedSourceRecord, field_0c) == 0xc);
static_assert(offsetof(SoundRetainedSourceRecord, field_10) == 0x10);

// Full00A7C5F0..00A7C692. ECX destination, stack source; EAX destination;
// RET4. Accepts actual14h storage, including an owner's bytes+5C. Clear
// destination sample before reading source, retain actual sample+4, clear
// string header then deep-copy it, and copy floats sequentially with x87.
// On a C++ allocation exception, release CURRENT destination sample and clear
// its slot after the zero-reference callback. No string cleanup is registered
// by the original constructor. Self-construction abandons sample/name.
void* copy_sound_retained_source_record_00a7c5f0(void* actual_destination,
    const void* actual_source, NativeStringStorage&, GameplayEffectComponentLifetime&);

// Full004C7FA0..004C801D. ECX actual14h record, RET. Destroy current string
// buffer leaving its header stale, then reload sample, atomically decrement
// its actual+4 and dispatch current vtable+0 through the required lifetime
// service only at zero. Clear captured sample slot AFTER that callback.
// Explicit NativeStringStorage::release is noexcept; native pool-getter SEH
// is outside this host interface. No cleanup retry after a zero callback throws.
void destroy_sound_retained_source_record_004c7fa0(void* actual_record,
    NativeStringStorage&, GameplayEffectComponentLifetime&);

// Full004E7BB0..004E7BF0. ECX destination pointer slot, stack source pointer
// slot; EAX destination, RET4. Capture both pointers; equal values do nothing.
// Publish new sample, atomically retain it, then release the captured old
// sample. Never clear/restore the destination after a zero-reference callback.
void* assign_sound_sample_reference_004e7bb0(void* actual_destination_slot,
    const void* actual_source_slot, GameplayEffectComponentLifetime&);
} // namespace bsp
