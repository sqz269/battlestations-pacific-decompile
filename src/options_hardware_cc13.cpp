#include "bsp/options_hardware_cc13.hpp"

#include <cstring>
#include <utility>

namespace bsp {

void save_profile_settings_008d6c00(const GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsSaveHost& host,
    ProfileCompletion completion)
{
    // 008D6C1F..008D6C37: no storage-operation reset and no game check here,
    // unlike the restore at 008D7A50; the gate is the Xenon selected user.
    if (host.storage_available_21() && host.xenon_has_selected_user_00a3e510()) {
        // 008D6C61: the profile save name when present, otherwise the storage
        // manager's fallback name (008D6C80..008D6CBA).
        const std::string name = host.profile_has_save_name_007f8ca0()
            ? host.copy_profile_save_name_684()
            : host.storage_fallback_name_004c5e00();
        // 008D6CC4/008D6CCE: 00436710 builds the empty string and 00449AF0
        // compares headers, so only nonemptiness matters.
        if (!name.empty()) {
            // 008D6D06 stores the callback before the request; 008D6D11 passes
            // the continuation literal 008D6B70 to the driver.
            state.completion = std::move(completion);
            host.request_single_write_00bd3e10(name, kSettingsSaveStorageKind);
            host.run_storage_operation_006adb50([&settings, &state, &host] {
                complete_profile_settings_save_008d6b70(settings, state, host);
            });
            return;
        }
    }
    // 008D6D7B..008D6D83: rejection calls the argument; the slot is untouched.
    if (completion) completion();
}

void complete_profile_settings_save_008d6b70(const GameSettingsBlock& settings,
    ProfileSettingsRestoreState& state, ProfileSettingsSaveHost& host)
{
    if (host.storage_state_08() == kSettingsSaveWriterState) {
        {
            SettingsWriter& writer = host.begin_settings_writer_00ce4104();
            struct Retire {
                ProfileSettingsSaveHost& host;
                SettingsWriter& writer;
                ~Retire() { host.end_settings_writer(writer); }
            } retire{host, writer};
            write_settings_008d64a0(settings, writer); // 008D6BB3, ECX = 00F88980
        }
        // 008D6BB8..008D6BC8: take and clear the slot, then hand the saved
        // value (possibly null in the native) to the driver as its next step.
        ProfileCompletion next = std::move(state.completion);
        state.completion = nullptr;
        host.run_storage_operation_006adb50(std::move(next));
        return;
    }
    // 008D6BDC..008D6BEF: take and clear, then call when nonnull.
    ProfileCompletion next = std::move(state.completion);
    state.completion = nullptr;
    if (next) next();
}

std::string newest_valid_storage_slot_00beb630(const std::vector<StorageSlotCandidate>& entries)
{
    std::string best;           // 00BEB669: 00436710 empty string
    std::uint64_t best_time = 0;
    for (const StorageSlotCandidate& entry : entries) {
        if (!entry.has_valid_marker) continue;
        if (entry.valid_write_time > best_time) {
            best = entry.name;
            best_time = entry.valid_write_time;
        }
    }
    return best;
}

bool native_string_vector_equal_008d48e0(
    const std::vector<std::string>& self, const std::vector<std::string>& other)
{
    if (self.size() != other.size()) return false; // 008D48EB
    for (std::size_t i = 0; i < self.size(); ++i) {
        const std::string& a = self[i];
        const std::string& b = other[i];
        if (a.empty()) {
            if (!b.empty()) return false; // 008D4914 NEG/SBB
        } else if (b.empty()) {
            return false;                 // 008D491F
        } else if (_stricmp(a.c_str(), b.c_str()) != 0) {
            return false;                 // 008D4929
        }
    }
    return true;
}

namespace {

bool compare_fields(const NativeGameSettingsStorage& a, const NativeGameSettingsStorage& b,
    const SettingsComparedField* begin, const SettingsComparedField* end) noexcept
{
    for (const SettingsComparedField* f = begin; f != end; ++f) {
        const std::byte* pa = a.data() + f->offset;
        const std::byte* pb = b.data() + f->offset;
        switch (f->width) {
        case SettingsFieldWidth::Byte:
            if (*pa != *pb) return false;
            break;
        case SettingsFieldWidth::Dword: {
            std::uint32_t va, vb;
            std::memcpy(&va, pa, 4);
            std::memcpy(&vb, pb, 4);
            if (va != vb) return false;
            break;
        }
        case SettingsFieldWidth::Float: {
            float va, vb;
            std::memcpy(&va, pa, 4);
            std::memcpy(&vb, pb, 4);
            if (!(va == vb)) return false;
            break;
        }
        }
    }
    return true;
}

template <std::size_t N>
bool compare_fields(const NativeGameSettingsStorage& a, const NativeGameSettingsStorage& b,
    const SettingsComparedField (&fields)[N]) noexcept
{
    return compare_fields(a, b, fields, fields + N);
}

// 00435C40: recorded lengths must match, then __stricmp on the text.
bool native_string_equal_insensitive_00435c40(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) return false;
    if (a.empty()) return true;
    return _stricmp(a.c_str(), b.c_str()) == 0;
}

} // namespace

void apply_weather_camera_shake_00445790(WeatherConfig& config,
    WeatherShakeTarget* mover, NativeStringStorage& strings)
{
    if (!mover) return; // 004457B2
    std::array<float, 3> amplitudes = config.base_amplitudes_44; // 004457B8..004457D3
    float m;
    {
        NativeString view;
        view.assign_0041e870(strings, mover->view_mode_name_12c()); // 004457D7/004457DE
        struct Release {
            NativeString& s;
            NativeStringStorage& strings;
            ~Release() { s.release_to(strings); } // 00445842..00445856
        } release{view, strings};
        // 004457F2 then 00445805: the weather map first, each value used once.
        const double weather = lookup_weather_float_00444be0(
            config.weather_amp_multipliers_68.value(), config.current_weather_2c, strings);
        const double view_mult = lookup_weather_float_00444be0(
            config.view_mode_amp_multipliers_5c.value(), view, strings);
        // 0044580C FMUL qword, 0044581E FSTP dword: both products are exact
        // before the float rounding, so double arithmetic reproduces x87.
        m = static_cast<float>(view_mult * weather);
    }
    for (float& a : amplitudes) a = static_cast<float>(static_cast<double>(m) * a);
    mover->set_shake_0042c4c0(amplitudes, config.frequencies_50); // 004458A0
}

void set_current_weather_00445a30(WeatherConfig& config, NativeString name,
    WeatherShakeTarget* current_mover, NativeStringStorage& strings)
{
    struct Release {
        NativeString& s;
        NativeStringStorage& strings;
        ~Release() { s.release_to(strings); }
    } release{name, strings};
    // 00445A5A..00445A81. The native tests the length before the copy where
    // the 00BE0A30 fragment tests the buffer; they differ only on a failed
    // allocation, which the host storage does not return.
    config.current_weather_2c.copy_from_00be0a30_fragment(strings, name);
    config.current_weather_modifier_34 = lookup_weather_float_00444be0(
        config.weather_recon_modifiers_38.value(), name, strings); // 00445A8C/00445A93
    apply_weather_camera_shake_00445790(config, current_mover, strings); // 00445AA4
}

void read_weather_state_00445bf0(WeatherConfig& config, WeatherStateReader& reader,
    WeatherShakeTarget* current_mover, NativeStringStorage& strings)
{
    reader.read_day_time(config.start_day_time_04); // 00445C42
    NativeString active;
    struct Release {
        NativeString& s;
        NativeStringStorage& strings;
        ~Release() { s.release_to(strings); }
    } release{active, strings};
    reader.read_active_weather(active); // 00445C7B
    NativeString argument;              // 00445C7D..00445CB8 by-value copy
    argument.copy_from_00be0a30_fragment(strings, active);
    set_current_weather_00445a30(config, std::move(argument), current_mover, strings);
    float& t = config.start_day_time_04;
    while (0.0f > t) t = static_cast<float>(t + kWeatherDayHoursD);            // 00445CC8..00445CD9
    while (t > kWeatherDayHoursF) t = static_cast<float>(t - kWeatherDayHoursD); // 00445CDB..00445CF9
}

bool game_settings_equal_008d4af0(
    const GameSettingsComparand& self, const GameSettingsComparand& other)
{
    return compare_fields(self.bytes, other.bytes, kSettingsEqualityHead) &&
        native_string_vector_equal_008d48e0(self.downloaded_content_98,
            other.downloaded_content_98) && // 008D4D15: ECX = this+98h
        compare_fields(self.bytes, other.bytes, kSettingsEqualityMiddle) &&
        native_string_equal_insensitive_00435c40(self.clan_text_b4, other.clan_text_b4) &&
        compare_fields(self.bytes, other.bytes, kSettingsEqualityTail);
}

} // namespace bsp
