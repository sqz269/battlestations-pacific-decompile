#include "bsp/sound_channel_runtime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <exception>
#include <stdexcept>

namespace bsp {
SoundChannelInstance& SoundChannelRuntime::channel(SoundLevelEntry* entry) {
    auto& base = *static_cast<SoundInstance*>(entry);
    if (base.native_vtable_00 != 0x00d5abf8) throw std::logic_error("Unbound active sound instance profile");
    return static_cast<SoundChannelInstance&>(base);
}
SoundLevelEntry* SoundChannelRuntime::create_nonspatial(SoundSystemOwner& owner, void* sample,
    std::int32_t class_index, std::int32_t type_index, bool flag) {
    return create_nonspatial_sound_00a7f640(owner, sample, class_index,
        static_cast<std::uint32_t>(type_index), static_cast<std::uint8_t>(flag), context_);
}
void SoundChannelRuntime::retain_reference(void* canonical_entry) {
    auto& s = channel(static_cast<SoundLevelEntry*>(canonical_entry));
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&s.references_04));
}
void SoundChannelRuntime::release_reference(void* canonical_entry) noexcept {
    // Native slot teardown is a nonthrowing callback contract. Missing FMOD
    // symbols or an unsupported profile cannot be converted into a fake release.
    try {
        auto& s = channel(static_cast<SoundLevelEntry*>(canonical_entry));
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(&s.references_04)) == 0)
            scalar_delete_sound_channel_00a7d5a0(&s, 1, context_);
    } catch (...) { std::terminate(); }
}
SoundInstance& SoundChannelRuntime::instance_fields(SoundLevelEntry* entry) { return channel(entry); }
void SoundChannelRuntime::update_slot30(SoundLevelEntry* entry, float dt, SoundListenerOwnerState& listener) {
    static_assert(sizeof(void*) == 4);
    update_sound_channel_00a7af10(channel(entry), dt, reinterpret_cast<std::uint32_t>(&listener), context_);
}
bool SoundChannelRuntime::transition_slot24(SoundLevelEntry* entry) { return sound_channel_virtual_transition_00a79990(channel(entry)); }
void SoundChannelRuntime::refresh_slot10(SoundLevelEntry* entry) { refresh_sound_channel_ended_00a7a660(channel(entry), context_); }
bool SoundChannelRuntime::completed_slot0c(SoundLevelEntry* entry) { return sound_channel_completed_00a799b0(channel(entry), context_); }
bool SoundChannelRuntime::nonvirtual_slot14(SoundLevelEntry* entry) { return sound_channel_nonvirtual_00a7a630(channel(entry), context_); }
void SoundChannelRuntime::stop_slot08(SoundLevelEntry* entry, std::uint8_t flag) { stop_sound_channel_00a7a570(channel(entry), flag, context_); }
} // namespace bsp
