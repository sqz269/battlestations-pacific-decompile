#include "bsp/sound_retained_factory.hpp"
#include "bsp/voice_slot_start.hpp"
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
struct TemporaryReference {
    void* value{};
    VoiceReferenceHost& references;
    ~TemporaryReference() { if (value) references.release_reference(value); }
};
struct ClassName {
    NativeString value;
    NativeStringStorage& strings;
    explicit ClassName(NativeStringStorage& storage) : strings(storage) {
        value.assign_0041e870(strings, "3DEffect");
    }
    ~ClassName() { destroy_native_string_header_0041dd20(&value, strings); }
};
template<class Channel, class Constructor>
void create_retained(SoundSystemOwner& owner, const SoundRetainedSourceRecord& source,
    SoundRetainedFactoryContext& context, TemporaryReference& temporary, Constructor construct) {
    // Native allocation failure still reaches append/assignment with a null
    // temporary. C++ allocation size differs from the original 70h/74h size.
    std::unique_ptr<Channel> allocation(new(std::nothrow) Channel);
    if (!allocation) {
        adopt_sound_reference_0054d510(temporary.value, nullptr, context.channels);
        return;
    }
    ClassName name(context.instance.strings);
    auto& current = *context.instance.current_owner_00f8bbd8;
    const auto index = find_sound_class_00a7acf0(current.levels, name.value, context.names);
    auto* slot = sound_class_slot_00a7f0f0(owner, index);
    construct(*allocation, source, slot, context.instance, context.names);
    auto* entry = static_cast<SoundLevelEntry*>(allocation.release());
    adopt_sound_reference_0054d510(temporary.value, entry, context.channels);
    // The temporary owns the new reference before class-name destruction.
}
} // namespace

SoundClassLevel* const* sound_class_slot_00a7f0f0(SoundSystemOwner& owner, std::int32_t index) {
    if (index < 0 || index == (std::numeric_limits<std::int32_t>::max)())
        throw std::out_of_range("Sound class index outside reconstructed domain");
    if (static_cast<std::size_t>(index) >= owner.levels.classes_98.size())
        owner.classes.resize_00a7c2c0(index + 1);
    return &owner.levels.classes_98[static_cast<std::size_t>(index)];
}

void*& adopt_sound_reference_0054d510(void*& destination, void* source, VoiceReferenceHost& references) {
    auto& adopted_slot = adopt_sound_reference_0054d510(
        static_cast<void* volatile&>(destination), source, references);
    // Discarding a volatile glvalue would read the slot again. Take its address.
    (void)&adopted_slot;
    return destination;
}

void* volatile& adopt_sound_reference_0054d510(void* volatile& destination, void* source, VoiceReferenceHost& references) {
    if (auto* old = destination) {
        references.release_reference(old);
        destination = nullptr;
    }
    destination = source;
    return destination;
}

void set_retained_sound_00a7f2f0(SoundSystemOwner& owner, const SoundRetainedSourceRecord& source,
    std::int32_t index, SoundRetainedFactoryContext& context) {
    if (!owner.system.sound_enabled) return;
    // Native accesses the arrays before its switch; negative/out-of-range
    // callers have no valid native array element. Do not map them to slot0.
    if (index < 0 || index >= 3) throw std::out_of_range("Retained sound slot must be 0, 1 or 2");
    auto& channel_slot = owner.pointers_74[static_cast<std::size_t>(index)];
    auto& sample_slot = owner.pointers_80[static_cast<std::size_t>(index)];
    if (sample_slot == source.sample_00) return;
    if (auto* old = channel_slot)
        context.channels.stop_slot08(static_cast<SoundLevelEntry*>(old), 0);
    TemporaryReference temporary{nullptr, context.channels};
    switch (index) {
    case 0:
        create_retained<RetainedSoundChannelInstance>(owner, source, context, temporary,
            construct_retained_sound_channel_00a7da40);
        break;
    case 1:
        create_retained<ScaledRetainedSoundChannelInstance>(owner, source, context, temporary,
            construct_scaled_retained_sound_channel_00a7db80);
        break;
    case 2:
        create_retained<RetainedSoundChannelInstance>(owner, source, context, temporary,
            construct_underwater_retained_sound_channel_00a7dcd0);
        break;
    }
    append_voice_sound_entry_00a7d5c0(owner, static_cast<SoundLevelEntry*>(temporary.value), context.channels);
    assign_voice_reference_0054d4c0(channel_slot, temporary.value, context.channels);
    assign_sound_sample_reference_004e7bb0(&sample_slot, &source.sample_00, context.instance.sample_lifetime);
}
} // namespace bsp
