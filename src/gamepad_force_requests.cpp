#include "bsp/gamepad_force_requests.hpp"

#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
std::uint32_t float_bits(float value) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}
float bits_float(std::uint32_t bits) noexcept {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
float add_float_x87(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fadd b
        fstp result
    }
    return result;
}
float subtract_float_x87(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fsub b
        fstp result
    }
    return result;
}
void check_channel(std::uint32_t channel) {
    if (channel >= 2) throw std::out_of_range("force channel must address one of the two native amplitudes");
}
InputBindingDeviceGroups* current_groups(GamepadForceContext& context) {
    auto* groups = context.host.current_input_groups_00f8bbf4();
    if (!groups) throw std::runtime_error("force requests require the current actual input backend");
    return groups;
}
std::size_t gamepad_count(const InputBindingDeviceGroups& groups) noexcept {
    return groups.size() > 2 ? groups[2].size() : 0;
}
InputDevice* gamepad_at(InputBindingDeviceGroups& groups, std::size_t index) noexcept {
    return index < gamepad_count(groups) ? groups[2][index] : nullptr;
}
void delete_request_preserving_entry(std::unique_ptr<GamepadForceRequest>& owner) {
    // Native deleting vslot runs while its node still contains the old pointer.
    delete owner.get();
    owner.release();
}
void destroy_requests_in_order(GamepadForceState& state) {
    for (auto& entry : state.requests) delete_request_preserving_entry(entry.second);
    state.requests.clear();
}
bool insert_request(GamepadForceState& state, std::uint32_t id, GamepadForceRequest* request) {
    if (state.requests.find(id) != state.requests.end()) return false;
    state.requests.emplace(id, std::unique_ptr<GamepadForceRequest>(request));
    return true;
}
}

ConstantGamepadForceRequest::ConstantGamepadForceRequest(std::uint32_t channel,
    float amplitude, float remaining) : channel_08(channel), remaining_10(remaining) {
    payload_0c = float_bits(amplitude);
}
std::uint32_t ConstantGamepadForceRequest::channel() const { return channel_08; }
float ConstantGamepadForceRequest::value() const { return bits_float(payload_0c); }
bool ConstantGamepadForceRequest::expired() const { return remaining_10 <= 0.0f; }
void ConstantGamepadForceRequest::update(float seconds) {
    // D7A278 is the exact double representation of FLT_MAX; NaN is unequal.
    if (remaining_10 != (std::numeric_limits<float>::max)())
        remaining_10 = subtract_float_x87(remaining_10, seconds);
}
FadingGamepadForceRequest::FadingGamepadForceRequest(std::uint32_t channel,
    float amplitude, float duration) : channel_08(channel), duration_10(duration) {
    payload_0c = float_bits(amplitude);
}
std::uint32_t FadingGamepadForceRequest::channel() const { return channel_08; }
float FadingGamepadForceRequest::value() const {
    const float duration = duration_10, elapsed = elapsed_14, amplitude = bits_float(payload_0c);
    float result;
    __asm {
        fld duration
        fld st(0)
        fsub elapsed
        fdivrp st(1),st(0)
        fmul amplitude
        fstp result
    }
    return result;
}
bool FadingGamepadForceRequest::expired() const { return elapsed_14 >= duration_10; }
void FadingGamepadForceRequest::update(float seconds) {
    elapsed_14 = add_float_x87(seconds, elapsed_14);
}
AlternatingGamepadForceRequest::AlternatingGamepadForceRequest(std::uint32_t channel,
    bool second_first, float first_value, float second_value, float first_period,
    float second_period, float duration) : channel_08(channel), first_value_10(first_value),
    second_value_14(second_value), first_period_18(first_period),
    second_period_1c(second_period), duration_20(duration) {
    payload_0c = second_first ? 1u : 0u;
}
std::uint32_t AlternatingGamepadForceRequest::channel() const { return channel_08; }
float AlternatingGamepadForceRequest::value() const {
    if ((payload_0c & 0xffu) != 0)
        return second_period_1c > phase_24 ? second_value_14 : first_value_10;
    return first_period_18 > phase_24 ? first_value_10 : second_value_14;
}
bool AlternatingGamepadForceRequest::expired() const { return phase_24 >= duration_20; }
void AlternatingGamepadForceRequest::update(float seconds) {
    const float next = add_float_x87(seconds, phase_24);
    const float period = add_float_x87(second_period_1c, first_period_18);
    // BF857A dispatch record E15500 explicitly names fmod. Use the real CRT.
    phase_24 = static_cast<float>(std::fmod(static_cast<double>(next), static_cast<double>(period)));
}

GamepadForceState::~GamepadForceState() { destroy_requests_in_order(*this); }

void refresh_gamepad_force_channel_00a949a0(InputDevice& device, std::uint32_t channel,
    GamepadForceContext& context) {
    if (!context.enabled_e12f2c) return;
    check_channel(channel);
    auto& state = context.host.force_state(device);
    float maximum = 0.0f;
    for (auto& entry : state.requests) {
        if (!entry.second) throw std::runtime_error("native force registry contains a null request");
        const float value = entry.second->value(); // value before channel, unlike pump
        const auto request_channel = entry.second->channel();
        if (request_channel == channel && value > maximum) maximum = value;
    }
    if (maximum != state.amplitudes[channel]) {
        state.amplitudes[channel] = maximum;
        context.host.set_force_vslot38(device, channel, maximum);
    }
}
GamepadForceRequest* find_gamepad_force_request_00a94b50(GamepadForceState& state,
    std::uint32_t id) noexcept {
    const auto found = state.requests.find(id);
    return found == state.requests.end() ? nullptr : found->second.get();
}
bool set_gamepad_force_payload_00a94bc0(InputDevice& device, std::uint32_t id,
    float value, GamepadForceContext& context) {
    auto* request = find_gamepad_force_request_00a94b50(context.host.force_state(device), id);
    if (!request) return false;
    request->payload_0c = float_bits(value);
    const auto channel = request->channel();
    refresh_gamepad_force_channel_00a949a0(device, channel, context);
    return true;
}
void set_gamepad_force_enabled_00a94c50(bool enabled, GamepadForceContext& context) {
    context.enabled_e12f2c = enabled;
    auto* groups = current_groups(context);
    const auto count = gamepad_count(*groups);
    for (std::size_t i = 0; i < count; ++i) {
        auto* device = gamepad_at(*groups, i);
        if (!device) continue;
        auto& state = context.host.force_state(*device);
        for (std::uint32_t channel = 0; channel < 2; ++channel) {
            if (context.enabled_e12f2c) refresh_gamepad_force_channel_00a949a0(*device, channel, context);
            else {
                context.host.set_force_vslot38(*device, channel, 0.0f);
                state.amplitudes[channel] = 0.0f; // after output; even if already zero
            }
        }
        groups = current_groups(context);
    }
}
GamepadForceRequest* find_current_gamepad_force_request_00a94d40(std::uint32_t id,
    GamepadForceContext& context) {
    auto* groups = current_groups(context);
    const auto count = gamepad_count(*groups);
    for (std::size_t i = 0; i < count; ++i) {
        auto* device = gamepad_at(*groups, i);
        if (!device) continue;
        if (auto* request = find_gamepad_force_request_00a94b50(context.host.force_state(*device), id))
            return request;
        groups = current_groups(context);
    }
    return nullptr;
}
bool remove_gamepad_force_request_00a95410(InputDevice& device, std::uint32_t id,
    GamepadForceContext& context) {
    auto& state = context.host.force_state(device);
    auto found = state.requests.find(id);
    if (found == state.requests.end()) return false;
    if (!found->second) throw std::runtime_error("native force registry contains a null request");
    const auto channel = found->second->channel();
    delete_request_preserving_entry(found->second);
    state.requests.erase(found);
    refresh_gamepad_force_channel_00a949a0(device, channel, context);
    return true;
}
void pump_gamepad_force_requests_00a954c0(InputDevice& device, float seconds,
    GamepadForceContext& context) {
    auto& state = context.host.force_state(device);
    const auto previous = state.amplitudes;
    state.amplitudes.fill(0.0f);
    auto current = state.requests.begin();
    while (current != state.requests.end()) {
        auto entry = current++; // native advances before invoking request code
        auto* request = entry->second.get();
        if (!request) throw std::runtime_error("native force registry contains a null request");
        request->update(seconds);
        if (request->expired()) {
            delete_request_preserving_entry(entry->second);
            state.requests.erase(entry);
        } else if (context.enabled_e12f2c) {
            const auto channel = request->channel(); // channel before value
            const float value = request->value();
            check_channel(channel);
            if (value > state.amplitudes[channel]) state.amplitudes[channel] = value;
        }
    }
    if (context.enabled_e12f2c) {
        for (std::uint32_t channel = 0; channel < 2; ++channel) {
            const float value = state.amplitudes[channel];
            if (previous[channel] != value) context.host.set_force_vslot38(device, channel, value);
        }
    }
}
bool insert_gamepad_force_request_00a95780(InputDevice& device, std::uint32_t id,
    GamepadForceRequest* request, GamepadForceContext& context) {
    if (!request) throw std::invalid_argument("native force insertion requires a request");
    const bool inserted = insert_request(context.host.force_state(device), id, request);
    const auto channel = request->channel(); // native calls this even on duplicate ID
    refresh_gamepad_force_channel_00a949a0(device, channel, context);
    return inserted;
}
void validate_gamepad_force_handle_00a957d0(std::uint32_t& id, GamepadForceContext& context) {
    if (id && !find_current_gamepad_force_request_00a94d40(id, context)) id = 0;
}
void remove_current_gamepad_force_request_00a957f0(std::uint32_t id, GamepadForceContext& context) {
    auto* groups = current_groups(context);
    const auto count = gamepad_count(*groups);
    for (std::size_t i = 0; i < count; ++i) {
        auto* device = gamepad_at(*groups, i);
        if (!device) continue;
        const bool removed = remove_gamepad_force_request_00a95410(*device, id, context);
        groups = current_groups(context); // native reloads even on successful removal
        if (removed) return;
    }
}
void clear_gamepad_force_requests_00a95890(InputDevice& device, GamepadForceContext& context) {
    destroy_requests_in_order(context.host.force_state(device));
    pump_gamepad_force_requests_00a954c0(device, 0.0f, context);
}
void pump_current_gamepad_force_requests_00a95960(float seconds, GamepadForceContext& context) {
    auto* groups = current_groups(context);
    const auto count = gamepad_count(*groups);
    for (std::size_t i = 0; i < count; ++i) {
        auto* device = gamepad_at(*groups, i);
        if (!device) continue;
        pump_gamepad_force_requests_00a954c0(*device, seconds, context);
        groups = current_groups(context);
    }
}
void destroy_gamepad_force_state_00a95a80(InputDevice& device, GamepadForceContext& context) {
    auto& state = context.host.force_state(device);
    destroy_gamepad_force_state_00a95a80(state, context.enabled_e12f2c);
}
void destroy_gamepad_force_state_00a95a80(GamepadForceState& state, const bool& enabled) {
    destroy_requests_in_order(state);
    const auto previous = state.amplitudes;
    state.amplitudes.fill(0.0f);
    // A95AA2 already installed D5B670; its +38 slot is BF698E (_purecall).
    // Do not silently route that base-destructor call to the derived device.
    if (enabled && (previous[0] != 0.0f || previous[1] != 0.0f))
        throw std::logic_error("native gamepad base destruction reaches purecall with enabled nonzero amplitudes");
    // Remaining native work destroys the empty STL sentinel and base layout.
}
std::uint32_t submit_gamepad_force_request_00a95bf0(std::uint32_t device_index,
    GamepadForceRequest* request, GamepadForceContext& context) {
    auto* groups = current_groups(context);
    auto* device = gamepad_at(*groups, device_index);
    if (!device) { delete request; return 0; }
    const auto id = context.next_id_e12f30;
    ++context.next_id_e12f30;
    if (context.next_id_e12f30 == 0) context.next_id_e12f30 = 1;
    insert_gamepad_force_request_00a95780(*device, id, request, context);
    return id;
}

void set_joystick_force_00a98cc0(JoystickForceOutputState& state, std::uint32_t channel,
    float value) {
    static_assert(sizeof(DIEFFECT) == 0x38);
    static_assert(sizeof(DICONSTANTFORCE) == 4 && sizeof(DIRAMPFORCE) == 8);
    check_channel(channel);
    if (!state.effects_b24[channel]) return; // native does not read clock here
    if (!state.current_clock_01090ab0_vslot14)
        throw std::runtime_error("joystick force output requires the current actual clock");
    const float now = timestamp_seconds_x87(state.current_clock_01090ab0_vslot14());
    if (now > state.activity_deadline_b38) value = 0.0f;
    DIEFFECT effect{};
    effect.dwSize = sizeof(effect); effect.dwFlags = 0x12;
    effect.dwDuration = 0xffffffffu; effect.dwGain = 10000;
    effect.dwTriggerButton = 0xffffffffu;
    DICONSTANTFORCE constant{};
    DIRAMPFORCE ramp{};
    if (state.effect_kind_b10 == 0 || state.effect_kind_b10 == 1) {
        const double scale = 10000.0; // CE4BD8; native BF7420 conversion boundary
        double scaled;
        __asm {
            fld value
            fmul scale
            fstp scaled
        }
        auto magnitude = static_cast<std::int32_t>(scaled);
        if (state.direction_mode_b14 == 1 && channel == 1)
            magnitude = static_cast<std::int32_t>(0u - static_cast<std::uint32_t>(magnitude));
        else if (state.effect_kind_b10 == 1 && state.direction_mode_b14 == 0 && channel == 1)
            magnitude /= 3;
        if (state.effect_kind_b10 == 0) {
            constant.lMagnitude = magnitude;
            effect.cbTypeSpecificParams = sizeof(constant);
            effect.lpvTypeSpecificParams = &constant;
        } else {
            ramp.lStart = magnitude; ramp.lEnd = magnitude;
            effect.cbTypeSpecificParams = sizeof(ramp);
            effect.lpvTypeSpecificParams = &ramp;
        }
    }
    // Reload actual effect pointer after the clock call, as the native does.
    auto* output = state.effects_b24[channel];
    if (!output) throw std::runtime_error("joystick effect disappeared during clock callback");
    output->SetParameters(&effect, 0x20000100); // DIEP_TYPESPECIFICPARAMS | DIEP_START
}

} // namespace bsp
