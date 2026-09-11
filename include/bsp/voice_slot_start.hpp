#pragma once

#include "bsp/sound_system_owner.hpp"
#include "bsp/voice_playback.hpp"

namespace bsp {

// Native process globals, not per-slot defaults. Bit0 is set BEFORE lookup;
// reentry sees the previous class value. Initialization is not synchronized.
struct VoiceSlotStartGlobals {
    std::uint32_t guard_00e19ac8{};
    std::int32_t warnings_class_00e19ac4{};
};
struct VoiceSoundStartFields {
    std::uint8_t enabled_38{};
    float duration_3c{};
};

// Actual opaque-resource/virtual/alternate-engine boundaries. Resource+8 is
// a descriptor beginning with kind0/1; it is NOT SoundOwnedResource's FMOD
// event-project pointer at +8. No asset cast or fabricated successful load.
class VoiceSlotStartHost : public SoundLevelNameHost {
public:
    virtual SoundSystemOwner& current_sound_owner_00f8bbd8() = 0;
    virtual std::int32_t resource_kind_08(void* resource) = 0;
    // Return one owned intrusive reference (or native null), identity is the
    // canonical SoundLevelEntry used by the existing dirty/volume walkers.
    virtual SoundLevelEntry* create_sound_vslot_0c(SoundSystemOwner&,
        void* resource, std::int32_t class_index, std::int32_t type_index,
        bool flag) = 0;
    virtual SoundLevelEntry* create_sound_vslot_10(SoundSystemOwner&,
        void* resource, std::int32_t class_index, std::int32_t type_index,
        bool flag) = 0;
    virtual VoiceSoundStartFields& sound_start_fields(void* sound) = 0;
    // Must perform the real alternate-engine named request. Inspection shows
    // A78CE0 ignores its second stack argument and enqueues via A786F0; the
    // caller still supplies the recovered 0.1f. No implicit success fallback.
    virtual void play_alternate_00a78ce0(const NativeString&, float argument) = 0;
    // Only slot0 is established. Native computes manager+8+24*index without
    // a guard, including -1. A host mapping must preserve that index/address
    // behavior or raise a fault; it must not silently substitute slot0.
    virtual VoicePlaybackSlot& resolve_nonzero_slot(VoicePlaybackManager&,
        std::int32_t index) = 0;
    virtual void log_sound_004254b0(const char* format, const char* text,
        std::uint32_t resource_index) = 0;
};
struct VoiceSlotStartContext {
    VoiceSlotStartHost& host;
    VoiceSlotStartGlobals& globals;
    NativeStringStorage& strings;
};

// Typed projections, not native memory layouts or drop-in Win32 entrypoints.
// Each ABI/evidence/limit is documented in docs/VOICE_SLOT_START.md.
std::int32_t find_sound_type_00a7b0a0(const SoundConfigurationState&,
    const NativeString&, SoundLevelNameHost&);
void assign_voice_reference_0054d4c0(void*& destination, void* source,
    VoiceLineHost&);
// Containers require 0<=count<=capacity<=INT32_MAX/2 and stable structure
// during intrusive releases. Retain is the native nonthrowing atomic primitive.
void reserve_voice_sound_entries_00a7c080(SoundSystemOwner&, std::int32_t capacity,
    VoiceLineHost&);
void append_voice_sound_entry_00a7d5c0(SoundSystemOwner&, SoundLevelEntry*,
    VoiceLineHost&);
SoundLevelEntry* create_voice_sound_00a7e490(SoundSystemOwner&, void* resource,
    std::int32_t class_index, std::int32_t type_index, bool flag,
    VoiceLineHost&, VoiceSlotStartHost&);
void set_voice_sound_start_00a798c0(VoiceSoundStartFields&, float duration) noexcept;

// ECX=fresh18h slot; Clip12* and owned resource argument stack; RET8/EAX=this.
// Consumes exactly one reference to bank, even for negative record sound IDs.
VoicePlaybackSlot& construct_voice_slot_00702cc0(VoicePlaybackSlot&,
    const VoiceClip&, void* bank, VoiceLineHost&, VoiceSlotStartContext&);
// ECX=destination; slot* stack; RET4/EAX=this. State first; current source
// auxiliary/string/timestamp are read after preceding lifetime callbacks.
VoicePlaybackSlot& assign_voice_slot_005b82c0(VoicePlaybackSlot&,
    const VoicePlaybackSlot&, VoiceLineHost&, NativeStringStorage&);
// ECX=slot; RET. String destruction leaves its header untouched, then releases
// and clears auxiliary, then releases and clears current sound. No stop calls.
void destroy_voice_slot_005b7fc0(VoicePlaybackSlot&, VoiceLineHost&,
    NativeStringStorage&) noexcept;
// ECX=manager; index + Clip12 by value + owned resource stack; RET14.
// Constructs temporary, resolves destination, assigns, destroys temporary,
// polls destination, logs (played is unconditional), then consumes argument.
void start_voice_clip_005b9050(VoicePlaybackManager&, std::int32_t slot_index,
    VoiceClip, void* bank, VoiceLineHost&, VoiceSlotStartContext&);

} // namespace bsp
