#pragma once

#include "bsp/app_bootstrap.hpp"

#include <cstdint>
#include <vector>

struct IDirect3D9;

namespace bsp {

// Only the renderer state consumed/produced by the recovered settings queries.
// This is a C++ ownership model, not the original renderer layout.
struct SettingsRendererCapabilities {
    std::vector<Resolution> resolutions;       // renderer+1Ch
    std::vector<int> antialias_levels;          // renderer+28h
    std::uint32_t adapter_mode_state_19dc = 0;  // meaning unresolved; enumeration clears it
    std::uint32_t pixel_shader_version_28 = 0;  // record+28h / renderer+1B40h
    int max_shader_model = 0;                  // renderer+1B48h; native values 1 or 2
};

struct SettingsAdapterMode {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

// D3D9 calls exposed without requiring a device or Windows headers in the
// recovered operations. HRESULTs retain their signed 32-bit representation.
class SettingsCapabilityQueries {
public:
    virtual ~SettingsCapabilityQueries() = default;
    virtual std::uint32_t adapter_mode_count(std::uint32_t adapter,
        std::uint32_t format) = 0;
    virtual std::int32_t enum_adapter_mode(std::uint32_t adapter,
        std::uint32_t format, std::uint32_t index, SettingsAdapterMode& mode) = 0;
    virtual std::int32_t device_pixel_shader_version(std::uint32_t adapter,
        std::uint32_t device_type, std::uint32_t& version) = 0;
    virtual std::int32_t check_multisample_type(std::uint32_t adapter,
        std::uint32_t device_type, std::uint32_t surface_format, bool windowed,
        std::uint32_t samples, std::uint32_t& quality_levels) = 0;
};

// Borrows the same IDirect3D9 interface as the renderer; does not AddRef/Release.
class Win32SettingsCapabilityQueries final : public SettingsCapabilityQueries {
public:
    explicit Win32SettingsCapabilityQueries(IDirect3D9& api) : api_(api) {}
    std::uint32_t adapter_mode_count(std::uint32_t adapter,
        std::uint32_t format) override;
    std::int32_t enum_adapter_mode(std::uint32_t adapter, std::uint32_t format,
        std::uint32_t index, SettingsAdapterMode& mode) override;
    std::int32_t device_pixel_shader_version(std::uint32_t adapter,
        std::uint32_t device_type, std::uint32_t& version) override;
    std::int32_t check_multisample_type(std::uint32_t adapter,
        std::uint32_t device_type, std::uint32_t surface_format, bool windowed,
        std::uint32_t samples, std::uint32_t& quality_levels) override;
private:
    IDirect3D9& api_;
};

// 00b1ffd0, cdecl(two resolution pointers), RET. Native signed subtraction,
// width first and height on equal widths, including 32-bit wraparound.
int compare_settings_resolutions_00b1ffd0(const Resolution& left,
    const Resolution& right);

// 00b27d80, thiscall(renderer), RET. Appends absent modes >=640x480 and sorts
// the entire vector. Deliberately does not clear an existing resolution vector.
void enumerate_settings_resolutions_00b27d80(SettingsRendererCapabilities& state,
    SettingsCapabilityQueries& queries);

// Settings-only projection of 00b2c8e0: GetDeviceCaps(0,HAL), LOWORD(pixel
// version), then version<0200h ? 1 : 2. Other capability/format queries in that
// routine are outside this module. Native ignores the query result; this API
// returns false without changing the fields on failure, avoiding its undefined
// uninitialized-stack reads. Startup must handle that failure explicitly.
bool gather_settings_shader_caps_00b2c8e0(SettingsRendererCapabilities& state,
    SettingsCapabilityQueries& queries);

// 00b295c0, thiscall(renderer, format), RET4. Clears AA, appends 0, then probes
// samples 2..15 with adapter 0, HAL, Windowed FALSE; only HRESULT==0 appends.
void rebuild_settings_antialias_00b295c0(SettingsRendererCapabilities& state,
    SettingsCapabilityQueries& queries, std::uint32_t surface_format);

const std::vector<Resolution>& settings_resolutions_00b1fff0(
    const SettingsRendererCapabilities& state);
const std::vector<int>& settings_antialias_levels_00b20000(
    const SettingsRendererCapabilities& state);
int settings_max_shader_model_00b200b0(const SettingsRendererCapabilities& state);
// Entire native body is RET4. This is an observed no-op, not an unresolved call.
void select_settings_shader_model_00b200c0(int selected_model);

} // namespace bsp
