#include "bsp/sound_event_instance.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t n) noexcept {
    T v; std::memcpy(&v, static_cast<const unsigned char*>(p) + n, sizeof v); return v;
}
template<class T> void write(void* p, std::size_t n, T v) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + n, &v, sizeof v);
}
void* at(void* p, std::uint32_t n) noexcept { return static_cast<unsigned char*>(p) + n; }
float difference(float a, float b) noexcept {
    float v;
    __asm { fld a }
    __asm { fsub b }
    __asm { fstp v }
    return v;
}
float product(float a, float b) noexcept {
    float v;
    __asm { fld a }
    __asm { fmul b }
    __asm { fstp v }
    return v;
}
void memory_result(FmodResult result, SoundEventInstanceContext& c) {
    if (static_cast<std::uint32_t>(result) == 0x2b) {
        std::int32_t current, maximum;
        c.instance.fmod.memory_get_stats(&current, &maximum);
    }
}
const std::uint64_t normalization_threshold_bits = 0x3f1a36e2e0000000ULL; // D7A268 double
}
void reserve_sound_event_parameters_00a893d0(SoundEventParameterArray& a, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (requested <= a.capacity) return;
    const auto bytes = static_cast<std::uint32_t>(requested) * 12u;
    if (static_cast<std::uint32_t>(requested) > UINT32_MAX / 12u) throw std::bad_alloc();
    void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    for (std::int32_t i = 0; i < a.count; ++i) {
        const auto offset = static_cast<std::uint32_t>(i) * 12u;
        for (std::uint32_t word = 0; word < 12; word += 4)
            write(allocation, offset + word, read<std::uint32_t>(a.data, offset + word));
    }
    singleton_lifetime_free(a.data); a.data = allocation; a.capacity = requested;
}
void resize_sound_event_parameters_00a899e0(SoundEventParameterArray& a, std::int32_t requested) {
    if (requested < 0) throw std::out_of_range("Event parameter count");
    if (requested > a.capacity) reserve_sound_event_parameters_00a893d0(a, requested);
    for (auto i = a.count; i < requested; ++i) {
        const auto offset = static_cast<std::uint32_t>(i) * 12u;
        write<std::uint8_t>(a.data, offset + 4, 0); write<void*>(a.data, offset + 8, nullptr);
    }
    while (requested < a.count) --a.count;
    a.count = requested;
}
void destroy_sound_event_parameters_00a89be0(SoundEventParameterArray& a) {
    resize_sound_event_parameters_00a899e0(a, 0); singleton_lifetime_free(a.data);
    // Native header data/capacity survive disposal; there is no second owner.
}
void set_sound_event_parameter_00a89b80(SpatialSoundEventInstance& s, std::int32_t index, float value) {
    if (index < 0 || index == INT32_MAX) throw std::out_of_range("Event parameter index");
    if (s.parameters_58.count <= index) {
        s.parameters_dirty_64 = 1; resize_sound_event_parameters_00a899e0(s.parameters_58, index + 1);
    }
    void* const record = at(s.parameters_58.data, static_cast<std::uint32_t>(index) * 12u);
    // Native new-slot value is NOT initialized by resize; initialized backing
    // value bytes are required for this comparison's defined-input domain.
    if (read<float>(record, 0) != value) {
        write<std::uint8_t>(record, 4, 1); write(record, 0, value); s.parameters_dirty_64 = 1;
    }
}
void* find_sound_event_parameter_00a896e0(const SpatialSoundEventInstance& s, void* output,
    const char* name, SoundLevelNameHost& names) {
    void* const options = sound_sample_options_00a81860(s.sample_4c);
    std::int32_t selected = 0;
    for (std::int32_t i = 0; i < read<std::int32_t>(options, 0x40); ++i) {
        const auto offset = static_cast<std::uint32_t>(i) * 16u;
        if (names.compare_class_name_case_insensitive(read<const char*>(read<void*>(options, 0x3c), offset + 12), name) == 0) {
            selected = i; break;
        }
    }
    void* const data = read<void*>(options, 0x3c);
    if (!data) throw std::logic_error("Native event parameter lookup requires a first record");
    void* const record = at(data, static_cast<std::uint32_t>(selected) * 16u);
    const auto index = read<std::uint32_t>(record, 0), minimum = read<std::uint32_t>(record, 4);
    write(output, 0, index); const auto maximum = read<std::uint32_t>(record, 8);
    write(output, 4, minimum); const auto label = read<std::uint32_t>(record, 12);
    write(output, 8, maximum); write(output, 12, label); return output;
}
SpatialSoundEventInstance& construct_spatial_sound_event_00a89dc0(SpatialSoundEventInstance& s,
    void* sample, SoundClassLevel* const* cls, std::uint32_t type, std::uint8_t flag,
    std::array<float, 3> position, std::array<float, 3> velocity, SoundInstanceContext& c) {
    construct_sound_instance_00a7c480(s, sample, cls, type, flag, c);
    s.native_vtable_00 = 0x00d5b480; s.parameters_58 = {}; s.parameters_dirty_64 = 0;
    s.channel_group_68 = nullptr; s.event_54 = nullptr; s.velocity_78 = {}; s.position_6c = {};
    s.distance_band_88 = 0; s.ended_8c = 0; s.native_vtable_00 = 0x00d5b4c8;
    s.velocity_78 = velocity; s.position_6c = position; s.spatial_dirty_84 = 1; return s;
}
void destroy_sound_event_00a89c80(SpatialSoundEventInstance& s, SoundEventInstanceContext& c) {
    s.native_vtable_00 = 0x00d5b480;
    try { if (s.event_54) c.fmod.event_stop(s.event_54, false); }
    catch (...) {
        // DEC50C states1->0: parameter array, then base instance.
        try { destroy_sound_event_parameters_00a89be0(s.parameters_58); }
        catch (...) { destroy_sound_instance_00a7bd90(s, c.instance); throw; }
        destroy_sound_instance_00a7bd90(s, c.instance); throw;
    }
    try { destroy_sound_event_parameters_00a89be0(s.parameters_58); }
    catch (...) { destroy_sound_instance_00a7bd90(s, c.instance); throw; }
    destroy_sound_instance_00a7bd90(s, c.instance);
}
SpatialSoundEventInstance* scalar_delete_spatial_sound_event_00a89ee0(
    SpatialSoundEventInstance* s, std::uint32_t flags, SoundEventInstanceContext& c) {
    destroy_sound_event_00a89c80(*s, c); if (flags & 1u) delete s; return s;
}
void create_sound_event_00a894e0(SpatialSoundEventInstance& s, SoundEventInstanceContext& c) {
    void* const system = c.instance.current_owner_00f8bbd8->system.event_system;
    const char* const name = read<const char*>(s.sample_4c, 0x64);
    memory_result(c.fmod.event_system_get_event(system, name ? name : c.null_data_00f8bbef, 2, &s.event_54), c);
    memory_result(c.fmod.event_start(s.event_54), c);
    s.channel_group_68 = c.instance.current_owner_00f8bbd8->configuration.master_channel_group_140;
}
void update_sound_event_parameters_00a89560(SpatialSoundEventInstance& s, SoundEventInstanceContext& c) {
    if (!s.parameters_dirty_64) return;
    s.parameters_dirty_64 = 0;
    for (std::int32_t index = 0; index < s.parameters_58.count; ++index) {
        const auto offset = static_cast<std::uint32_t>(index) * 12u;
        if (!read<std::uint8_t>(s.parameters_58.data, offset + 4)) continue;
        write<std::uint8_t>(s.parameters_58.data, offset + 4, 0);
        if (!read<void*>(s.parameters_58.data, offset + 8))
            memory_result(c.fmod.event_get_parameter_by_index(s.event_54, index,
                static_cast<void**>(at(s.parameters_58.data, offset + 8))), c);
        // Current array fields are reloaded after FMOD and memory callbacks.
        const float value = read<float>(s.parameters_58.data, offset);
        memory_result(c.fmod.event_parameter_set_value(read<void*>(s.parameters_58.data, offset + 8), value), c);
    }
}
void stop_sound_event_00a89170(SpatialSoundEventInstance& s, std::uint8_t immediate, SoundEventInstanceContext& c) {
    void* const event = s.event_54; s.stop_requested_51 = 1;
    if (!event) { s.stopped_15 = 1; return; }
    void* stop_parameter{};
    c.fmod.event_get_parameter(event, "Stop", &stop_parameter);
    if (immediate || stop_parameter) { c.fmod.event_stop(s.event_54, true); return; }
    void* loop_parameter{};
    c.fmod.event_get_parameter(s.event_54, "Loop", &loop_parameter);
    if (loop_parameter) c.fmod.event_parameter_key_off(loop_parameter);
}
bool refresh_spatial_sound_event_ended_00a89340(SpatialSoundEventInstance& s, SoundEventInstanceContext& c) {
    if (s.stopped_15) { s.ended_8c = 1; return true; }
    if (!s.event_54) { s.ended_8c = 0; return false; }
    std::uint32_t state = 0; c.fmod.event_get_state(s.event_54, &state);
    s.ended_8c = (state & 8u) == 0; return s.ended_8c != 0;
}
void SpatialSoundEventVirtuals::refresh_slot10(SpatialSoundEventInstance& s, SoundEventInstanceContext& c) {
    if (s.native_vtable_00 != 0x00d5b4c8) throw std::logic_error("Unbound event instance profile");
    refresh_spatial_sound_event_ended_00a89340(s, c);
}
bool spatial_sound_event_completed_00a88ba0(SpatialSoundEventInstance& s, SoundEventInstanceContext& c) {
    if (s.stop_requested_51) c.virtuals.refresh_slot10(s, c);
    if (s.event_54) return s.stopped_15 || s.ended_8c;
    return s.stop_requested_51 != 0;
}
bool sound_event_nonvirtual_00a89c40() noexcept { return false; }
bool sound_event_virtual_transition_00a89c70() noexcept { return false; }
void set_spatial_sound_event_attributes_00a89120(SpatialSoundEventInstance& s,
    std::array<float, 3> position, std::array<float, 3> velocity) noexcept {
    s.velocity_78 = velocity; s.position_6c = position; s.spatial_dirty_84 = 1;
}
std::array<float, 3>& get_spatial_sound_event_position_00a89e80(
    const SpatialSoundEventInstance& s, std::array<float, 3>& output) noexcept {
    const auto position = s.position_6c; output = position; return output;
}
__declspec(naked) float* __fastcall normalize_sound_event_velocity_00a88ff0(
    const float*, const CameraAxesCrtAccess*, float*) {
    __asm {
        push ebx
        mov ebx, edx
        sub esp, 8
        push esi
        mov esi, ecx
        fld dword ptr [esi + 4]
        fld dword ptr [esi]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [esi + 8]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp dword ptr [esp + 8]
        fld dword ptr [esp + 8]
        mov ecx, ebx
        call native_crt_sqrt_st0_00bf7030
        fstp dword ptr [esp + 8]
        fld dword ptr [esp + 8]
        mov eax, dword ptr [esp + 0x14]
        fstp dword ptr [esp + 8]
        fld dword ptr [esp + 8]
        fld qword ptr normalization_threshold_bits
        fcomip st(0), st(1)
        jbe divide_components
        xorps xmm0, xmm0
        fstp st(0)
        movss dword ptr [eax + 8], xmm0
        movss dword ptr [eax + 4], xmm0
        movss dword ptr [eax], xmm0
        jmp finished
    divide_components:
        fld dword ptr [esp + 4]
        fdiv st(0), st(1)
        fstp dword ptr [eax]
        fld dword ptr [esi + 4]
        fdiv st(0), st(1)
        fstp dword ptr [eax + 4]
        fdivr dword ptr [esi + 8]
        fstp dword ptr [eax + 8]
    finished:
        pop esi
        add esp, 8
        pop ebx
        ret 4
    }
}
void update_spatial_sound_event_00a89770(SpatialSoundEventInstance& s, float dt,
    const SoundListenerOwnerState& listener, SoundEventInstanceContext& c) {
    if (s.stopped_15) return;
    auto& base = c.instance; advance_sound_instance_fade_00a7a2e0(s, dt);
    if (s.duration_3c > 0) {
        dt = difference(s.duration_3c, dt); s.duration_3c = dt;
        if (dt <= 0) s.enabled_38 = 0;
    }
    if (!s.event_54) create_sound_event_00a894e0(s, c);
    ++s.updates_18;
    const auto position = s.position_6c;
    const std::array<float, 3> relative{difference(listener.transform_c4[12], position[0]),
        difference(listener.transform_c4[13], position[1]), difference(listener.transform_c4[14], position[2])};
    const float distance = camera_vector_length_00419440(relative.data(), &c.crt);
    const auto band = sound_distance_band_00a79a40(base.current_owner_00f8bbd8->configuration.scalars, distance);
    if (band != s.distance_band_88) {
        s.distance_band_88 = band;
        s.type_48 = static_cast<std::uint32_t>(sound_distance_type_00a7ec60(base.current_owner_00f8bbd8->configuration,
            static_cast<std::int32_t>(s.type_48), band, base.strings, c.names));
    }
    void* const group = sound_type_listener_group_00a7ba40(base.current_owner_00f8bbd8->configuration, static_cast<std::int32_t>(s.type_48));
    if (group != s.channel_group_68) {
        if (group) {
            // Native reuses the dt stack slot as the output without clearing it.
            // The slot contains remaining delay if the countdown ran above.
            void* event_group; static_assert(sizeof event_group == sizeof dt);
            std::memcpy(&event_group, &dt, sizeof dt);
            memory_result(c.fmod.event_get_channel_group(s.event_54, &event_group), c);
            c.fmod.channel_group_add_group(group, event_group);
            s.channel_group_68 = group;
            if (s.scale_28 != 1) { s.dirty_14 = 1; s.scale_28 = 1; }
        } else if (s.scale_28 != 0) { s.dirty_14 = 1; s.scale_28 = 0; }
        s.channel_group_68 = group;
    }
    if (s.dirty_14) {
        c.fmod.event_set_pitch(s.event_54, difference(s.frequency_scale_20, 1.0f), 0);
        float volume = sound_instance_volume_00a7aaa0(s, base);
        if (s.pan_level_40 != 1) volume = product(base.current_owner_00f8bbd8->configuration.scalars.player_sound_boost_04, volume);
        c.fmod.event_set_volume(s.event_54, volume); s.dirty_14 = 0;
    }
    if (s.spatial_dirty_84) {
        std::array<float, 3> orientation;
        normalize_sound_event_velocity_00a88ff0(s.velocity_78.data(), &c.crt, orientation.data());
        const auto velocity = s.velocity_78, current_position = s.position_6c;
        c.fmod.event_set_3d_attributes(s.event_54, current_position, velocity, orientation);
        s.spatial_dirty_84 = 0;
    }
    update_sound_event_parameters_00a89560(s, c);
    if (!s.paused_1c && !s.class_44) throw std::logic_error("Spatial event requires a live class descriptor");
    const bool paused = s.paused_1c || static_cast<SoundClassDescriptor*>(s.class_44)->flags_0c;
    c.fmod.event_set_paused(s.event_54, static_cast<std::uint8_t>(s.enabled_38 | static_cast<std::uint8_t>(paused)));
}
SoundInstance* create_spatial_sound_00a7f710(SoundSystemOwner& owner, void* sample,
    std::int32_t cls, std::uint32_t type, std::uint8_t flag, SoundInstanceContext& c) {
    ++owner.words_160[0];
    if (sound_sample_has_fmod_sound_00a818c0(sample))
        return allocate_spatial_bank_sound_fragment(owner, sample, cls, type, flag, c);
    auto* s = new(std::nothrow) SpatialSoundEventInstance;
    if (!s) return nullptr;
    try {
        if (cls < 0 || cls == INT32_MAX) throw std::out_of_range("Sound class index");
        if (static_cast<std::size_t>(cls) >= owner.levels.classes_98.size()) owner.classes.resize_00a7c2c0(cls + 1);
        construct_spatial_sound_event_00a89dc0(*s, sample, &owner.levels.classes_98[static_cast<std::size_t>(cls)], type, flag, {}, {}, c);
    } catch (...) { delete s; throw; } // DEB7E8 state1 frees allocation only.
    return s;
}
} // namespace bsp
