#pragma once
#include "bsp/sound_retained_channel.hpp"
#include "bsp/sound_channel_runtime.hpp"

namespace bsp {
// A7F0F0: ECX=class table, signed index on stack, RET4, EAX=slot address.
// The supported domain is nonnegative indices below INT32_MAX. Grow with
// null entries through the existing A7C2C0 owner; no descriptor is invented.
SoundClassLevel* const* sound_class_slot_00a7f0f0(SoundSystemOwner&, std::int32_t);
// 54D510: ECX=reference slot, replacement pointer on stack, RET4/EAX=slot.
// Release the captured old pointer, clear after its callback, then adopt the
// replacement without retaining. Same-pointer adoption is NOT an early exit.
void*& adopt_sound_reference_0054d510(void*&, void*, VoiceReferenceHost&);

struct SoundRetainedFactoryContext {
    SoundInstanceContext& instance;
    SoundLevelNameHost& names;
    SoundChannelRuntime& channels;
};
// A7F2F0: ECX=manager; source14h*, signed slot index; RET8, void result.
// Slots 0/1/2 create D5AD58/D5ADA0/D5ADE8 respectively. The source and manager
// storage must survive callbacks. Requires the configured 3DEffect class and
// valid sample. +74 contains canonical SoundLevelEntry pointers; +80 contains
// actual native sample pointers. C++ allocations/layout are not the native ABI.
void set_retained_sound_00a7f2f0(SoundSystemOwner&, const SoundRetainedSourceRecord&,
    std::int32_t index, SoundRetainedFactoryContext&);
} // namespace bsp
