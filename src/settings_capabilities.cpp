#include "bsp/settings_capabilities.hpp"
#include "bsp/d3d9_startup.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {

int wrapped_difference(int left, int right) {
    const std::uint32_t bits = static_cast<std::uint32_t>(left)
        - static_cast<std::uint32_t>(right);
    std::int32_t result;
    static_assert(sizeof(result) == sizeof(bits));
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

int __cdecl compare_resolution_pointers(const void* left, const void* right) {
    return compare_settings_resolutions_00b1ffd0(
        *static_cast<const Resolution*>(left), *static_cast<const Resolution*>(right));
}

} // namespace

std::uint32_t Win32SettingsCapabilityQueries::adapter_mode_count(
    std::uint32_t adapter, std::uint32_t format) {
    return api_.GetAdapterModeCount(adapter, static_cast<D3DFORMAT>(format));
}

std::int32_t Win32SettingsCapabilityQueries::enum_adapter_mode(
    std::uint32_t adapter, std::uint32_t format, std::uint32_t index,
    SettingsAdapterMode& mode) {
    D3DDISPLAYMODE native_mode{};
    const HRESULT result = api_.EnumAdapterModes(adapter, static_cast<D3DFORMAT>(format),
        index, &native_mode);
    if (result == D3D_OK) {
        mode.width = native_mode.Width;
        mode.height = native_mode.Height;
    }
    return result;
}

std::int32_t Win32SettingsCapabilityQueries::device_pixel_shader_version(
    std::uint32_t adapter, std::uint32_t device_type, std::uint32_t& version) {
    D3DCAPS9 caps{};
    const HRESULT result = api_.GetDeviceCaps(adapter, static_cast<D3DDEVTYPE>(device_type),
        &caps);
    if (SUCCEEDED(result)) version = caps.PixelShaderVersion;
    return result;
}

std::int32_t Win32SettingsCapabilityQueries::check_multisample_type(
    std::uint32_t adapter, std::uint32_t device_type, std::uint32_t surface_format,
    bool windowed, std::uint32_t samples, std::uint32_t& quality_levels) {
    DWORD native_quality = quality_levels;
    const HRESULT result = api_.CheckDeviceMultiSampleType(adapter,
        static_cast<D3DDEVTYPE>(device_type), static_cast<D3DFORMAT>(surface_format),
        windowed ? TRUE : FALSE, static_cast<D3DMULTISAMPLE_TYPE>(samples), &native_quality);
    quality_levels = native_quality;
    return result;
}

int compare_settings_resolutions_00b1ffd0(const Resolution& left,
    const Resolution& right) {
    return left.width == right.width ? wrapped_difference(left.height, right.height)
        : wrapped_difference(left.width, right.width);
}

void enumerate_settings_resolutions_00b27d80(SettingsRendererCapabilities& state,
    SettingsCapabilityQueries& queries) {
    state.adapter_mode_state_19dc = 0;
    const std::uint32_t count = queries.adapter_mode_count(0, 0x16);
    for (std::uint32_t index = 0; index < count; ++index) {
        SettingsAdapterMode mode;
        if (queries.enum_adapter_mode(0, 0x16, index, mode) != 0
            || mode.width < 0x280 || mode.height < 0x1e0) continue;
        const Resolution entry{static_cast<int>(mode.width), static_cast<int>(mode.height)};
        const auto found = std::find_if(state.resolutions.begin(), state.resolutions.end(),
            [&entry](const Resolution& known) {
                return known.width == entry.width && known.height == entry.height;
            });
        if (found == state.resolutions.end()) state.resolutions.push_back(entry);
    }
    if (state.resolutions.size() > 1) {
        std::qsort(state.resolutions.data(), state.resolutions.size(), sizeof(Resolution),
            compare_resolution_pointers);
    }
}

bool gather_settings_shader_caps_00b2c8e0(SettingsRendererCapabilities& state,
    SettingsCapabilityQueries& queries) {
    std::uint32_t version = 0;
    if (queries.device_pixel_shader_version(0, 1, version) < 0) return false;
    state.pixel_shader_version_28 = version & 0xffff;
    state.max_shader_model = state.pixel_shader_version_28 < 0x200 ? 1 : 2;
    return true;
}

void rebuild_settings_antialias_00b295c0(SettingsRendererCapabilities& state,
    SettingsCapabilityQueries& queries, std::uint32_t surface_format) {
    state.antialias_levels.clear();
    state.antialias_levels.push_back(0);
    // Native reuses the format stack argument as the quality output while EBP
    // preserves the format for every call. Quality never determines acceptance.
    std::uint32_t quality_levels = surface_format;
    for (std::uint32_t samples = 2; samples < 16; ++samples) {
        if (queries.check_multisample_type(0, 1, surface_format, false,
                samples, quality_levels) == 0) {
            state.antialias_levels.push_back(static_cast<int>(samples));
        }
    }
}

const std::vector<Resolution>& settings_resolutions_00b1fff0(
    const SettingsRendererCapabilities& state) {
    return state.resolutions;
}

const std::vector<int>& settings_antialias_levels_00b20000(
    const SettingsRendererCapabilities& state) {
    return state.antialias_levels;
}

int settings_max_shader_model_00b200b0(const SettingsRendererCapabilities& state) {
    return state.max_shader_model;
}

void select_settings_shader_model_00b200c0(int) {}

} // namespace bsp
