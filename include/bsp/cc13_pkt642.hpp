#pragma once

// cc13_pkt642: the HUD unit-marker builder 00642040 with its two widget
// writers 0063C620 and 0063D1E0 (packet cc13-pkt642, docs/CC13_PKT642.md).
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces over the native records, not drop-in replacements of the
// original ABI. Nothing here is wired into the game.
//
// Same conventions as cc13_sweepB..B6: native objects are raw byte blocks
// touched only at the offsets the image reads or writes, and every call the
// image makes goes through a named host method at the same point with the
// same arguments. Cc13Pkt642Host extends sweepB6's Cc13SweepB6Host, so the
// sweepB..B6 hooks are reused as they are; 00638C60 and 00640A20 are called
// through their sweepB2 reconstructions.

#include "bsp/cc13_sweepB6.hpp"

#include <cstdint>

namespace bsp {

// The 16-byte colour the image passes by value (r, g, b, a).
struct Cc13Pkt642Colour {
    float rgba[4];
};
static_assert(sizeof(Cc13Pkt642Colour) == 0x10);

class Cc13Pkt642Host : public Cc13SweepB6Host {
public:
    // 00638B90 BSP_HudMarker_ClampToScreenMargins: __fastcall(ECX = float* x,
    // EDX = float* y), RET; clamps both in place.
    virtual void hud_marker_clamp_to_margins_00638b90(float* x, float* y) = 0;
    // 00639810 BSP_HudMarker_IsWithinDisplayRange: stdcall(unit), RET 4, AL.
    // Both callers load ECX = [[00E198C4]+48h] first; it is passed through.
    virtual bool hud_marker_is_within_display_range_00639810(void* ecx_value, void* unit) = 0;
    // 0059CB90 BSP_Unit_GetHudIconIndex: __fastcall(ECX = unit, EDX = class
    // descriptor or null), RET, EAX.
    virtual std::int32_t unit_hud_icon_index_0059cb90(void* unit, void* descriptor) = 0;
    // 00639990 BSP_InGameHudMarkers_ResolveMarkerColourIndex: stdcall(unit),
    // RET 4, EAX (ECX = markers is loaded by the caller and not read).
    virtual std::int32_t hud_markers_colour_index_00639990(void* unit) = 0;
    // 006407A0 (marker record acquire): __thiscall(ECX = markers, dword), RET 4,
    // EAX = the marker record that 0063C620 / 0063D1E0 then fill.
    virtual void* hud_markers_acquire_record_006407a0(void* markers, std::uint32_t arg) = 0;
    // 004323D0 BSP_PointerVector_PushBack (STL instantiation): __thiscall(ECX =
    // vector, void* const* value), RET 4.
    virtual void pointer_vector_push_back_004323d0(void* vector, void* const* value) = 0;
    // 005220C0 (unit line of sight from the viewer): __thiscall(ECX =
    // [[00E198C4]+0CCh], unit), RET 4, AL.
    virtual bool unit_line_of_sight_005220c0(void* viewer, void* unit) = 0;
    // 00803DC0 BSP_Recon_DisplayNameForParty: __fastcall(ECX = party table,
    // EDX = unit), RET, EAX = const char*.
    virtual const char* recon_display_name_for_party_00803dc0(void* party, void* unit) = 0;
    // 0063E5C0 BSP_HudMarker_FormatDistance: stdcall(NativeString* out,
    // target), RET 8 (the caller's ECX = markers is not read).
    virtual void hud_marker_format_distance_0063e5c0(void* out, void* target) = 0;
    // 0077A2F0 BSP_Unit_HealthColour: __thiscall(ECX = float out[4], float
    // fraction), RET 4, EAX = out.
    virtual const float* unit_health_colour_0077a2f0(float* out, float fraction) = 0;

    // ---- options commit 005F65C0 (segment 23) ----
    // 008D4210 BSP_Settings_VideoModeDiffers: __thiscall(ECX = settings copy, other), RET 4, AL.
    virtual bool settings_video_mode_differs_008d4210(void* settings, void* other) = 0;
    // 008D44C0 BSP_Settings_SetXboxCompatibilityMode: __thiscall(ECX = 00F88980, dword), RET 4.
    virtual void settings_set_xbox_compatibility_008d44c0(void* settings, std::uint32_t value) = 0;
    // 005F52C0: __thiscall(ECX = 00F88980, settings copy), RET 4.
    virtual void settings_copy_from_screen_005f52c0(void* settings, void* copy) = 0;
    // 00698A10 BSP_InputConfiguration_Load: ECX = [00E188A8]+3Ch, RET.
    virtual void input_configuration_load_00698a10(void* configuration) = 0;
    // 00992830 BSP_SystemOptions_ReadLanguageFromOptions: no register or stack input, RET, EAX.
    virtual std::int32_t system_options_read_language_00992830() = 0;
    // 009927D0 BSP_HardwareInfo_Construct: __thiscall(ECX = info, dword), RET 4.
    virtual void hardware_info_construct_009927d0(void* info, std::int32_t language) = 0;
    // 0098C820 / 0098C870 / 0098C810 / 0098D3D0 / 0098C7F0 / 0098C800: the
    // hardware-info getters, ECX = info (0098D3D0 ignores it), RET, EAX.
    virtual const char* hardware_info_gpu_model_name_0098c820(void* info) = 0;
    virtual std::int32_t hardware_info_gpu_device_id_0098c870(void* info) = 0;
    virtual const char* hardware_info_os_version_0098c810(void* info) = 0;
    virtual const char* hardware_info_sound_device_name_0098d3d0(void* info) = 0;
    virtual std::uint32_t hardware_info_cpu_speed_0098c7f0(void* info) = 0;
    virtual std::uint32_t hardware_info_memory_size_0098c800(void* info) = 0;
    // 005EFFA0 BSP_OptionRow_Construct: __thiscall(ECX = row, key and value
    // native strings by value, both consumed), RET 10h, EAX = row.
    virtual void* option_row_construct_005effa0(void* row, Cc13SweepBNativeString key,
                                                Cc13SweepBNativeString value) = 0;
    // 005EFDC0 BSP_OptionRow_Destruct: ECX = row.
    virtual void option_row_destruct_005efdc0(void* row) = 0;
    // 005F6530 (STL vector push_back): __thiscall(ECX = vector, const row*), RET 4.
    virtual void option_row_vector_push_back_005f6530(void* vector, const void* row) = 0;
    // 005F5F80 (STL vector tidy/destroy): ECX = vector.
    virtual void option_row_vector_destroy_005f5f80(void* vector) = 0;
    // 00BF7A6A sprintf (cdecl) with "%llx" and one 64-bit argument.
    virtual std::int32_t crt_sprintf_u64_00bf7a6a(char* buffer, const char* format, std::uint64_t value) = 0;
    // 005F1750 BSP_NativeString_FromUnsigned: __thiscall(ECX = out, dword), RET 4.
    virtual void* native_string_from_unsigned_005f1750(void* out, std::uint32_t value) = 0;
    // 005F1840 BSP_NativeString_ConstructSignedDecimal: __thiscall(ECX = out, int), RET 4, EAX = out.
    virtual void* native_string_signed_decimal_005f1840(void* out, std::int32_t value) = 0;
    // 00753570 (metrics send): __thiscall(ECX = [00E1AED4], const row vector*), RET 4.
    virtual void metrics_send_00753570(void* metrics, const void* rows) = 0;
    // 004C1710 BSP_MediaManager_GetOrCreate: no arguments, RET, EAX.
    virtual void* media_manager_get_or_create_004c1710() = 0;
    // 00A4C2D0: a one-instruction RET 4 body, __thiscall(ECX = media manager,
    // dword); 005F6E33 pushes its 1 before calling 004C1710.
    virtual void media_manager_ret4_00a4c2d0(void* manager, std::int32_t value) = 0;
    // 008D6C00 BSP_Settings_SaveToProfile: __thiscall(ECX = 00F88980, callback address), RET 4.
    virtual void settings_save_to_profile_008d6c00(void* settings, std::uint32_t callback) = 0;
    // 00427110 BSP_NativeStringVector_Resize: __thiscall(ECX = 00F88A24, count), RET 4.
    virtual void native_string_vector_resize_00427110(void* vector, std::uint32_t count) = 0;
    // 004D7F90 BSP_Game_RequestReturnToTitle: ECX = [00E188A8].
    virtual void game_request_return_to_title_004d7f90(void* game) = 0;
    // 008D5B50 BSP_Settings_ApplyAll: ECX = 00F88980.
    virtual void settings_apply_all_008d5b50(void* settings) = 0;
    // 005F54F0 BSP_OptionsScreen_BuildMainList: ECX = screen.
    virtual void options_screen_build_main_list_005f54f0(void* screen) = 0;
    // Screen-object vtable +60h with one dword argument.
    virtual void object_vcall_60(void* object, std::int32_t value) = 0;
};

// 0063C620 BSP_HudMarker_EmitBoxMarker, __thiscall(ECX = marker record),
// RET 38h (14 dwords): corners (float[5][2]: four box corners, then the
// projected anchor), group (0..5), show (dword; low byte tested), text_mode
// (low byte), text (const char* or null), unit, the distance NativeString by
// value (released here), scale, colour by value, flag (low byte).
void cc13_pkt642_hud_marker_emit_box_marker_0063c620(void* record, float* corners, std::int32_t group,
                                                     std::uint32_t show, std::uint32_t text_mode,
                                                     const char* text, void* unit,
                                                     Cc13SweepBNativeString distance, float scale,
                                                     Cc13Pkt642Colour colour, std::uint32_t flag,
                                                     Cc13Pkt642Host& host);

// 0063D1E0 BSP_HudMarker_WriteUnitMarkerWidgets, __thiscall(ECX = marker
// record), RET 38h (14 dwords): unit, x, y, angle, group, colour by value,
// name (const char*), the distance NativeString by value (released here),
// scale, flag (dword; low byte tested, whole dword passed to +80h's +34h).
void cc13_pkt642_hud_marker_write_unit_marker_widgets_0063d1e0(void* record, void* unit, float x, float y,
                                                               float angle, std::int32_t group,
                                                               Cc13Pkt642Colour colour, const char* name,
                                                               Cc13SweepBNativeString distance, float scale,
                                                               std::uint32_t flag, Cc13Pkt642Host& host);

// 00642040 BSP_InGameHudMarkers_BuildUnitMarker, __thiscall(ECX = markers
// screen), RET 18h: unit, is_self, is_friendly, is_target (each a dword whose
// low byte is tested; is_self is also passed whole to 0063D1E0), kind_a,
// kind_b.
void cc13_pkt642_hud_markers_build_unit_marker_00642040(void* markers, void* unit, std::uint32_t is_self,
                                                        std::uint32_t is_friendly, std::uint32_t is_target,
                                                        std::int32_t kind_a, std::int32_t kind_b,
                                                        Cc13Pkt642Host& host);

// 005F65C0 (options commit), __thiscall(ECX = options screen), RET: copies
// the screen's settings into 00F88980, sends the hardware/metrics rows once
// (or when +150h differs), saves to the profile and rebuilds or returns to
// the title screen.
void cc13_pkt642_options_screen_commit_005f65c0(void* screen, Cc13Pkt642Host& host);

}  // namespace bsp
