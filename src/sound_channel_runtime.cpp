#include "bsp/sound_channel_runtime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <exception>
#include <stdexcept>

namespace bsp {
SoundChannelRuntime::SoundChannelRuntime(SoundSpatialChannelContext& spatial, SoundEventInstanceContext& events)
    : context_(spatial.instance), spatial_(&spatial), events_(&events) {
    if (&events.instance != &context_) throw std::logic_error("Sound runtime contexts must share instance services");
}
SoundInstance& SoundChannelRuntime::checked(SoundLevelEntry* entry) {
    auto& base = *static_cast<SoundInstance*>(entry);
    if (base.native_vtable_00 != 0x00d5abf8 && !(base.native_vtable_00 == 0x00d5b510 && spatial_) &&
        !(base.native_vtable_00 == 0x00d5b4c8 && events_))
        throw std::logic_error("Unbound active sound instance profile");
    return base;
}
SoundChannelInstance& SoundChannelRuntime::channel(SoundLevelEntry* entry) {
    auto& base = checked(entry);
    if (base.native_vtable_00 == 0x00d5b4c8) throw std::logic_error("Event object is not a channel");
    return static_cast<SoundChannelInstance&>(base);
}
SpatialSoundEventInstance& SoundChannelRuntime::event(SoundLevelEntry* entry) {
    auto& base = checked(entry);
    if (base.native_vtable_00 != 0x00d5b4c8) throw std::logic_error("Channel object is not an event");
    return static_cast<SpatialSoundEventInstance&>(base);
}
SoundLevelEntry* SoundChannelRuntime::create_nonspatial(SoundSystemOwner& owner, void* sample,
    std::int32_t class_index, std::int32_t type_index, bool flag) {
    return create_nonspatial_sound_00a7f640(owner, sample, class_index,
        static_cast<std::uint32_t>(type_index), static_cast<std::uint8_t>(flag), context_);
}
void SoundChannelRuntime::retain_reference(void* canonical_entry) {
    auto& s = checked(static_cast<SoundLevelEntry*>(canonical_entry));
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&s.references_04));
}
SoundLevelEntry* SoundChannelRuntime::create_spatial_bank(SoundSystemOwner& owner, void* sample,
    std::int32_t class_index, std::int32_t type_index, bool flag) {
    if (!spatial_) throw std::logic_error("Spatial sound context is not bound");
    return create_spatial_bank_sound_00a7f710_fragment(owner, sample, class_index,
        static_cast<std::uint32_t>(type_index), static_cast<std::uint8_t>(flag), context_);
}
void SoundChannelRuntime::release_reference(void* canonical_entry) noexcept {
    // Native slot teardown is a nonthrowing callback contract. Missing FMOD
    // symbols or an unsupported profile cannot be converted into a fake release.
    try {
        auto& s = checked(static_cast<SoundLevelEntry*>(canonical_entry));
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(&s.references_04)) == 0) {
            if (s.native_vtable_00 == 0x00d5b4c8)
                scalar_delete_spatial_sound_event_00a89ee0(static_cast<SpatialSoundEventInstance*>(&s), 1, *events_);
            else if (s.native_vtable_00 == 0x00d5b510)
                scalar_delete_spatial_sound_channel_00a8a460(static_cast<SpatialSoundChannelInstance*>(&s), 1, *spatial_);
            else scalar_delete_sound_channel_00a7d5a0(static_cast<SoundChannelInstance*>(&s), 1, context_);
        }
    } catch (...) { std::terminate(); }
}
SoundLevelEntry* SoundChannelRuntime::create_spatial(SoundSystemOwner& owner, void* sample,
    std::int32_t cls, std::int32_t type, bool flag) {
    if (!spatial_ || !events_) throw std::logic_error("Both spatial factory profiles must be bound");
    return create_spatial_sound_00a7f710(owner, sample, cls, static_cast<std::uint32_t>(type), static_cast<std::uint8_t>(flag), context_);
}
SoundInstance& SoundChannelRuntime::instance_fields(SoundLevelEntry* entry) { return checked(entry); }
void SoundChannelRuntime::update_slot30(SoundLevelEntry* entry, float dt, SoundListenerOwnerState& listener) {
    static_assert(sizeof(void*) == 4);
    auto& s = checked(entry);
    if (s.native_vtable_00 == 0x00d5b4c8)
        update_spatial_sound_event_00a89770(static_cast<SpatialSoundEventInstance&>(s), dt, listener, *events_);
    else if (s.native_vtable_00 == 0x00d5b510)
        update_spatial_sound_channel_00a8a480(static_cast<SpatialSoundChannelInstance&>(s), dt, listener, *spatial_);
    else update_sound_channel_00a7af10(static_cast<SoundChannelInstance&>(s), dt, reinterpret_cast<std::uint32_t>(&listener), context_);
}
bool SoundChannelRuntime::transition_slot24(SoundLevelEntry* entry) {
    if (checked(entry).native_vtable_00 == 0x00d5b4c8) return sound_event_virtual_transition_00a89c70();
    return sound_channel_virtual_transition_00a79990(channel(entry));
}
void SoundChannelRuntime::refresh_slot10(SoundLevelEntry* entry) {
    if (checked(entry).native_vtable_00 == 0x00d5b4c8) { events_->virtuals.refresh_slot10(event(entry), *events_); return; }
    refresh_sound_channel_ended_00a7a660(channel(entry), context_);
}
bool SoundChannelRuntime::completed_slot0c(SoundLevelEntry* entry) {
    if (checked(entry).native_vtable_00 == 0x00d5b4c8) return spatial_sound_event_completed_00a88ba0(event(entry), *events_);
    return sound_channel_completed_00a799b0(channel(entry), context_);
}
bool SoundChannelRuntime::nonvirtual_slot14(SoundLevelEntry* entry) {
    if (checked(entry).native_vtable_00 == 0x00d5b4c8) return sound_event_nonvirtual_00a89c40();
    return sound_channel_nonvirtual_00a7a630(channel(entry), context_);
}
void SoundChannelRuntime::stop_slot08(SoundLevelEntry* entry, std::uint8_t flag) {
    if (checked(entry).native_vtable_00 == 0x00d5b4c8) { stop_sound_event_00a89170(event(entry), flag, *events_); return; }
    stop_sound_channel_00a7a570(channel(entry), flag, context_);
}
} // namespace bsp
