#pragma once
#include <array>
#include <cstddef>
#include <string>

namespace bsp {
// Retained fog-owner field projection, not a native object layout/constructor.
// Suffixes identify native byte offsets; the first eight native bytes and all
// later fields are deliberately absent. Supply actual retained values, without
// copying from MaterialLighting or treating camera ambient_rgba as this owner.
struct SystemFogState {
    std::array<float, 4> color_08;
    std::array<float, 4> underwater_color_18;
    std::array<std::array<float, 4>, 4> directional_colors_28;
    float scalar_68, scalar_6c, scalar_70;
    float scalar_74, scalar_78, scalar_7c;
    float scalar_80, scalar_84;
    float scalar_88, scalar_8c, scalar_90;
};

// Bounded native00B46D97..00B46ED0; new C++ ABI, MSVC Win32 only.
// captured_fog is camera+184 captured at00B46D79. current_camera_fog references
// that camera's LIVE owner slot; it is reloaded before c37 and again between
// the c37.w source read and destination write. Both owners remain borrowed.
// Null captured owner skips all writes. Otherwise prefix must cover >=300
// initialized floats (through c74), with earlier/padding words preserved.
// Writes c72.xy,c73.xyz,c35.xyz,c36.xyz,c38..41,c37,c74 in that order.
// A missing reloaded owner is a NEW interface error, not native absence: native
// dereferences it. Failure retains earlier writes, including c37.w before an
// invalid underwater owner is reported. No clears, resizing or fallback values.
bool write_system_fog_constants_00b46d97(const SystemFogState* captured_fog,
    const SystemFogState* const& current_camera_fog,
    float* prefix, std::size_t word_count, std::string& error);
}
