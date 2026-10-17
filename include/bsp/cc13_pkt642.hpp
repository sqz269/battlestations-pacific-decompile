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

}  // namespace bsp
