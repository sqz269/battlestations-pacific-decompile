#include "bsp/voice_slot_start.hpp"

#include <algorithm>
#include <utility>

namespace bsp {
namespace {
class OwnedReference {
public:
    OwnedReference(void* value, VoiceLineHost& host) : value_(value), host_(host) {}
    ~OwnedReference() { if (value_) host_.release_reference(value_); }
    OwnedReference(const OwnedReference&) = delete;
    OwnedReference& operator=(const OwnedReference&) = delete;
private:
    void* value_;
    VoiceLineHost& host_;
};
class ScopedText {
public:
    ScopedText(NativeStringStorage& storage, const char* text) : storage_(storage) {
        value.assign_0041e870(storage, text);
    }
    ~ScopedText() { value.release_to(storage_); }
    NativeString value;
private:
    NativeStringStorage& storage_;
};
class SlotCleanup {
public:
    SlotCleanup(VoicePlaybackSlot& slot, VoiceLineHost& host, NativeStringStorage& strings)
        : slot_(slot), host_(host), strings_(strings) {}
    ~SlotCleanup() { if (active_) destroy_voice_slot_005b7fc0(slot_, host_, strings_); }
    void dismiss() noexcept { active_ = false; }
private:
    VoicePlaybackSlot& slot_;
    VoiceLineHost& host_;
    NativeStringStorage& strings_;
    bool active_{true};
};

void create_into_slot(void*& field, void* resource, VoiceLineHost& lifetime,
    VoiceSlotStartContext& context)
{
    ScopedText normal(context.strings, "Normal");
    auto& owner = context.host.current_sound_owner_00f8bbd8(); // captured before lookup
    const auto type = find_sound_type_00a7b0a0(owner.configuration, normal.value, context.host);
    auto* sound = create_voice_sound_00a7e490(owner, resource,
        context.globals.warnings_class_00e19ac4, type, true, lifetime, context.host);
    OwnedReference temporary(sound, lifetime);
    assign_voice_reference_0054d4c0(field, sound, lifetime);
    // Temporary sound reference is released before Normal string storage.
}
} // namespace

std::int32_t find_sound_type_00a7b0a0(const SoundConfigurationState& state,
    const NativeString& name, SoundLevelNameHost& names)
{
    std::int32_t index = 0;
    for (const auto& type : state.types_38) {
        if (type.name.size() == name.length()
            && (type.name.empty() || names.compare_class_name_case_insensitive(
                type.name.c_str(), name.data()) == 0)) return index;
        ++index;
    }
    return 0; // native miss is zero, unlike the class lookup's -1
}

void assign_voice_reference_0054d4c0(void*& destination, void* source, VoiceLineHost& host)
{
    void* old = destination;
    if (old == source) return;
    destination = source; // publish before either reference operation
    if (source) host.retain_reference(source);
    if (old) host.release_reference(old);
}

void reserve_voice_sound_entries_00a7c080(SoundSystemOwner& owner,
    std::int32_t requested, VoiceLineHost& host)
{
    requested = std::max(requested, 1);
    if (requested <= owner.entry_capacity_94) return;
    auto& entries = owner.levels.entries_8c;
    std::vector<SoundLevelEntry*> replacement;
    replacement.reserve(static_cast<std::size_t>(requested));
    for (auto* entry : entries) {
        replacement.push_back(entry);
        if (entry) host.retain_reference(entry);
    }
    // All replacement references exist before any old slot is released.
    for (auto& entry : entries) {
        if (entry) { host.release_reference(entry); entry = nullptr; }
    }
    entries.swap(replacement);
    owner.entry_capacity_94 = requested; // tail hidden by _free no-return
}

void append_voice_sound_entry_00a7d5c0(SoundSystemOwner& owner,
    SoundLevelEntry* entry, VoiceLineHost& host)
{
    auto& entries = owner.levels.entries_8c;
    if (entries.size() == static_cast<std::size_t>(owner.entry_capacity_94))
        reserve_voice_sound_entries_00a7c080(owner,
            std::max(owner.entry_capacity_94 * 2, 1), host);
    // Retain has no game callback: native stores, InterlockedIncrement, count++.
    if (entry) host.retain_reference(entry);
    entries.push_back(entry);
}

SoundLevelEntry* create_voice_sound_00a7e490(SoundSystemOwner& owner, void* resource,
    std::int32_t class_index, std::int32_t type_index, bool flag,
    VoiceLineHost& lifetime, VoiceSlotStartHost& host)
{
    SoundLevelEntry* sound;
    if (host.resource_kind_08(resource) == 0)
        sound = host.create_sound_vslot_0c(owner, resource, class_index, type_index, flag);
    else if (host.resource_kind_08(resource) == 1)
        sound = host.create_sound_vslot_10(owner, resource, class_index, type_index, flag);
    else return nullptr;
    OwnedReference temporary(sound, lifetime);
    if (sound) append_voice_sound_entry_00a7d5c0(owner, sound, lifetime);
    if (sound) lifetime.retain_reference(sound); // separate hidden-return reference
    return sound;
}

void set_voice_sound_start_00a798c0(VoiceSoundStartFields& sound, float duration) noexcept
{
    const bool positive = duration > 0.0f; // native JBE includes unordered
    sound.duration_3c = positive ? duration : 0.0f;
    sound.enabled_38 = positive ? 1 : 0;
}

VoicePlaybackSlot& construct_voice_slot_00702cc0(VoicePlaybackSlot& slot,
    const VoiceClip& clip, void* bank, VoiceLineHost& lifetime, VoiceSlotStartContext& context)
{
    OwnedReference argument(bank, lifetime);
    // Fresh object precondition; resetting a live NativeString would leak.
    slot.sound_04 = nullptr;
    slot.auxiliary_08 = nullptr;
    SlotCleanup unwind(slot, lifetime, context.strings);
    if ((context.globals.guard_00e19ac8 & 1u) == 0) {
        context.globals.guard_00e19ac8 |= 1u;
        ScopedText warnings(context.strings, "Warnings");
        auto& owner = context.host.current_sound_owner_00f8bbd8();
        context.globals.warnings_class_00e19ac4 =
            find_sound_class_00a7acf0(owner.levels, warnings.value, context.host);
    }
    slot.started_at_14 = lifetime.mission_clock_00f876a4();
    slot.state_00 = 0;
    const auto& record = *clip.record_04; // native keeps this pointer across calls
    if (record.sound_id_08 >= 0) {
        lifetime.set_sound_flag_00a7d120(1);
        if (bank) create_into_slot(slot.auxiliary_08, bank, lifetime, context);
        if (record.alternate_14) {
            bool has_name;
            { // 00449AF0 against literal empty string reduces to length!=0
                ScopedText empty(context.strings, "");
                has_name = record.alternate_name_18.length() != 0;
            }
            if (has_name) {
                slot.alternate_name_0c.copy_from_00be0a30_fragment(context.strings,
                    record.alternate_name_18);
                context.host.play_alternate_00a78ce0(slot.alternate_name_0c, 0.1f);
                slot.state_00 = 2;
            }
        } else {
            // Valid array index is a native precondition; no bounds fallback.
            void* resource = record.resources_20[clip.word_08];
            if (resource) lifetime.retain_reference(resource);
            OwnedReference resource_argument(resource, lifetime);
            if (resource) {
                create_into_slot(slot.sound_04, resource, lifetime, context);
                // A null factory result is not converted to a success stub:
                // native calls A798C0 with it and faults; the host must do so.
                set_voice_sound_start_00a798c0(context.host.sound_start_fields(slot.sound_04), 0.1f);
                slot.state_00 = 1;
            }
        }
    }
    unwind.dismiss();
    return slot;
}

VoicePlaybackSlot& assign_voice_slot_005b82c0(VoicePlaybackSlot& destination,
    const VoicePlaybackSlot& source, VoiceLineHost& host, NativeStringStorage& strings)
{
    destination.state_00 = source.state_00;
    assign_voice_reference_0054d4c0(destination.sound_04, source.sound_04, host);
    assign_voice_reference_0054d4c0(destination.auxiliary_08, source.auxiliary_08, host);
    destination.alternate_name_0c.copy_from_00be0a30_fragment(strings, source.alternate_name_0c);
    destination.started_at_14 = source.started_at_14;
    return destination;
}

void destroy_voice_slot_005b7fc0(VoicePlaybackSlot& slot, VoiceLineHost& host,
    NativeStringStorage& strings) noexcept
{
    destroy_native_string_header_0041dd20(&slot.alternate_name_0c, strings);
    if (slot.auxiliary_08) {
        host.release_reference(slot.auxiliary_08);
        slot.auxiliary_08 = nullptr;
    }
    if (slot.sound_04) {
        host.release_reference(slot.sound_04);
        slot.sound_04 = nullptr;
    }
}

void start_voice_clip_005b9050(VoicePlaybackManager& manager, std::int32_t slot_index,
    VoiceClip clip, void* bank, VoiceLineHost& lifetime, VoiceSlotStartContext& context)
{
    OwnedReference argument(bank, lifetime);
    VoicePlaybackSlot temporary;
    if (bank) lifetime.retain_reference(bank);
    construct_voice_slot_00702cc0(temporary, clip, bank, lifetime, context);
    SlotCleanup cleanup(temporary, lifetime, context.strings);
    auto& destination = slot_index == 0 ? manager.slot_08
        : context.host.resolve_nonzero_slot(manager, slot_index);
    assign_voice_slot_005b82c0(destination, temporary, lifetime, context.strings);
    destroy_voice_slot_005b7fc0(temporary, lifetime, context.strings);
    cleanup.dismiss();
    if (poll_voice_slot_007027b0(destination, lifetime) == 0)
        context.host.log_sound_004254b0("Message sound NOT FOUND for: %s (%d)",
            clip.record_04->text_00.data() ? clip.record_04->text_00.data() : "", clip.word_08);
    context.host.log_sound_004254b0("Message sound played for: %s (%d)",
        clip.record_04->text_00.data() ? clip.record_04->text_00.data() : "", clip.word_08);
}

} // namespace bsp
