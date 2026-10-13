#pragma once
// Non-gameplay leaves from the 0090-00BF naming sweep (cc13-sweep9x,
// docs/CC13_BAND_0090_00BF.md). Every name is a hypothesis, not a recovered
// symbol; the original address and ABI are recorded beside each declaration.
//
// Status: reconstructed, build-tested. These are new C++ interfaces, not drop-in
// binary replacements. Nothing here draws randomness.

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace bsp::band0090 {

// ---------------------------------------------------------------------------
// Hardware detection object (seg 66). Field offsets are the image's; the named
// getters are BSP_HardwareInfo_MemorySize (+2Ch), BSP_HardwareInfo_CpuSpeed
// (+30h) and BSP_HardwareInfo_GpuDeviceId (+28h index into the 12-byte GPU
// table at 00E0CFF0: {score, device id, name}).
struct HardwareSnapshot {
    std::uint32_t field_20{};       // +20h: gates the unknown-GPU row; meaning unproven
    std::int32_t gpu_row{-1};       // +28h: GPU table row, -1 = not found
    std::uint32_t memory_mb{};      // +2Ch
    std::uint32_t cpu_mhz{};        // +30h
    std::int32_t shader_model{};    // +24h, printed as "ShaderModel %i"
    std::vector<std::pair<std::int32_t, std::int32_t>> display_modes;  // +4/+8, 8-byte {w,h}
    std::vector<std::int32_t> antialias_levels;                         // +14h/+18h
    std::uint32_t language_lcid{};  // +C38h
    bool sound_device_found{};      // global 00F8A0E0 != 0 (BSP_HardwareInfo_DirectSoundEnumFirst)
};

// 0098DF90 BSP_HardwareInfo_RecommendDetailLevel, __fastcall(ECX = hardware
// object), RET, int in EAX (0..3). gpu_scores is the score column of the GPU
// table, row 0 being the 'Unknown' row (score 1 in this installation's image).
// The image indexes the table without a bounds check; this uses .at().
// x87 order from the listing (0098E04A..0098E082): FILD cpu tier * 1.6 (double
// 00CE3D48) + memory tier * 0.5 + gpu score * 0.9 (double 00D7A390, the float
// 0.9 widened), FSTP float, then / 3.0 and FSTP float; compared with the floats
// 1.5 (00CE380C) and 2.5 (00CF87C8).
int recommend_detail_level(const HardwareSnapshot& hw, const std::vector<std::uint32_t>& gpu_scores);

// 0098E0E0, __thiscall(ECX = hardware object, width, height), RET 8: true when
// the display-mode vector holds {width, height}.
bool supports_display_mode(const HardwareSnapshot& hw, std::int32_t width, std::int32_t height);

// 0098E910 BSP_HardwareInfo_BuildDefaultOptionsText, __fastcall(ECX = hardware
// object), returns ECX+C3Ch: the default options text appended with strcat
// semantics into the buffer at +C3Ch (cleared first). Lines, in order:
// fullscreen, Resolution, Vsync, ShaderModel, Antialias, Clouds, Foliage,
// Shadow, Reflection, OceanDetail, TextureDetail, ObjectDetail, SoundEnabled,
// Language.
std::string build_default_options_text(const HardwareSnapshot& hw, const std::vector<std::uint32_t>& gpu_scores);

// ---------------------------------------------------------------------------
// MSAR archive checksum (seg 93).
// 00BB9F40, static initialiser body: fills the 256-entry table at 01090500 with
// the reflected CRC-32 polynomial EDB88320h (eight unrolled shift/xor steps).
std::array<std::uint32_t, 256> build_crc32_table();
// 00BBA030 BSP_Crc32_Update, __thiscall(ECX = uint32 crc state, bytes, count),
// RET 8: crc = (crc >> 8) ^ table[(crc ^ byte) & 0FFh]. Pre- and post-inversion
// are the caller's (00BBBAB0 applies NOT before and after).
void crc32_update(std::uint32_t& crc, const std::uint8_t* bytes, std::size_t count);

// ---------------------------------------------------------------------------
// Streamed dialog / music manager (seg 73).
// 00A77860 BSP_StreamedDialogManager_SetTargetLevel, __thiscall(ECX = manager,
// float target), RET 4: +224h = target; +228h = |float(float(+220h - target)
// * 0.5)| (FSUB, FSTP float, FMUL double 00D7A280, FSTP float, AND 7FFFFFFFh).
struct DialogLevelState {
    float current_220{};
    float target_224{};
    float fade_rate_228{};
};
void set_dialog_target_level(DialogLevelState& state, float target);

}  // namespace bsp::band0090
