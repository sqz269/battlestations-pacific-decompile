#pragma once
#include "bsp/native_profile_settings.hpp"
#include "bsp/native_input_settings_lifetime.hpp"
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
// F88980..F88A3B / the options screen's actual BCh copy. Opaque bytes and
// padding are not initialized by the constructor. The static owner starts zero
// through loader storage; ordinary copies must supply their own preimage.
struct alignas(4) NativeGameSettingsStorage { std::byte bytes[0xbc]; };
static_assert(sizeof(NativeGameSettingsStorage)==0xbc);
static_assert(std::is_trivially_default_constructible_v<NativeGameSettingsStorage>);
struct NativeGameSettingsCalls : NativeProfileCollectionCalls {
    virtual std::uint32_t online_state_00a3e500(NativeOnlineManagerStorage*);
    virtual std::uint8_t selected_user_00a3e510(NativeOnlineManagerStorage*);
    virtual void import_profile_008d45d0(void*,NativeProfileSettingsContext&);
    virtual void delete_current_input(void*,std::uint32_t,NativeInputSettingsLifetimeContext&);
    virtual int register_shutdown_00bf6ff5(void (*shutdown)());
    virtual void free_00bf6989(void*);
};
struct NativeGameSettingsContext {
    NativeStringStorage& strings;
    NativeStringRawPoolContext& raw_strings;
    NativeProfileSettingsContext& profile;
    NativeInputSettingsLifetimeContext& input;
    NativeGameSettingsCalls& calls;
    const volatile std::uint32_t& audio_bits_00ce3800;
    const volatile std::uint32_t& video_bits_00ce7d20;
    const char* empty_00ce3a0c;
};
struct NativeGameSettingsOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    void* owner{};
    NativeGameSettingsContext* context{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    NativeGameSettingsOperation()=default;
    ~NativeGameSettingsOperation();
    NativeGameSettingsOperation(const NativeGameSettingsOperation&)=delete;
    NativeGameSettingsOperation& operator=(const NativeGameSettingsOperation&)=delete;
    // Diagnostic caller must resolve retained native ownership first. No
    // resource release, rollback or replay is hidden behind acknowledgement.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Full normal bodies; explicit source contexts, not original binary ABI.
// 8D4950: no native arguments/RET. Capture the first manager's section,
// recheck publication, resolve manager again, unregister CURRENT input,
// reload/delete CURRENT input through slot0 flags1, clear, leave captured lock.
void release_native_settings_input_008d4950(NativeInputSettingsLifetimeContext&,NativeGameSettingsCalls&);
// 8D7710: ECX BCh owner, EAX same, RET. Partial byte/DWORD stores, actual
// string construction and current-manager selected-user import; no zero-fill.
void* construct_native_game_settings_008d7710(void*,NativeGameSettingsContext&,NativeGameSettingsOperation&);
// 8D78D0: ECX owner, RET. Input release, clan string, A4 then98 vectors.
// Preserve headers after backing frees, and retain partial effects on failure.
void destroy_native_game_settings_008d78d0(void*,NativeGameSettingsContext&,NativeGameSettingsOperation&);
// 8D7980: ECX owner, stack flags, EAX original owner, RET4; bit0 controls free.
void* delete_native_game_settings_008d7980(void*,std::uint32_t,NativeGameSettingsContext&,NativeGameSettingsOperation&);
// Caller supplies the actual static storage and its stable source shutdown
// thunk. CD2D80 constructs before registering CDEEC0; expose atexit's result.
int initialize_native_game_settings_static_00cd2d80(NativeGameSettingsStorage&,NativeGameSettingsContext&,NativeGameSettingsOperation&,void (*shutdown)());
void destroy_native_game_settings_static_00cdeec0(NativeGameSettingsStorage&,NativeGameSettingsContext&,NativeGameSettingsOperation&);
} // namespace bsp
