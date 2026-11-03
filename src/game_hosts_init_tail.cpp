// bsp_game.exe milestone 2c: the Init tail. See include/bsp/game_hosts_init_tail.hpp
// for the address list and the evidence.
#include "bsp/game_hosts_init_tail.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/band_0090_cc13.hpp"
#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/hardware_options_seg66_argon.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/oct18_tail_c.hpp"
#include "bsp/vfs_mounts.hpp"
#include "bsp/wire_goal2_chain46_switches.hpp"
#include "bsp/wire_goal2_chain82_switches.hpp"
#include "bsp/wire_goal4_chain261_switches.hpp"
#include "bsp/wire_goal4_chain269_switches.hpp"

#include <d3d9.h>
#include <mmsystem.h>
#include <dsound.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>

namespace bsp::game {
namespace {

// 0098d4e0's key and access mask: RegOpenKeyExA(HKLM, "SOFTWARE\Eidos\BSM_HWD",
// 0x20019) for the reads, RegCreateKeyExA with 0x20006 for the writes.
constexpr char kHardwareRegistryKey[] = "SOFTWARE\\Eidos\\BSM_HWD";
constexpr DWORD kHardwareReadAccess = 0x20019u;
constexpr DWORD kHardwareWriteAccess = 0x20006u;
// 0098d5c0 demands type 0x0b (REG_QWORD) and keeps the low dword at 0098d5e2;
// 0098d5f0 demands type 1 (REG_SZ) into a 0x400-byte buffer.
constexpr DWORD kHardwareQwordType = 0x0bu;
constexpr std::size_t kHardwareStringCapacity = 0x400;
// True: read_stored_qword / read_stored_string call cc13's 0098d5c0 / 0098d5f0
// (both through 0098d4e0, returning its bool). False: the host's own reads, as before.
// Pair this on the read-only path only: never run a pair or test with
// --hardware-probe-commit, which writes this machine's real HKLM Eidos key.
constexpr bool kHardwareRegistryReadsCc13Bound = true;

// The read half of the probe, as bsp::probe_hardware_0073c3b0 wants it. The
// decision is not taken here: it belongs to bsp::run_hardware_probe_tail_0073c3b0,
// which models the native message, the two side effects and their ordering.
// src/app_bootstrap.cpp's own failure path joins its lines with no separator and
// models no write-back; see the corrections in docs/APP_INIT_TAIL.md.
class HardwareProbeReads final : public HardwareProbeHost {
public:
    explicit HardwareProbeReads(GameHardwareProbe& owner) : owner_(owner) {}
    bool read_hardware_qword(const std::string& value_name,
        std::uint32_t& out) const override {
        return owner_.read_stored_qword(value_name, out);
    }
    bool read_hardware_string(const std::string& value_name,
        std::string& out) const override {
        return owner_.read_stored_string(value_name, out);
    }
    std::vector<std::string> failed_requirements(const HardwareProfile& profile) const override {
        // Delegated: the four comparisons at 0073c493..0073c7a4 are
        // bsp::hardware_probe_changes_0073c3b0, which the tail runs next.
        static_cast<void>(profile);
        return {};
    }
    bool ask_write_default_options(const std::string& message) const override {
        static_cast<void>(message);
        return false;
    }
    void write_default_options_file(const std::string& path) const override {
        static_cast<void>(path);
    }

private:
    GameHardwareProbe& owner_;
};

namespace hardware = engine_seg11_35_66_cc13;

const hardware::GpuTableRow* probe_gpu_catalog() {
    static constexpr hardware::GpuTableRow rows[] = {
#include "bsp/wire_goal2_chain46_gpu_catalog.inc"
    };
    return rows;
}

class HardwareProbeMessages final
    : public hardware_options_seg66_argon::LocalizationMessageHost {
public:
    void* image_address(std::uint32_t address) override {
        if (address != 0x00d1e918u)
            throw std::runtime_error("hardware probe: unknown localization image address");
        // Producer: the binary's complete 9-message, 6-language pointer table.
        // Relocate its pointers to process-static copies of the UTF-16 strings.
        static const wchar_t* messages[] = {
#include "bsp/wire_goal4_chain261_messages.inc"
        };
        static_assert(sizeof messages / sizeof messages[0] == 9 * 6);
        return messages;
    }
    std::int32_t localization_language_row_for_lcid_0098d400(std::uint32_t lcid) override {
        return hardware::language_row_for_lcid_0098d400(lcid);
    }
};

std::wstring hardware_message_wide(const std::string& text, UINT code_page) {
    if (text.empty()) return {};
    const int length = static_cast<int>(text.size());
    const int count = MultiByteToWideChar(code_page, 0, text.data(), length, nullptr, 0);
    if (count == 0) throw std::runtime_error("hardware probe: message conversion failed");
    std::wstring wide(static_cast<std::size_t>(count), L'\0');
    if (MultiByteToWideChar(code_page, 0, text.data(), length, wide.data(), count) != count)
        throw std::runtime_error("hardware probe: message conversion failed");
    return wide;
}

std::string hardware_probe_localized_message(std::uint32_t language, std::int32_t index,
    const std::string& first, const std::string& second) {
    HardwareProbeMessages messages;
    // 0073C3B0 requests rows 2..7; 00996060 is __fastcall(LCID, message index).
    // The two string arguments originate in the stored/detected GPU or sound
    // names. The numeric arguments are the tail's decimal serialization of the
    // stored/detected uint32 values, restored before the native %d formatting.
    const wchar_t* format = hardware_options_seg66_argon::
        localization_already_running_message_00996060(messages, language, index);
    wchar_t buffer[0x1000]{}; // 00735450's capacity at each 0073C3B0 call site.
    int written = 0;
    if (index == 2 || index == 3) {
        const auto a = hardware_message_wide(first, CP_ACP);
        const auto b = hardware_message_wide(second, CP_ACP);
        written = _swprintf_p(buffer, 0x1000, format, a.c_str(), b.c_str());
    } else if (index == 4 || index == 5) {
        written = _swprintf_p(buffer, 0x1000, format,
            static_cast<int>(std::strtoul(first.c_str(), nullptr, 10)),
            static_cast<int>(std::strtoul(second.c_str(), nullptr, 10)));
    } else {
        written = _swprintf_p(buffer, 0x1000, format);
    }
    if (written < 0) throw std::runtime_error("hardware probe: message formatting failed");
    if (written == 0) return {};
    // The probe-tail interface transports strings as UTF-8; restore UTF-16 at
    // MessageBoxW rather than narrowing any of the catalog's six languages.
    const int count = WideCharToMultiByte(CP_UTF8, 0, buffer, written, nullptr, 0, nullptr, nullptr);
    if (count == 0) throw std::runtime_error("hardware probe: message encoding failed");
    std::string text(static_cast<std::size_t>(count), '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, buffer, written, text.data(), count, nullptr, nullptr) != count)
        throw std::runtime_error("hardware probe: message encoding failed");
    return text;
}

// Native lifetime: 0073C467 constructs after the four registry reads succeed;
// 0073C8A3..0073C8DD releases its vectors on leaving the probe. Producers below
// own every read field.
// The existing collector uses a borrowed typed view; the image offsets remain
// explicit for the constructor's two vector headers and its LCID at +C38h.
struct ProbeHardwareImage {
    hardware::HardwareInfoVectors vectors_00;
    std::uint32_t unused_20;
    std::int32_t shader_model_24;
    hardware::HardwareInfoFields fields_28;
    char description_a34[0x100];
    char driver_b34[0x100];
    bool nt5_c34;
    std::uint8_t padding_c35[3];
    void* language_c38;
    // The native caller owns a 4C3Ch hardware object (also retained by
    // cc13_pkt642.cpp). 0098E910 writes its text into the final 4000h bytes.
    char default_options_c3c[0x4000];

    std::int32_t options_detail() const {
        band0090::HardwareSnapshot state;
        state.gpu_row = fields_28.gpu_index_28;
        state.memory_mb = fields_28.memory_mb_2c;
        state.cpu_mhz = fields_28.cpu_mhz_30;
        std::vector<std::uint32_t> scores;
        const auto* const catalog = probe_gpu_catalog();
        for (const auto* row = catalog; row->name != nullptr; ++row)
            scores.push_back(row->score);
        // 0098DFB4..0098DFE7: an unknown GPU selects catalog row zero,
        // conditional on +20h. That word has no producer in 00992640 or
        // 009927D0. The installed catalog's row-zero score is zero, so BOTH
        // outcomes return zero. Normalize to that actual fallback row; no
        // invented +20h value is consumed by the existing detail body.
        if (state.gpu_row == -1) {
            if (scores.at(0) != 0)
                throw std::runtime_error("hardware probe: unknown-GPU +20h has no producer");
            state.gpu_row = 0;
        }
        return band0090::recommend_detail_level(state, scores);
    }
    std::uint8_t options_supports(std::uint32_t width, std::uint32_t height) const {
        band0090::HardwareSnapshot state;
        const auto* const first = reinterpret_cast<const std::pair<std::uint32_t, std::uint32_t>*>(
            static_cast<std::uintptr_t>(vectors_00.words[1]));
        const std::uint32_t bytes = vectors_00.words[2] - vectors_00.words[1];
        for (std::uint32_t i = 0; i < bytes / 8; ++i)
            state.display_modes.emplace_back(first[i].first, first[i].second);
        return band0090::supports_display_mode(state, static_cast<std::int32_t>(width),
            static_cast<std::int32_t>(height)) ? 1 : 0;
    }
};
static_assert(offsetof(ProbeHardwareImage, fields_28) == 0x28);
static_assert(offsetof(ProbeHardwareImage, fields_28)
    + offsetof(hardware::HardwareInfoFields, memory_mb_2c) == 0x2c);
static_assert(offsetof(ProbeHardwareImage, fields_28)
    + offsetof(hardware::HardwareInfoFields, cpu_mhz_30) == 0x30);
static_assert(offsetof(ProbeHardwareImage, fields_28)
    + offsetof(hardware::HardwareInfoFields, cpu_name_34) == 0x34);
static_assert(offsetof(ProbeHardwareImage, description_a34) == 0xa34);
static_assert(offsetof(ProbeHardwareImage, driver_b34) == 0xb34);
static_assert(offsetof(ProbeHardwareImage, nt5_c34) == 0xc34);
static_assert(offsetof(ProbeHardwareImage, language_c38) == 0xc38);
static_assert(offsetof(ProbeHardwareImage, default_options_c3c) == 0xc3c);
static_assert(sizeof(ProbeHardwareImage) == 0x4c3c);

// MSVC Win32 single-class member pointers are one code pointer. These two
// nonvirtual members have exactly the bindings' __thiscall ABI: ECX is the
// actual image, with zero / two stack arguments and RET / RET 8. Copy the
// representation without a mismatched-function-type cast (C4191).
template <class Callback, class Method>
Callback probe_hardware_callback(Method method) {
    static_assert(sizeof(Callback) == sizeof(Method));
    Callback callback;
    std::memcpy(&callback, &method, sizeof(callback));
    return callback;
}

// Adapts the Windows survey to the existing constructor/collector and the
// probe tail. No hardware state is constructed until the chain's ON arm runs.
class HardwareProbeMachine final : public hardware::HardwareCollectCalls,
    public hardware::OsVersionCalls, public hardware::DisplayModeList,
    public hardware::DirectSoundProbe, public HardwareProbeTailHost,
    public hardware::OptionsTextBuilder {
public:
    HardwareProbeMachine(GameHardwareProbe& owner, GameHostLog& log) : owner_(owner), log_(log) {}
    ~HardwareProbeMachine() override {
        if (sound_module_ != nullptr) FreeLibrary(sound_module_);
    }
    void construct(std::uint32_t language) {
        hardware::construct_hardware_info_009927d0(
            hardware::HardwareInfoConstructView{image_.vectors_00, image_.language_c38,
                image_.fields_28}, language, *this);
    }

    void describe_os_version_0098c9c0() override {
        std::string text;
        hardware::describe_os_version_0098c9c0(text, image_.nt5_c34, *this);
        // 0098C9C0's destination is +234h, within the CPU-name backing region.
        std::strcpy(image_.fields_28.cpu_name_34 + 0x200, text.c_str());
    }
    bool get_version_ex(std::uint32_t size, hardware::OsVersionInfoEx& out) override {
        OSVERSIONINFOEXA version{};
        version.dwOSVersionInfoSize = size;
        using GetVersion = BOOL(WINAPI*)(LPOSVERSIONINFOA);
        const auto get_version = reinterpret_cast<GetVersion>(
            GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetVersionExA"));
        if (get_version == nullptr || !get_version(reinterpret_cast<LPOSVERSIONINFOA>(&version)))
            return false;
        out = {version.dwMajorVersion, version.dwMinorVersion, version.dwBuildNumber,
            version.dwPlatformId, version.szCSDVersion, version.wSuiteMask, version.wProductType};
        return true;
    }
    bool reg_open_hklm(const char* path) override {
        return RegOpenKeyExA(HKEY_LOCAL_MACHINE, path, 0, KEY_QUERY_VALUE, &os_key_) == ERROR_SUCCESS;
    }
    bool reg_query_string(const char* name, std::string& value, std::uint32_t& length) override {
        char bytes[0x50]{};
        DWORD size = (std::min)(static_cast<DWORD>(length), static_cast<DWORD>(sizeof bytes));
        DWORD type = 0;
        const LSTATUS status = RegQueryValueExA(os_key_, name, nullptr, &type,
            reinterpret_cast<BYTE*>(bytes), &size);
        length = size;
        if (status != ERROR_SUCCESS) return false;
        value.assign(bytes, strnlen(bytes, sizeof bytes));
        return true;
    }
    void reg_close() override { RegCloseKey(os_key_); os_key_ = nullptr; }
    std::uint32_t total_physical_memory() override {
        MEMORYSTATUS status{};
        status.dwLength = sizeof status;
        GlobalMemoryStatus(&status);
        return status.dwTotalPhys;
    }
    bool query_processor_info_00992180() override {
        return hardware::query_processor_info_00992180(image_.fields_28);
    }
    std::uint32_t measure_cpu_clock_0098c940() override {
        return hardware::measure_cpu_clock_0098c940();
    }
    void query_display_driver_00991ef0() override {
        struct D3DOwner {
            IDirect3D9* api{Direct3DCreate9(D3D_SDK_VERSION)};
            ~D3DOwner() { if (api != nullptr) api->Release(); }
        } d3d;
        D3DADAPTER_IDENTIFIER9 id{};
        D3DCAPS9 caps{};
        if (d3d.api == nullptr
            || FAILED(d3d.api->GetAdapterIdentifier(0, 0, &id))
            || FAILED(d3d.api->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps)))
            throw std::runtime_error("hardware probe: D3D9 adapter survey unavailable");
        hardware::D3DAdapterSurvey survey{};
        survey.driver_version_low = id.DriverVersion.LowPart;
        survey.driver_version_high = static_cast<std::uint32_t>(id.DriverVersion.HighPart);
        survey.description = id.Description;
        survey.device_id = id.DeviceId;
        survey.pixel_shader_version = caps.PixelShaderVersion;
        const UINT count = d3d.api->GetAdapterModeCount(0, D3DFMT_X8R8G8B8);
        for (UINT i = 0; i < count; ++i) {
            D3DDISPLAYMODE mode{};
            if (SUCCEEDED(d3d.api->EnumAdapterModes(0, D3DFMT_X8R8G8B8, i, &mode)))
                survey.modes_x8r8g8b8.emplace_back(mode.Width, mode.Height);
        }
        survey.multisample_ok.resize(16);
        for (unsigned i = 2; i < 16; ++i) {
            DWORD quality = 0;
            survey.multisample_ok[i] = SUCCEEDED(d3d.api->CheckDeviceMultiSampleType(0,
                D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, FALSE, static_cast<D3DMULTISAMPLE_TYPE>(i), &quality));
        }
        hardware::query_display_driver_00991ef0(survey, gpu_catalog(),
            {&image_.shader_model_24, &image_.fields_28.gpu_index_28,
                image_.description_a34, image_.driver_b34}, *this);
    }
    bool contains_0098e0e0(std::uint32_t width, std::uint32_t height) override {
        return std::find(modes_.begin(), modes_.end(), std::make_pair(width, height)) != modes_.end();
    }
    void append_00991c20(std::uint32_t width, std::uint32_t height) override {
        modes_.emplace_back(width, height);
        publish_vector(image_.vectors_00.words + 1, modes_);
    }
    void append_multisample_type(std::int32_t type) override {
        samples_.push_back(type);
        publish_vector(image_.vectors_00.words + 5, samples_);
    }
    void reset_sound_probe(std::uint32_t& counter, char& first) override {
        sound_count_ = counter;
        sound_name_[0] = first;
    }
    void enumerate_sound_devices() override {
        sound_module_ = LoadLibraryA("dsound.dll");
        using Enumerate = HRESULT(WINAPI*)(LPDSENUMCALLBACKA, LPVOID);
        const auto enumerate = sound_module_ == nullptr ? nullptr : reinterpret_cast<Enumerate>(
            GetProcAddress(sound_module_, "DirectSoundEnumerateA"));
        if (enumerate == nullptr)
            throw std::runtime_error("hardware probe: DirectSound enumeration unavailable");
        enumerate(&sound_callback, this);
    }
    bool create_default_device() override {
        using Create = HRESULT(WINAPI*)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN);
        const auto create = reinterpret_cast<Create>(GetProcAddress(sound_module_, "DirectSoundCreate"));
        if (create == nullptr) return false;
        LPDIRECTSOUND device = nullptr;
        const HRESULT status = create(nullptr, &device, nullptr);
        if (SUCCEEDED(status)) device->Release();
        return SUCCEEDED(status);
    }
    std::uint32_t current_gpu_device_id() override {
        log_.implemented("HardwareProbe::current_gpu_device_id", "0098c870");
        return hardware::gpu_device_id_0098c870(image_.fields_28, gpu_catalog());
    }
    std::string gpu_device_name(std::uint32_t id) override {
        return hardware::gpu_device_name_0098c890(id, gpu_catalog());
    }
    std::string current_sound_device() override { return sound_name_; }
    std::uint32_t current_cpu_speed() override { return image_.fields_28.cpu_mhz_30; }
    std::uint32_t current_memory_size() override { return image_.fields_28.memory_mb_2c; }
    std::string localized_message(std::int32_t index, const std::string& a,
        const std::string& b) override {
        if constexpr (kWireGoal4Chain261Bound) {
            // 0073C467's detection object owns the LCID until the probe returns.
            return hardware_probe_localized_message(static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(image_.language_c38)), index, a, b);
        } else {
            return owner_.localized_message(index, a, b);
        }
    }
    bool ask_write_default_options(const std::string& text, const std::string& caption) override {
        return owner_.ask_write_default_options(text, caption);
    }
    void write_default_options_file(const std::string& path) override {
        if constexpr (kWireGoal4Chain269Bound) {
            // 0073C83E reaches this only after the real probe dialog's IDYES.
            // 0098F430 opens first, then calls 0098E910 twice (strlen/write).
            // Keep the same live detection owner through both formatter calls.
            const bool written = hardware::write_default_options_file_0098f430(path.c_str(), *this);
            log_.implemented("HardwareProbe::write_default_options_file", "0098f430");
            log_.notef("hardware default options path=%s written=%d", path.c_str(), written ? 1 : 0);
        } else {
            owner_.write_default_options_file(path);
        }
    }
    const char* build_default_options_text_0098e910() override {
        using Bindings = oct18_tail_c::HardwareOptionsBindings;
        const Bindings bindings{
            probe_hardware_callback<decltype(Bindings::detail_0098df90)>(
                &ProbeHardwareImage::options_detail),
            probe_hardware_callback<decltype(Bindings::supports_0098e0e0)>(
                &ProbeHardwareImage::options_supports),
            [] { throw std::runtime_error("hardware probe: invalid antialias iterator"); }};
        return oct18_tail_c::hardware_default_options_0098e910(&image_, sound_count_, bindings);
    }
    void write_hardware_dword(const std::string& name, std::uint32_t value) override {
        owner_.write_hardware_dword(name, value);
    }
    void write_hardware_string(const std::string& name, const std::string& value) override {
        owner_.write_hardware_string(name, value);
    }
private:
    static const hardware::GpuTableRow* gpu_catalog() {
        // Producer: static image table 00E0CFF4 and its pointed-to strings.
        static constexpr hardware::GpuTableRow rows[] = {
#include "bsp/wire_goal2_chain46_gpu_catalog.inc"
        };
        return rows;
    }
    template<class T> static void publish_vector(std::uint32_t* header, std::vector<T>& values) {
        header[0] = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(values.data()));
        header[1] = header[0] + static_cast<std::uint32_t>(values.size() * sizeof(T));
        header[2] = header[0] + static_cast<std::uint32_t>(values.capacity() * sizeof(T));
    }
    static BOOL CALLBACK sound_callback(LPGUID, LPCSTR description, LPCSTR, LPVOID context) {
        auto& self = *static_cast<HardwareProbeMachine*>(context);
        return hardware::direct_sound_enum_first_0098d360(description, self.sound_count_,
            self.sound_name_, self) ? TRUE : FALSE;
    }
    GameHardwareProbe& owner_;
    GameHostLog& log_;
    ProbeHardwareImage image_{};
    std::vector<std::pair<std::uint32_t, std::uint32_t>> modes_;
    std::vector<std::int32_t> samples_;
    HKEY os_key_{};
    HMODULE sound_module_{};
    std::uint32_t sound_count_{};
    char sound_name_[0x100]{}; // 00F8A0E8, reset by 00992640 before enumeration.
};

}  // namespace

// ---------------------------------------------------------------------------
// GameHardwareProbe, 0073c3b0
// ---------------------------------------------------------------------------

GameHardwareProbe::GameHardwareProbe(GameHostLog& log, bool commit)
    : log_(log), commit_(commit) {
    summary_.commit_enabled = commit;
}

bool GameHardwareProbe::read_stored_qword(const std::string& value_name,
    std::uint32_t& out) const {
    if constexpr (kHardwareRegistryReadsCc13Bound) {
        // 0098d5e6 stores the low dword whether or not the read succeeded; seeding
        // the buffer with out keeps a failed read from changing it, as the host does.
        return engine_seg11_35_66_cc13::read_hardware_qword_0098d5c0(value_name.c_str(), out, out);
    } else {
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, kHardwareRegistryKey, 0, kHardwareReadAccess, &key)
        != ERROR_SUCCESS) {
        return false;
    }
    DWORD type = 0;
    std::uint64_t value = 0;
    DWORD size = sizeof(value);
    const LSTATUS status = RegQueryValueExA(key, value_name.c_str(), nullptr, &type,
        reinterpret_cast<LPBYTE>(&value), &size);
    RegCloseKey(key);
    // 0098d4e0 returns true only when the call succeeded and the type matches
    // the one in EDX; 0098d5e2 then keeps the low dword of the eight bytes.
    if (status != ERROR_SUCCESS || type != kHardwareQwordType) return false;
    out = static_cast<std::uint32_t>(value & 0xffffffffu);
    return true;
    }
}

bool GameHardwareProbe::read_stored_string(const std::string& value_name,
    std::string& out) const {
    if constexpr (kHardwareRegistryReadsCc13Bound) {
        char buffer[kHardwareStringCapacity] = {};
        std::uint32_t size = static_cast<std::uint32_t>(sizeof(buffer));
        if (!engine_seg11_35_66_cc13::read_hardware_string_0098d5f0(value_name.c_str(), buffer, &size)) return false;
        buffer[sizeof(buffer) - 1] = '\0';
        out.assign(buffer);
        return true;
    } else {
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, kHardwareRegistryKey, 0, kHardwareReadAccess, &key)
        != ERROR_SUCCESS) {
        return false;
    }
    DWORD type = 0;
    char buffer[kHardwareStringCapacity] = {};
    DWORD size = static_cast<DWORD>(sizeof(buffer));
    const LSTATUS status = RegQueryValueExA(key, value_name.c_str(), nullptr, &type,
        reinterpret_cast<LPBYTE>(buffer), &size);
    RegCloseKey(key);
    if (status != ERROR_SUCCESS || type != REG_SZ) return false;
    buffer[sizeof(buffer) - 1] = '\0';
    out.assign(buffer);
    return true;
    }
}

std::uint32_t GameHardwareProbe::current_gpu_device_id() {
    log_.unimplemented("HardwareProbe::current_gpu_device_id", "0098c870");
    return 0;
}

std::string GameHardwareProbe::gpu_device_name(std::uint32_t id) {
    // 0098c890 calls gpu_device_name_0098c890; OFF keeps the unimplemented lookup.
    if constexpr (kWireGoal2Chain46Bound) {
        // The current-device producer is still unimplemented and returns zero.
        // Only the nonzero stored GPUDeviceID admitted by run()'s profile gate
        // supplies real state (0073c49f -> CALL 0073c4b4); keep the synthetic
        // current-device call unchanged until detection owns its adapter index.
        if (id == 0) {
            static_cast<void>(id);
            log_.unimplemented("HardwareProbe::gpu_device_name", "0098c890");
            return {};
        }
        using engine_seg11_35_66_cc13::GpuTableRow;
        static_assert(sizeof(GpuTableRow) == 12);
        static_assert(offsetof(GpuTableRow, device_id) == 0);
        static_assert(offsetof(GpuTableRow, name) == 4);
        static_assert(offsetof(GpuTableRow, score) == 8);
        // Producer: the original image's static 00e0cff4 catalog and pointed-to
        // strings. Native lifetime is process-static, present before 0073c3b0;
        // this immutable owner is materialized only in the ON arm, at first use.
        static constexpr GpuTableRow gpu_catalog[] = {
#include "bsp/wire_goal2_chain46_gpu_catalog.inc"
        };
        log_.implemented("HardwareProbe::gpu_device_name", "0098c890");
        return engine_seg11_35_66_cc13::gpu_device_name_0098c890(id, gpu_catalog);
    } else {
    static_cast<void>(id);
    log_.unimplemented("HardwareProbe::gpu_device_name", "0098c890");
    return {};
    }
}

std::string GameHardwareProbe::current_sound_device() {
    // 0098d3d0 is two instructions, MOV EAX,00f8a0e8 and RET: the detected name
    // is a process-wide buffer the detection object 009927d0 fills.
    log_.unimplemented("HardwareProbe::current_sound_device", "0098d3d0");
    return {};
}

std::uint32_t GameHardwareProbe::current_cpu_speed() {
    log_.unimplemented("HardwareProbe::current_cpu_speed", "0098c7f0");
    return 0;
}

std::uint32_t GameHardwareProbe::current_memory_size() {
    log_.unimplemented("HardwareProbe::current_memory_size", "0098c800");
    return 0;
}

std::string GameHardwareProbe::localized_message(std::int32_t index, const std::string& first,
    const std::string& second) {
    // 00996060 indexes the flat wide table at 00d1e918 as [language + index * 6]
    // and 00735450 formats it with _vswprintf_p_l. Neither the table nor the
    // language row is reconstructed, and phase 2 runs before the locale tables.
    static_cast<void>(first);
    static_cast<void>(second);
    log_.unimplemented("HardwareProbe::localized_message", "00996060");
    log_.notef("hardware probe message %d requested", index);
    return {};
}

bool GameHardwareProbe::ask_write_default_options(const std::string& text,
    const std::string& caption) {
    // 0073c827 is MessageBoxW(NULL, text, caption, MB_YESNO | MB_ICONEXCLAMATION).
    // A frame-limited run has no operator, so the dialog is only raised when the
    // run asked for it; otherwise the answer is the decline branch. This is the
    // milestone's decision, not recovered behaviour.
    log_.implemented("HardwareProbe::ask_write_default_options", "0073c827");
    if (!commit_) {
        log_.note("hardware probe dialog suppressed (no --hardware-probe-commit); "
            "taking the IDNO branch");
        return false;
    }
    if constexpr (kWireGoal4Chain261Bound) {
        const auto wide_text = hardware_message_wide(text, CP_UTF8);
        const auto wide_caption = hardware_message_wide(caption, CP_UTF8);
        return MessageBoxW(nullptr, wide_text.c_str(), wide_caption.c_str(),
            MB_YESNO | MB_ICONEXCLAMATION) == IDYES;
    } else {
    const std::wstring wide_text(text.begin(), text.end());
    const std::wstring wide_caption(caption.begin(), caption.end());
    return MessageBoxW(nullptr, wide_text.c_str(), wide_caption.c_str(),
        MB_YESNO | MB_ICONEXCLAMATION) == IDYES;
    }
}

void GameHardwareProbe::write_default_options_file(const std::string& path) {
    // 0098f430 is __thiscall on the detection object: fopen(path, "wb") plus the
    // defaults that object generates. The object is packet
    // `hardware_detection_object`, so there is nothing to write.
    static_cast<void>(path);
    log_.unimplemented("HardwareProbe::write_default_options_file", "0098f430");
}

void GameHardwareProbe::write_hardware_dword(const std::string& value_name,
    std::uint32_t value) {
    log_.implemented("HardwareProbe::write_hardware_dword", "0098d570");
    if (!commit_) {
        log_.notef("hardware write-back %s = %lu suppressed (no --hardware-probe-commit)",
            value_name.c_str(), static_cast<unsigned long>(value));
        return;
    }
    if constexpr (kWireGoal4Chain261Bound) {
        // 0073C856/86E/886 pass the detected MemSize/CPUSpeed/GPUDeviceID.
        // 0098D570 -> 0098D470 owns the sole write and key-close on this arm.
        hardware::write_hardware_dword_0098d570(value_name.c_str(), value);
    } else {
    HKEY key = nullptr;
    DWORD disposition = 0;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, kHardwareRegistryKey, 0, nullptr, 0,
        kHardwareWriteAccess, nullptr, &key, &disposition) != ERROR_SUCCESS) {
        log_.notef("hardware write-back %s failed to open the key", value_name.c_str());
        return;
    }
    // 0098d570 writes four bytes typed REG_QWORD (0098d571 PUSH 4, 0098d57c MOV
    // EDX,0Bh into 0098d470's RegSetValueExA dwType), so 0098d5c0's REG_QWORD
    // read accepts its own write-back.
    RegSetValueExA(key, value_name.c_str(), 0, REG_QWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));
    RegCloseKey(key);
    }
}

void GameHardwareProbe::write_hardware_string(const std::string& value_name,
    const std::string& value) {
    log_.implemented("HardwareProbe::write_hardware_string", "0098d590");
    if (!commit_) {
        log_.notef("hardware write-back %s = \"%s\" suppressed (no --hardware-probe-commit)",
            value_name.c_str(), value.c_str());
        return;
    }
    if constexpr (kWireGoal4Chain261Bound) {
        // 0073C89E supplies the detected DirectSound name. 0098D590 retains
        // the image's REG_SZ strlen+1 convention; the host write is replaced.
        hardware::write_hardware_string_0098d590(value_name.c_str(), value.c_str());
    } else {
    HKEY key = nullptr;
    DWORD disposition = 0;
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, kHardwareRegistryKey, 0, nullptr, 0,
        kHardwareWriteAccess, nullptr, &key, &disposition) != ERROR_SUCCESS) {
        log_.notef("hardware write-back %s failed to open the key", value_name.c_str());
        return;
    }
    // 0098d590 writes strlen + 1 bytes.
    RegSetValueExA(key, value_name.c_str(), 0, REG_SZ,
        reinterpret_cast<const BYTE*>(value.c_str()),
        static_cast<DWORD>(value.size() + 1));
    RegCloseKey(key);
    }
}

void GameHardwareProbe::run() {
    summary_.ran = true;
    HardwareProbeReads reads(*this);
    HardwareProfile profile{};
    const HardwareProbeResult gate = probe_hardware_0073c3b0(reads, profile);
    summary_.stored = profile;
    summary_.stored_values_read = (profile.cpu_speed != 0 ? 1 : 0)
        + (profile.mem_size != 0 ? 1 : 0) + (profile.gpu_device_id != 0 ? 1 : 0)
        + (profile.sound_device.empty() ? 0 : 1);
    if (gate == HardwareProbeResult::profile_incomplete) {
        // 0073c443's all-four gate: a single missing or mistyped value skips the
        // whole comparison silently, so the machine is never inspected.
        summary_.result = HardwareProbeResult::profile_incomplete;
        log_.notef("hardware profile incomplete under HKLM\\%s; the probe returns "
            "without comparing anything", kHardwareRegistryKey);
        return;
    }
    summary_.profile_complete = true;
    // 009927D0/0098C870 construct/read the survey; OFF keeps the host's accessor stand-ins.
    if constexpr (kWireGoal2Chain82Bound) {
        HardwareProbeMachine machine(*this, log_);
        machine.construct(hardware::read_language_from_options_00992830());
        log_.implemented("HardwareProbe::construct_detection_object", "009927d0");
        HardwareProbeInputs inputs;
        inputs.stored = profile;
        inputs.current.gpu_device_id = machine.current_gpu_device_id();
        inputs.current.sound_device = machine.current_sound_device();
        inputs.current.cpu_speed = machine.current_cpu_speed();
        inputs.current.mem_size = machine.current_memory_size();
        summary_.changes = hardware_probe_changes_0073c3b0(inputs).size();
        summary_.result = run_hardware_probe_tail_0073c3b0(profile, machine);
        log_.notef("hardware probe stored gpu=%lu cpu=%lu mem=%lu sound=\"%s\" changes=%zu "
            "result=%d", static_cast<unsigned long>(profile.gpu_device_id),
            static_cast<unsigned long>(profile.cpu_speed),
            static_cast<unsigned long>(profile.mem_size), profile.sound_device.c_str(),
            summary_.changes, static_cast<int>(summary_.result));
    } else {
    HardwareProbeInputs inputs;
    inputs.stored = profile;
    inputs.current.gpu_device_id = current_gpu_device_id();
    inputs.current.sound_device = current_sound_device();
    inputs.current.cpu_speed = current_cpu_speed();
    inputs.current.mem_size = current_memory_size();
    summary_.changes = hardware_probe_changes_0073c3b0(inputs).size();
    summary_.result = run_hardware_probe_tail_0073c3b0(profile, *this);
    log_.notef("hardware probe stored gpu=%lu cpu=%lu mem=%lu sound=\"%s\" changes=%zu "
        "result=%d", static_cast<unsigned long>(profile.gpu_device_id),
        static_cast<unsigned long>(profile.cpu_speed),
        static_cast<unsigned long>(profile.mem_size), profile.sound_device.c_str(),
        summary_.changes, static_cast<int>(summary_.result));
    }
}

// ---------------------------------------------------------------------------
// GameStartupSingletonPublication, 0073fa70 and 00af06a0
// ---------------------------------------------------------------------------

GameStartupSingletonPublication::GameStartupSingletonPublication(GameHostLog& log,
    const StartupSingletonPublication& record, const char* label)
    : log_(log), record_(record), label_(label) {}

void GameStartupSingletonPublication::enter_lifetime_lock() {
    // 00415350 then the singleton lifetime manager's critical section at +10h,
    // with the depth at +18h. The section itself is the manager's, not ours.
    ++depth_;
    log_.implemented("StartupSingleton::enter_lifetime_lock", "00415350");
}

void GameStartupSingletonPublication::register_lifetime(void* instance) {
    // 00bd0c30. The native re-reads the publication global it has just written
    // rather than using the register it still holds.
    static_cast<void>(instance);
    log_.unimplemented("StartupSingleton::register_lifetime", "00bd0c30");
}

void GameStartupSingletonPublication::leave_lifetime_lock() {
    if (depth_ > 0) --depth_;
    log_.implemented("StartupSingleton::leave_lifetime_lock", "00415350+leave");
}

void GameStartupSingletonPublication::publish(void* instance) {
    published_ = instance;
    log_.implemented("StartupSingleton::publish", "0073fac6");
    log_.notef("startup singleton %s published at %08lx (base vtable %08lx)",
        label_ != nullptr ? label_ : "(unnamed)",
        static_cast<unsigned long>(record_.instance_global),
        static_cast<unsigned long>(record_.base_vtable));
}

// ---------------------------------------------------------------------------
// GameDecalTable, 00740840
// ---------------------------------------------------------------------------

GameDecalTable::GameDecalTable(GameHostLog& log, GameVfsHost& vfs) : log_(log), vfs_(vfs) {}
GameDecalTable::~GameDecalTable() = default;

void GameDecalTable::run() {
    // 0073fa70 first: the base vtable and the 00e1aea0 publication. The derived
    // vtable 00cff2a0 that 00740872 installs is packet `decal_system_runtime`.
    GameStartupSingletonPublication publication(log_, kDecalSystemBase_0073fa70,
        "decal system");
    publish_startup_singleton_00af06a0(publication, this);
    summary_.published = publication.published() == this;

    // 00740903 runs the 29-byte path scripts/datatables/decals.lua through
    // 00b69d40, which reads it out of the VFS: 00bdf4c0 resolves the name and
    // 00bdf310 opens it into memory, the same pair every other asset takes.
    std::string text;
    {
        auto* manager = vfs_.ready() ? &vfs_.context() : nullptr;
        std::string resolved = kDecalTablePath;
        if (manager == nullptr
            || !resolve_existing_resource_00bdf4c0_fragment(*manager,
                vfs_.search_registrations(), resolved)) {
            summary_.error = "cannot resolve the decal table";
            log_.notef("decal table %s not resolved", kDecalTablePath);
            log_.unimplemented("Phase 9 decal_definitions", "00740840");
            return;
        }
        VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(*manager,
            resolved, 2);
        if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
            summary_.error = opened.error.empty() ? "cannot open the decal table" : opened.error;
            log_.notef("decal table %s not read: %s", kDecalTablePath, summary_.error.c_str());
            log_.unimplemented("Phase 9 decal_definitions", "00740840");
            return;
        }
        text.assign(reinterpret_cast<const char*>(opened.stream->data_00bef610()),
            static_cast<std::size_t>(opened.stream->size_00bef600()));
    }
    summary_.script_read = true;

    // 007408a0 builds the loader's own Lua state owner (00b66bd0) and 007408b6
    // opens it with library mask 1 (00b6a020). The repository's stock Lua 5.1.1
    // stands in for the native interpreter, as src/decal_table_probe.cpp does.
    GuiLua51Host lua;
    log_.implemented("DecalTable::construct_lua_state_owner", "007408a0");
    log_.implemented("DecalTable::open_lua_libraries", "007408b6");
    std::string error;
    if (!lua.execute_archive(text, error)) {
        summary_.error = error;
        log_.notef("decals.lua did not run: %s", error.c_str());
        log_.unimplemented("Phase 9 decal_definitions", "00740840");
        return;
    }
    summary_.script_ran = true;
    log_.implemented("DecalTable::run_script", "00740903");

    // 0109EEA4, the CRT's double-to-int selector, queried rather than assumed.
    const bool crt_sse2_conversion =
        IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE;
    definitions_ = load_decal_definitions_00740840(lua, crt_sse2_conversion);
    summary_.definitions = definitions_.size();
    log_.implemented("Phase 9 decal_definitions", "00740840");
    for (const DecalDefinition& record : definitions_) {
        // 00740454 loads decal.mvfm once per record through renderer vtable +34h,
        // 00740b90 the texture through +64h and 00740c50 the shader through +48h.
        // The renderer at 00f8d394 is another owner's, so the three loads record
        // the names the table authored instead of resolving them.
        ++summary_.textures_requested;
        ++summary_.shaders_requested;
        log_.notef("decal %-20s size=%.4f radius=%.4f maxnum=%d texture=%s shader=%s "
            "life=%.4f fade=%.4f", record.name.c_str(),
            static_cast<double>(record.size), static_cast<double>(record.radius),
            record.max_count, record.texture_name.c_str(), record.shader_name.c_str(),
            static_cast<double>(record.life_time),
            static_cast<double>(record.fade_out_time));
    }
    if (!definitions_.empty()) {
        log_.unimplemented("DecalTable::load_vertex_format", "00740454");
        log_.unimplemented("DecalTable::load_texture", "00740b90");
        log_.unimplemented("DecalTable::load_shader", "00740c50");
    }
    log_.notef("decal definitions parsed: %zu", definitions_.size());
}

}  // namespace bsp::game
