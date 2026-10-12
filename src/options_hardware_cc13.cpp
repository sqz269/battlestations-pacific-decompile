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
