#pragma once

#include "bsp/native_string.hpp"
#include "bsp/profile_reset.hpp"
#include "bsp/xlive_signin.hpp"

#include <cstdint>

namespace bsp {

class XLiveLibrary;

constexpr std::uint32_t kOnlineProfileNameCallback = 0x00737d60u;

// Only the selected manager and game fields needed by these callbacks. Return
// references to live canonical storage; do not cache singleton values here.
struct XLiveApplicationManagerBinding {
    const OnlineSystemState& online;
    const XLiveSigninStorage& signin;
};
struct XLiveApplicationProfileBinding {
    ProfileResetState& profile;             // current game +650h
    std::array<char, 32>& game_name;         // current game +1FF0h
};
class XLiveApplicationGlobals {
public:
    virtual ~XLiveApplicationGlobals() = default;
    virtual XLiveApplicationManagerBinding current_manager_00f8abe8() = 0;
    virtual XLiveApplicationProfileBinding current_game_profile_00e188a8() = 0;
};

class XLiveApplicationContextHost {
public:
    virtual ~XLiveApplicationContextHost() = default;
    // Ordinal 5277, Win32 stdcall, three DWORD slots. Neither callback consumes
    // EAX; this boundary makes no claim about a returned SDK status.
    virtual void user_set_context(std::uint32_t user_index,
        std::uint32_t context_id, std::uint32_t value) = 0;
};

// Genuine DLL forwarding. The borrowed library/module must outlive this object
// and every invocation. Construction performs no SDK call and loads no DLL.
class XLiveApplicationContextAdapter final : public XLiveApplicationContextHost {
public:
    explicit XLiveApplicationContextAdapter(const XLiveLibrary& library);
    void user_set_context(std::uint32_t user_index,
        std::uint32_t context_id, std::uint32_t value) override;
private:
    void* module_;
};

// Full 00735510/00735520: incoming ECX is the user index, no stack parameters,
// plain RET. The original SDK call receives context 8001h and value 6 or 4.
void set_online_context_six_00735510(std::uint32_t incoming_ecx,
    XLiveApplicationContextHost&);
void set_online_context_four_00735520(std::uint32_t incoming_ecx,
    XLiveApplicationContextHost&);

// 00A3EAE0 pointer-arithmetic projection: native returns manager+this offset.
// SHL/ADD wrap as DWORDs. This is not an array of four cached user names.
// Existing signin_state_11c names a selected user index/sentinel here, not the
// SDK sign-in status cached at +8Ch. See the call-site evidence in the doc.
std::uint32_t selected_username_offset_00a3eae0(const OnlineSystemState&) noexcept;

// Full normal 00737D60 and temporary-string unwind. Incoming ECX is ignored.
// Supported source is the canonical +90h cache with a known NUL terminator;
// out-of-region or unspecified bytes throw instead of inventing native bytes.
// Reload game separately before display-name and player-name setters, in that
// order, using the existing canonical profile operations.
void apply_online_profile_name_00737d60(XLiveApplicationGlobals&,
    NativeStringStorage& = crt_string_storage());

// Concrete dispatcher for the native address identities stored at manager
// +18/+20/+24. No image address is executed. Existing startup/pump/sign-in call
// sites explicitly supply ECX=0; direct register-input callers can preserve
// another value with invoke_callback. Unknown identities are explicit errors.
class XLiveApplicationCallbackInvoker {
public:
    XLiveApplicationCallbackInvoker(XLiveApplicationGlobals& globals,
        XLiveApplicationContextHost& sdk,
        NativeStringStorage& storage = crt_string_storage()) noexcept;
    void invoke_state_callback(const void* callback);
    void invoke_callback(const void* callback, std::uint32_t incoming_ecx);
private:
    XLiveApplicationGlobals& globals_;
    XLiveApplicationContextHost& sdk_;
    NativeStringStorage& storage_;
};

} // namespace bsp
