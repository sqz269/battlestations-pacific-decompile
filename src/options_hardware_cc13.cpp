#include "bsp/options_hardware_cc13.hpp"
#include "bsp/engine_seg49_64_78_cc13.hpp"

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

namespace bsp {

bool raise_entity_warning(WarningReportHost& host, WarningEntity entity,
    std::string_view id, std::int32_t excluded_kind)
{
    host.lock_24();
    struct Unlock {
        WarningReportHost& host;
        ~Unlock() { host.unlock_24(); }
    } unlock{host};
    if (host.suppressed_d0()) return false;
    const std::int32_t slot = host.local_slot_18ec();
    if (slot < 0 || slot >= 8) return false;
    if (!(host.mission_clock_f876a4() > kWarningReportGrace)) return false; // COMISS/JBE
    const WarningEntityFlags flags = host.entity_flags(entity);
    if (!flags.present_5c || flags.flag_5d || flags.flag_60 || flags.flag_5e) return false;
    if (host.entity_suppressed_0077edf0(entity)) return false;
    if (!host.is_player_controlled_00965810(entity)) return false;
    if (excluded_kind >= 0 && host.is_kind_5c(entity, excluded_kind)) return false;
    host.report_entity_warning_00974150(entity, id);
    return true;
}

bool report_submarine_air_low_00977050(WarningReportHost& host, WarningEntity entity)
{
    return raise_entity_warning(host, entity, kWarningIdSubmarineAirLow);
}

bool report_submarine_air_critical_009771e0(WarningReportHost& host, WarningEntity entity)
{
    return raise_entity_warning(host, entity, kWarningIdSubmarineAirCritical);
}

bool report_submarine_depth_damage_00977370(WarningReportHost& host, WarningEntity entity)
{
    return raise_entity_warning(host, entity, kWarningIdSubmarineDepthDamage);
}

bool report_submarine_periscope_broken_00977500(WarningReportHost& host, WarningEntity entity)
{
    return raise_entity_warning(host, entity, kWarningIdSubmarinePeriscopeBroken);
}

bool report_torpedo_00977690(WarningReportHost& host, WarningEntity entity)
{
    return raise_entity_warning(host, entity, kWarningIdTorpedo, kTorpedoWarningExcludedKind);
}

cc13_seg49_64_78::WarningObject& construct_new_unit_warning_009756a0(cc13_seg49_64_78::WarningObject& w,
    cc13_seg49_64_78::WarningHost& host)
{
    cc13_seg49_64_78::construct_warning_base_00973a90(w, host);  // 009756BE
    w.vtable = 0x00D1B460;                                       // 009756D4
    w.id = "newunit";                                            // 009756DA resize 7, memcpy 00D1B458
    return w;
}

cc13_seg49_64_78::WarningObject& construct_powerup_warning_009757b0(cc13_seg49_64_78::WarningObject& w,
    cc13_seg49_64_78::WarningHost& host, bool network_session)
{
    cc13_seg49_64_78::construct_warning_base_00973a90(w, host);  // 009757D0
    w.vtable = 0x00D1B4B0;                                       // 009757E6
    w.id = "powerup";                                            // 009757EC resize 7, memcpy 00D1B4A8
    if (network_session) w.id += "multi";                        // 0097580E [[00E188A8]+1FE4h], 00CE39CC
    return w;
}

cc13_seg49_64_78::WarningObject& construct_named_powerup_warning_009758e0(cc13_seg49_64_78::WarningObject& w,
    cc13_seg49_64_78::WarningHost& host, const std::string& name, bool network_session)
{
    cc13_seg49_64_78::construct_warning_base_00973a90(w, host);  // 00975901
    w.vtable = 0x00D1B4B0;                                       // 0097591C, the powerup class
    w.id = "powerup" + name;                                     // 00975922 0043C130 (00D1B4A8 + name), 0097593C assign
    if (network_session) w.id += "multi";                        // 00975982 [[00E188A8]+1FE4h], 00CE39CC
    return w;
}

cc13_seg49_64_78::WarningObject& construct_score_warning_00975a10(cc13_seg49_64_78::WarningObject& w,
    cc13_seg49_64_78::WarningHost& host)
{
    cc13_seg49_64_78::construct_warning_base_00973a90(w, host);  // 00975A2E
    w.vtable = 0x00D1B4D4;                                       // 00975A44
    w.id = "score";                                              // 00975A4A resize 5, memcpy 00D18700
    return w;
}

cc13_seg49_64_78::WarningObject& construct_exit_zone_warning_00975340(cc13_seg49_64_78::WarningObject& w,
    cc13_seg49_64_78::WarningHost& host, WarningSubjectHost& subject, WarningEntity entity, bool exiting)
{
    // 00975368: the pushed 0 is the base's ignored second argument.
    cc13_seg49_64_78::construct_entity_warning_base_00973ba0(w, host, const_cast<void*>(entity));
    w.vtable = 0x00D1B344;                                       // 0097537E
    w.word_6c = 0x00D1B32C;                                      // 00975384
    w.id = "exitzone";                                           // 0097538B, 00D19074
    if (exiting) return w;                                       // 009753AD
    w.id += "enter";                                             // 009753B4, 00D1B1B0
    if (!entity) return w;                                       // 00975418
    WarningEntity owner = entity;
    if (subject.is_kind_5c(entity, 0x0F)) owner = subject.plane_owner_9d4(entity); // 00975428
    if (owner != subject.controlled_unit_e188d8()) return w;    // 00975434
    w.id += "player";                                            // 00975445, 00D199D0
    if (subject.is_kind_5c(entity, 6)) w.id += "ship";           // 009754AA, 00D186D4 (00425E10)
    return w;
}

cc13_seg49_64_78::WarningObject& construct_confirm_warning_00975540(cc13_seg49_64_78::WarningObject& w,
    cc13_seg49_64_78::WarningHost& host, WarningSubjectHost& subject, WarningEntity entity,
    const std::string& order)
{
    // 00975568: the pushed 0 is the base's ignored second argument.
    cc13_seg49_64_78::construct_entity_warning_base_00973ba0(w, host, const_cast<void*>(entity));
    w.vtable = 0x00D1B380;                                   // 0097556D
    w.word_6c = 0x00D1B368;                                  // 00975573
    if (subject.is_kind_5c(entity, 6)) {                     // 0097558B
        w.id = "confirmship";                                // 0097559A, 009755B3, 00D1B44C
        return w;
    }
    if (!subject.is_kind_5c(entity, 0x18)) return w;         // 009755C9
    w.id = "confirmplane";                                   // 009755DE, 00D1B43C
    if (!order.empty()) w.id += order;                       // 009755F7, 0097560B
    if (!subject.native_string_equals_ci_00425850(order, "land") &&      // 00975620, 00CFB614
        !subject.native_string_equals_ci_00425850(order, "retreat"))     // 00975633, 00CFB5D4
        return w;
    if (subject.field_3cc(entity) != 1) return w;            // 0097563C
    w.id += "player";                                        // 0097564E..00975683, 00D199D0
    return w;
}

void post_friendly_fire_warning_0096e8f0(WarningHitFeedbackState& state, float clock,
    WarningTextHost& host)
{
    state.text_state_180 = 2;
    state.text_deadline_17c = static_cast<float>(clock + kHitFeedbackWindow);
    host.post_text_00682990("ingame.warning_friendly", kFriendlyFireRgba, 2);
}

void report_hit_feedback_0096ef60(WarningHitFeedbackState& state, float clock, bool damaging,
    bool friendly, WarningTextHost& host)
{
    state.deadline_174 = static_cast<float>(clock + kHitFeedbackWindow); // 0096EF8E
    state.kind_178 = damaging ? 1 : 2;                                   // 0096EF88..0096EF9E
    if (friendly) {
        post_friendly_fire_warning_0096e8f0(state, clock, host);
        return;
    }
    if (state.text_state_180 == 0 && state.kind_178 == 2) {
        state.text_state_180 = 1;
        state.text_deadline_17c = static_cast<float>(kHitFeedbackWindow + clock);
        host.post_text_00682990("ingame.warning_ineffective", kIneffectiveFireRgba, 1);
    }
}

} // namespace bsp
