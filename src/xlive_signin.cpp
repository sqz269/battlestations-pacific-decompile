#include "bsp/xlive_signin.hpp"

#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error XLive sign-in debounce requires MSVC Win32 x87 behavior.
#endif

namespace bsp {
namespace {

std::uint8_t known_name_byte(const XLiveUserName128& name, std::size_t index) {
    if (!name.defined.test(index))
        throw std::logic_error("Native cached username byte is unspecified");
    return name.bytes[index];
}

bool same_name(const XLiveUserName128& left, const XLiveUserName128& right) {
    for (std::size_t index = 0; index != 128; ++index) {
        const auto first = known_name_byte(left, index);
        const auto second = known_name_byte(right, index);
        if (first != second) return false;
        if (first == 0) return true;
    }
    throw std::logic_error("Native username comparison exceeds its known 128-byte region");
}

std::int64_t signed_bits(std::uint64_t bits) noexcept {
    std::int64_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

ClockTimestamp saved_timestamp(XLiveSigninState state) noexcept {
    const auto frequency_bits =
        (static_cast<std::uint64_t>(state.storage.debounce_frequency_high_14) << 32)
        | state.online.field_10;
    return {signed_bits(state.storage.debounce_ticks_08), signed_bits(frequency_bits)};
}

void store_timestamp(XLiveSigninState state, const ClockTimestamp& timestamp) noexcept {
    state.storage.debounce_ticks_08 = static_cast<std::uint64_t>(timestamp.ticks);
    const auto frequency_bits = static_cast<std::uint64_t>(timestamp.frequency);
    state.online.field_10 = static_cast<std::uint32_t>(frequency_bits);
    state.storage.debounce_frequency_high_14 = static_cast<std::uint32_t>(frequency_bits >> 32);
}

bool elapsed_past_one_second(const ClockTimestamp& difference) noexcept {
    const float seconds = timestamp_seconds_x87(difference);
    unsigned char elapsed;
    __asm {
        fld1
        fld seconds
        fcomip st(0), st(1)
        fstp st(0)
        seta elapsed
    }
    return elapsed != 0;
}

} // namespace

void restart_signin_00a3e6a0(XLiveSigninState state) noexcept {
    if (state.signin_flag_120 && state.signin_flag_3bc)
        state.signin_flag_3bc = false;
    state.online.signin_flag_119 = false;
    state.online.signin_flag_11a = false;
    state.online.signin_state_11c = 1;
    state.flags.storage_removed = false;
    state.storage.flag_2d = 0;
    state.storage.flag_2e = 0;
    state.signin_flag_120 = false;
    state.online.signin_slot_124 = -1;
    state.online.field_3b8 = 1;
    state.flags.profile_changed = true;
    state.flags.invite_accepted = false;
}

void refresh_cached_local_user_00a3ebd0(XLiveSigninState state, bool forced,
    XLiveSigninHost& host) {
    const auto signin = host.user_get_signin_state(0);
    std::int32_t privilege = 0;
    XLiveUserName128 name;
    if (host.user_get_name(0, name, 128) != 0) {
        name.bytes[0] = 0;
        name.defined.set(0);
    }
    std::uint64_t xuid;
    if (host.user_get_xuid(0, xuid) != 0) xuid = 0;
    if (!forced && signin == 0) return;

    if (signin == 2) {
        if (host.user_check_privilege(0, 0xfe, privilege) != 0) privilege = 0;
        if (state.online.field_3b8 == 0 || state.online.field_3b8 == 1)
            state.flags.link_failure = false;
    }

    if (state.online.field_3b8 == 0) {
        if ((signin != 0) != (state.online.connected_flag != 0)
            || !same_name(name, state.storage.cached_username_90)) {
            restart_signin_00a3e6a0(state);
        }
    } else {
        const auto* callback = state.online.callback_20;
        if (callback && signin != 0) host.invoke_state_callback(callback);
    }
    state.online.connected_flag = signin;
    state.storage.cached_username_90 = name;
    state.storage.xuid_110 = xuid;
    state.storage.privilege_118 = privilege != 0;
}

void poll_signin_debounce_00a3f3e0(XLiveSigninState state, XLiveSigninHost& host) {
    if (!state.storage.debounce_pending_04) return;
    const auto now = host.sample_clock_vslot_20();
    // Native saved-timestamp pointer remains live over clock sampling.
    const auto saved = saved_timestamp(state);
    ClockTimestamp difference;
    subtract_timestamp_00530890(difference, now, saved);
    if (elapsed_past_one_second(difference)) {
        state.storage.debounce_pending_04 = false;
        refresh_cached_local_user_00a3ebd0(state, true, host);
    }
}

void refresh_signin_00a3f440(XLiveSigninState state, XLiveSigninHost& host) {
    if (state.storage.debounce_pending_04) {
        refresh_cached_local_user_00a3ebd0(state, true, host);
        state.storage.debounce_pending_04 = false;
    } else {
        refresh_cached_local_user_00a3ebd0(state, false, host);
        state.storage.debounce_pending_04 = true;
        const auto now = host.sample_clock_vslot_20();
        store_timestamp(state, now);
    }
}

void profile_setting_changed_00a3e600(XLiveSigninState state, std::uint8_t mask,
    XLiveSigninHost& host) {
    if (!(mask & 1)) return;
    XLiveUserName128 name;
    if (host.user_get_name(0, name, 128) != 0) return;
    if (same_name(state.storage.cached_username_90, name)) return;
    state.storage.cached_username_90 = name;
    if (state.online.signin_state_11c == 0) {
        const auto* callback = state.storage.callback_18;
        if (callback) host.invoke_state_callback(callback);
    }
}

std::uint32_t read_cached_manager_dword_8c_indexed(XLiveSigninState state,
    std::uint32_t index) {
    const auto offset = index * 4u;
    if (!offset) return state.online.connected_flag;
    if (offset > 128u)
        throw std::out_of_range("Indexed manager DWORD is outside the defined cached-signin/name region");
    const auto start = static_cast<std::size_t>(offset - 4u);
    std::uint32_t value = 0;
    for (std::size_t byte = 0; byte != 4; ++byte) {
        value |= static_cast<std::uint32_t>(known_name_byte(
            state.storage.cached_username_90, start + byte)) << (byte * 8u);
    }
    return value;
}

} // namespace bsp
