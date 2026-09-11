#include "bsp/sound_gameplay_methods.hpp"
#include "bsp/sound_sample.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t offset) noexcept {
    T value; std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, sizeof value); return value;
}
template<class T> void write(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + offset, &value, sizeof value);
}
void* offset(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
void copy_float(void* destination, const void* source, std::size_t n) noexcept {
    const float value = read<float>(source, n); float converted;
    __asm { fld value }
    __asm { fstp converted }
    write(destination, n, converted);
}
class ManagerSection {
public:
    explicit ManagerSection(SingletonLifetimeDomain& domain)
        : section_(domain.get_manager_00415350()->system_owner().section_10) {
        if (section_) { singleton_enter_critical_section(*section_); ++section_->recursion_18; }
    }
    ~ManagerSection() {
        if (section_) { --section_->recursion_18; singleton_leave_critical_section(*section_); }
    }
private:
    SystemSingletonCriticalSection* section_;
};
class QuerySection {
public:
    explicit QuerySection(TrackedCriticalSection* section) : section_(section) {
        if (section_) { EnterCriticalSection(&section_->native); ++section_->depth; }
    }
    ~QuerySection() {
        if (section_) { --section_->depth; LeaveCriticalSection(&section_->native); }
    }
private:
    TrackedCriticalSection* section_;
};
void memory_result(FmodResult result, SoundInstanceContext& context) {
    if (static_cast<std::uint32_t>(result) == 0x2b) {
        std::int32_t current, maximum; context.fmod.memory_get_stats(&current, &maximum);
    }
}
struct Options {
    alignas(4) unsigned char bytes[0x48];
    explicit Options(const void* source) { copy_construct_sound_sample_options_0093fdf0(bytes, source); }
    ~Options() {
        destroy_sound_sample_options_0093fdb0(bytes);
    }
};
}
SoundEventQueryLock& construct_sound_event_query_lock_00a89460(
    SoundEventQueryLock& object, SoundEventQueryLockBindings& bindings) {
    object.native_vtable_00 = 0x00d5b478;
    try { object.section_04 = critical_section_create_00bd1860(); }
    catch (...) { destroy_sound_event_query_lock_base_00a89390(object, bindings); throw; }
    return object;
}
SoundEventQueryLock* get_sound_event_query_lock_00a89a40(SoundEventQueryLockBindings& bindings) {
    if (!bindings.global_00f8bbdc) {
        ManagerSection lock(bindings.domain);
        if (!bindings.global_00f8bbdc) {
            auto* allocated = new(std::nothrow) SoundEventQueryLock;
            try { if (allocated) construct_sound_event_query_lock_00a89460(*allocated, bindings); }
            catch (...) { delete allocated; throw; }
            bindings.global_00f8bbdc = allocated;
            bindings.domain.get_manager_00415350()->register_object(bindings.global_00f8bbdc);
        }
    }
    return bindings.global_00f8bbdc;
}
SoundEventQueryLock* scalar_delete_sound_event_query_lock_00a89b40(
    SoundEventQueryLock* object, std::uint32_t flags, SoundEventQueryLockBindings& bindings) {
    object->native_vtable_00 = 0x00d5b478;
    critical_section_destroy_owned_0041cc80(object->section_04);
    destroy_sound_event_query_lock_base_00a89390(*object, bindings);
    if (flags & 1u) delete object;
    return object;
}
void destroy_sound_event_query_lock_base_00a89390(SoundEventQueryLock& object,
    SoundEventQueryLockBindings& bindings) noexcept {
    bindings.global_00f8bbdc = nullptr;
    object.native_vtable_00 = 0x00ce3818;
}
void destroy_sound_sample_options_0093fdb0(void* options) {
    auto* header = static_cast<unsigned char*>(options) + 0x3c;
    resize_sound_sample_parameters_0093f950(header, 0);
    singleton_lifetime_free(read<void*>(header, 0));
}
void* assign_sound_sample_parameters_0093fd00(void* destination, const void* source) {
    resize_sound_sample_parameters_0093f950(destination, 0);
    reserve_sound_sample_parameters_0093f6e0(destination, read<std::int32_t>(source, 4));
    for (std::int32_t i = 0; i < read<std::int32_t>(source, 4); ++i) {
        const auto* captured = static_cast<const unsigned char*>(read<void*>(source, 0)) +
            static_cast<std::uint32_t>(i) * 16u;
        const auto capacity = read<std::int32_t>(destination, 8);
        if (read<std::int32_t>(destination, 4) == capacity) {
            if (capacity > INT32_MAX / 2) throw std::out_of_range("Sound parameter capacity");
            reserve_sound_sample_parameters_0093f6e0(destination, capacity > 0 ? capacity * 2 : 1);
        }
        if (void* record = offset(read<void*>(destination, 0),
            static_cast<std::uint32_t>(read<std::int32_t>(destination, 4)) * 16u)) {
            for (std::size_t word = 0; word != 16; word += 4)
                write(record, word, read<std::uint32_t>(captured, word));
        }
        write(destination, 4, read<std::int32_t>(destination, 4) + 1);
    }
    return destination;
}
void* copy_construct_sound_sample_options_0093fdf0(void* destination, const void* source) {
    write(destination, 0, read<std::uint32_t>(source, 0));
    for (std::size_t n = 4; n <= 0x10; n += 4) copy_float(destination, source, n);
    for (std::size_t n = 0x14; n <= 0x16; ++n) write(destination, n, read<std::uint8_t>(source, n));
    copy_float(destination, source, 0x18); copy_float(destination, source, 0x1c);
    for (std::size_t n = 0x20; n <= 0x22; ++n) write(destination, n, read<std::uint8_t>(source, n));
    write(destination, 0x24, read<std::uint32_t>(source, 0x24));
    write(destination, 0x28, read<std::uint32_t>(source, 0x28));
    for (std::size_t n = 0x2c; n <= 0x34; n += 4) copy_float(destination, source, n);
    write(destination, 0x38, read<std::uint8_t>(source, 0x38));
    for (std::size_t n = 0x3c; n != 0x48; n += 4) write<std::uint32_t>(destination, n, 0);
    assign_sound_sample_parameters_0093fd00(static_cast<unsigned char*>(destination) + 0x3c,
        static_cast<const unsigned char*>(source) + 0x3c);
    return destination;
}
void configure_spatial_sound_channel_00a8a700(SpatialSoundChannelInstance& sound,
    SoundInstanceContext& context, SoundGameplayFmodHost& fmod) {
    // The native EH cleanup is activated only after copy construction succeeds.
    Options options(sound_sample_options_00a81860(sound.sample_4c));
    const auto* storage = options.bytes;
    memory_result(fmod.channel_set_3d_minmax_distance(sound.channel_54,
        read<float>(storage, 0xc), read<float>(storage, 0x10)), context);
    std::optional<std::uint32_t> mode;
    memory_result(fmod.channel_get_mode(sound.channel_54, mode), context);
    if (!mode) throw std::runtime_error("FMOD left native channel mode indeterminate");
    auto value = read<std::uint8_t>(storage, 0x20) ? (*mode & ~0x100000u) | 0x200000u :
        (*mode & ~0x200000u) | 0x100000u;
    value = read<std::uint8_t>(storage, 0x14) ? (value & ~0x200u) | 0x100u : (value & ~0x100u) | 0x200u;
    memory_result(fmod.channel_set_mode(sound.channel_54, value), context);
}
void sound_instance_noop_00a7be90() noexcept {}
void* sound_event_handle_00a88b90(const SpatialSoundEventInstance& sound) noexcept { return sound.event_54; }
void* sound_channel_handle_00a79a30(const SoundChannelInstance& sound) noexcept { return sound.channel_54; }
float sound_event_progress_00a89c60() noexcept {
    const std::uint32_t bits = 0x3eaa7efa; float value; std::memcpy(&value, &bits, sizeof value); return value;
}
float sound_channel_progress_00a7a6b0(SoundChannelInstance& sound, SoundGameplayFmodHost& fmod) {
    std::uint32_t position = 0;
    if (!sound.channel_54) return 0;
    fmod.channel_get_position(sound.channel_54, &position, 2);
    std::int32_t signed_position; std::memcpy(&signed_position, &position, sizeof position);
    const float unsigned_bias = 4294967296.0f; double numerator;
    __asm { fild signed_position }
    if (signed_position < 0) { __asm { fadd unsigned_bias } }
    __asm { fstp numerator }
    const auto bits = sound_sample_pcm_length_00a81870(sound.sample_4c);
    std::int32_t denominator; std::memcpy(&denominator, &bits, sizeof denominator); float result;
    __asm { fild denominator }
    __asm { fdivr numerator }
    __asm { fstp result }
    return result;
}
float sound_channel_audibility_00a7a710(SoundChannelInstance& sound, SoundInstanceContext& context) {
    float value = 1; context.fmod.channel_get_audibility(sound.channel_54, &value); return value;
}
float sound_group_max_audibility_00a89250(void* group,
    SoundInstanceContext& context, SoundGameplayFmodHost& fmod) {
    float maximum = 0; std::int32_t count = 0;
    std::optional<void*> handle; // Same uninitialized native output slot reused by both loops.
    fmod.channel_group_get_num_groups(group, &count);
    while (count > 0) {
        --count; fmod.channel_group_get_group(group, count, handle);
        if (!handle) throw std::runtime_error("FMOD left native child group indeterminate");
        const float candidate = sound_group_max_audibility_00a89250(*handle, context, fmod);
        if (!(maximum > candidate)) maximum = candidate; // JA preserves max only for ordered greater.
    }
    count = 0; fmod.channel_group_get_num_channels(group, &count);
    while (count > 0) {
        --count; float candidate = 0;
        fmod.channel_group_get_channel(group, count, handle);
        if (!handle) throw std::runtime_error("FMOD left native child channel indeterminate");
        context.fmod.channel_get_audibility(*handle, &candidate);
        if (!(maximum > candidate)) maximum = candidate;
    }
    return maximum;
}
float sound_event_audibility_00a89d00(SpatialSoundEventInstance& sound,
    SoundEventInstanceContext& context, SoundGameplayFmodHost& fmod, SoundEventQueryLockBindings& bindings) {
    if (!sound.event_54) return 0;
    auto* owner = get_sound_event_query_lock_00a89a40(bindings);
    if (!owner) throw std::runtime_error("Native event query lock allocation returned null");
    QuerySection lock(owner->section_04);
    void* group = nullptr; context.fmod.event_get_channel_group(sound.event_54, &group);
    return group ? sound_group_max_audibility_00a89250(group, context.instance, fmod) : 0.0f;
}
void set_sound_channel_paused_00a799a0(SoundChannelInstance& sound, std::uint8_t paused) noexcept {
    sound.paused_1c = paused;
}
void set_sound_event_paused_00a891f0(SpatialSoundEventInstance& sound, std::uint8_t paused,
    SoundEventInstanceContext& context) {
    if (sound.event_54) context.fmod.event_set_paused(sound.event_54, paused);
}
} // namespace bsp
