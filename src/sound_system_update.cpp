#include "bsp/sound_system_update.hpp"
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
float absolute_bits(float value) noexcept {
    std::uint32_t bits; std::memcpy(&bits, &value, 4); bits &= 0x7fffffffu;
    std::memcpy(&value, &bits, 4); return value;
}
bool keep_velocity(const std::array<float, 3>& input, const std::array<float, 3>& prior, float dt) noexcept {
    const float x = absolute_bits(input[0]), y = absolute_bits(input[1]), z = absolute_bits(input[2]);
    const float px = absolute_bits(prior[0]), py = absolute_bits(prior[1]), pz = absolute_bits(prior[2]);
    const double fifty = 50.0, six_hundred = 600.0;
    float temporary;
    unsigned char first, second;
    // Preserve the unusual subtraction/addition order, two float stores,
    // double constants and CF branches (unordered passes both comparisons).
    __asm { fld y }
    __asm { fadd x }
    __asm { fadd z }
    __asm { fstp temporary }
    __asm { fld temporary }
    __asm { fsub px }
    __asm { fadd py }
    __asm { fadd pz }
    __asm { fstp temporary }
    temporary = absolute_bits(temporary);
    __asm { fld temporary }
    __asm { fld dt }
    __asm { fdivr fifty }
    __asm { fxch st(1) }
    __asm { fcomip st(0), st(1) }
    __asm { fstp st(0) }
    __asm { setb first }
    if (!first) return false;
    __asm { fld y }
    __asm { fadd x }
    __asm { fadd z }
    __asm { fld six_hundred }
    __asm { fxch st(1) }
    __asm { fcomip st(0), st(1) }
    __asm { fstp st(0) }
    __asm { setb second }
    return second != 0;
}
void round_in_place(float& value) noexcept {
    auto* p = &value;
    __asm { mov eax, p }
    __asm { fld dword ptr [eax] }
    __asm { fstp dword ptr [eax] }
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t value; std::memcpy(&value, &bits, 4); return value;
}
float elapsed_and_publish(const ClockTimestamp& elapsed, const ClockTimestamp& now,
    std::array<std::uint32_t, 4>& destination) noexcept {
    const auto* difference = &elapsed; const auto* snapshot = &now;
    auto* target = destination.data(); float seconds;
    // A7E676..A7E698: publish low timestamp word before FDIVP, then the
    // remaining words before the float spill, including floating fault order.
    __asm {
        mov eax, difference
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        mov eax, snapshot
        mov ecx, target
        mov edx, dword ptr [eax]
        mov dword ptr [ecx], edx
        fdivp st(1), st(0)
        mov edx, dword ptr [eax + 4]
        mov dword ptr [ecx + 4], edx
        mov edx, dword ptr [eax + 8]
        mov dword ptr [ecx + 8], edx
        mov edx, dword ptr [eax + 12]
        mov dword ptr [ecx + 12], edx
        fstp seconds
    }
    return seconds;
}
}
const ClockTimestamp* sound_frame_clock_current_00bee050(FrameClock& clock) noexcept { return &clock.current; }
bool sound_channel_virtual_transition_00a79990(const SoundChannelInstance& s) noexcept {
    return s.was_virtual_58 != s.virtual_59;
}
void resize_sound_entries_00a7c1c0(SoundSystemOwner& owner, std::int32_t requested, VoiceReferenceHost& lifetime) {
    if (requested < 0) throw std::out_of_range("Negative sound entry count");
    if (requested > owner.entry_capacity_94) reserve_voice_sound_entries_00a7c080(owner, requested, lifetime);
    auto& entries = owner.levels.entries_8c;
    const auto count = static_cast<std::size_t>(requested);
    if (count > entries.size()) entries.resize(count, nullptr);
    while (count < entries.size()) {
        auto* slot = &entries.back(); auto* entry = *slot;
        entries.pop_back(); // native count decreases before release
        // pop_back ends the C++ pointer object's lifetime. Native only lowers
        // the count: release callbacks may still read the captured live slot.
        new(slot) SoundLevelEntry*(entry);
        if (entry) {
            lifetime.release_reference(entry);
            *slot = nullptr;
        }
    }
    entries.resize(count, nullptr);
}
void pop_sound_entry_00a7d640(SoundSystemOwner& owner, VoiceReferenceHost& lifetime) {
    auto& entries = owner.levels.entries_8c;
    if (entries.empty()) return;
    auto* slot = &entries.back();
    if (auto* entry = *slot) { lifetime.release_reference(entry); *slot = nullptr; }
    entries.pop_back(); // native count decreases after release
}
void erase_sound_entry_swap_last_00a7d680(SoundSystemOwner& owner, std::size_t index, VoiceReferenceHost& lifetime) {
    auto& entries = owner.levels.entries_8c;
    if (index >= entries.size()) throw std::out_of_range("Sound entry iterator");
    auto* slot = entries.data() + index;
    if (slot != &entries.back()) {
        auto* replacement = entries.back(); auto* previous = *slot;
        if (replacement != previous) {
            *slot = replacement;
            if (replacement) lifetime.retain_reference(replacement);
            if (previous) lifetime.release_reference(previous);
        }
    }
    // Reload the current last slot after the replaced entry's release.
    auto* last = &entries.back();
    if (auto* entry = *last) { lifetime.release_reference(entry); *last = nullptr; }
    entries.pop_back();
}
void update_sound_system_base_00a7e630(SoundSystemOwner& owner, const CameraMatrix& matrix,
    std::array<float, 3> velocity, SoundSystemUpdateContext& c) {
    const ClockTimestamp now = *c.host.current_timestamp_slot14();
    ClockTimestamp previous, elapsed;
    static_assert(sizeof previous == sizeof owner.time_118);
    std::memcpy(&previous, owner.time_118.data(), sizeof previous);
    subtract_timestamp_00530890(elapsed, now, previous);
    const float dt = elapsed_and_publish(elapsed, now, owner.time_118);
    if (void* alternate = c.host.current_alternate_00f8bbcc()) c.host.update_alternate_slot04(alternate, dt);
    std::int32_t ignored_channels = 0;
    c.fmod.system_get_channels_playing(owner.system.system, &ignored_channels);
    if (owner.flag_69) return;

    auto& listener = owner.listener;
    if (!keep_velocity(velocity, listener.position_b8, dt)) velocity = {0,0,0};
    listener.position_b8 = velocity; // historical field name; native FMOD velocity
    copy_camera_matrix_004134f0(listener.transform_c4, matrix);
    for (std::size_t i = 8; i != 11; ++i) round_in_place(listener.transform_c4[i]);
    // Capture all listener values BEFORE callbacks. The later FMOD handles are
    // reloaded, but callbacks cannot change these already-copied arguments.
    const std::array<float, 3> position{listener.transform_c4[12],listener.transform_c4[13],listener.transform_c4[14]};
    const auto captured_velocity = listener.position_b8;
    const std::array<float, 3> forward{listener.transform_c4[8],listener.transform_c4[9],listener.transform_c4[10]};
    const std::array<float, 3> up{listener.transform_c4[4],listener.transform_c4[5],listener.transform_c4[6]};
    auto& entries = owner.levels.entries_8c;
    auto* cursor = entries.data(); auto* const end = entries.empty() ? cursor : cursor + entries.size();
    for (; cursor != end; ++cursor) {
        c.entries.update_slot30(*cursor, dt, listener);
        if (c.entries.transition_slot24(*cursor)) ++owner.words_160[2];
    }
    c.fmod.event_system_set_3d_listener_attributes(owner.system.event_system, 0,
        position, captured_velocity, forward, up);
    c.fmod.update_event_system(owner.system.event_system);

    // Keep the same iterator after swap-last erasure so the moved element is
    // refreshed and considered. Native recomputes the end after every callback.
    std::size_t index = 0;
    while (index != entries.size()) {
        c.entries.refresh_slot10(entries[index]);
        const bool completed = c.entries.completed_slot0c(entries[index]);
        const bool real = c.entries.nonvirtual_slot14(entries[index]);
        bool retire = completed;
        if (!retire && !real) {
            auto& fields = c.entries.instance_fields(entries[index]);
            retire = fields.flag_50 == 0 && signed_bits(fields.updates_18) > 5;
        }
        if (!retire) ++index;
        else {
            c.entries.stop_slot08(entries[index], 0);
            erase_sound_entry_swap_last_00a7d680(owner, index, c.entries);
            ++owner.words_160[1];
        }
    }
}
void update_sound_system_00a87bf0(SoundSystemOwner& owner, const CameraMatrix& matrix,
    std::array<float, 3> velocity, SoundSystemUpdateContext& c) {
    auto& configuration = owner.configuration;
    if (configuration.current_listener_10c != configuration.previous_listener_110)
        configuration.previous_listener_110 = configuration.current_listener_10c;
    update_sound_system_base_00a7e630(owner, matrix, velocity, c);
}
} // namespace bsp
