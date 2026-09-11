#pragma once

#include "bsp/audio_online_startup.hpp"
#include "bsp/frame_clock.hpp"
#include "bsp/session_polls.hpp"

#include <array>
#include <bitset>
#include <cstdint>

namespace bsp {

// Bytes whose native stack/cache values are known. Backing zeros for unknown
// bytes are not recovered padding: defined is authoritative. SDK adapters mark
// only bytes they know the call wrote, normally name through its terminator.
struct XLiveUserName128 {
    std::array<std::uint8_t, 128> bytes{};
    std::bitset<128> defined{};
};

// Additional manager storage. Caller supplies the scalar preimage; these
// routines are not a constructor. A default name has no known bytes.
// +10 is deliberately absent: the timestamp frequency low DWORD aliases the
// existing OnlineSystemState.field_10 and is read/written there.
struct XLiveSigninStorage {
    bool debounce_pending_04;
    std::uint64_t debounce_ticks_08;
    std::uint32_t debounce_frequency_high_14;
    const void* callback_18;
    std::uint8_t flag_2d;
    std::uint8_t flag_2e;
    XLiveUserName128 cached_username_90;
    std::uint64_t xuid_110;
    bool privilege_118;
};

struct XLiveSigninState {
    OnlineSystemState& online;
    PlatformManagerFlags& flags;
    XLiveSigninStorage& storage;
    bool& signin_flag_120; // bind XLiveSystemPumpState::signin_flag_120
    bool& signin_flag_3bc; // bind XLiveSystemPumpState::signin_flag_3bc
};

class XLiveSigninHost {
public:
    virtual ~XLiveSigninHost() = default;
    virtual std::uint32_t user_get_signin_state(std::uint32_t user_index) = 0;
    virtual std::uint32_t user_get_name(std::uint32_t user_index,
        XLiveUserName128& output, std::uint32_t capacity) = 0;
    // SDK success must define the complete XUID; failure is overwritten with 0.
    virtual std::uint32_t user_get_xuid(std::uint32_t user_index,
        std::uint64_t& output) = 0;
    virtual std::uint32_t user_check_privilege(std::uint32_t user_index,
        std::uint32_t privilege, std::int32_t& output_bool) = 0;
    // Actual current clock virtual +20 (00BEE080), with no invented timestamps.
    virtual ClockTimestamp sample_clock_vslot_20() = 0;
    // Same callback boundary as XLiveStartupHost. Both native call sites here
    // set ECX=0 and supply no stack arguments. Callback is the captured pointer.
    virtual void invoke_state_callback(const void* callback) = 0;
};

// Full 00A3E6A0, ECX=manager, plain RET. Updates the canonical restart flags;
// +3BC is cleared only when both +120 and +3BC were nonzero.
void restart_signin_00a3e6a0(XLiveSigninState) noexcept;

// Full normal 00A3EBD0, ECX=manager, forced byte stack, RET4. SDK queries are
// always user0. Nonforced signout returns only after state/name/XUID queries.
// Errors zero only the fields observed in assembly. All 128 name bytes and
// their knownness are committed, without inventing native unwritten padding.
void refresh_cached_local_user_00a3ebd0(XLiveSigninState, bool forced,
    XLiveSigninHost&);

// Full 00A3F3E0, ECX=manager, RET: while pending, sample current clock, subtract
// the live saved timestamp using 00530890, spill x87 seconds to float, and test
// ordered >1.0. Clear pending BEFORE forced refresh when the threshold passes.
void poll_signin_debounce_00a3f3e0(XLiveSigninState, XLiveSigninHost&);

// Full 00A3F440, ECX=manager, RET: pending -> forced refresh then clear flag;
// otherwise nonforced refresh, set flag, sample clock, copy the 16-byte time.
void refresh_signin_00a3f440(XLiveSigninState, XLiveSigninHost&);

// Full 00A3E600, ECX=manager, mask byte stack, RET4. Bit0 queries user0 name;
// on success/different C string copy all128 bytes, then invoke current callback18
// only when current signin_state_11c==0. Name comparison is case-sensitive.
void profile_setting_changed_00a3e600(XLiveSigninState, std::uint8_t mask,
    XLiveSigninHost&);

// Raw pump load [manager+8Ch+index*4], preserving DWORD offset wrap. Index0 is
// Online.connected_flag; following 32 words are the SAME cached username bytes.
// This is not a user-state array. Unknown bytes/ranges raise an explicit error.
std::uint32_t read_cached_manager_dword_8c_indexed(XLiveSigninState,
    std::uint32_t index);

} // namespace bsp
